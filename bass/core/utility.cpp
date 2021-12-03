auto Plek::walkUp(const Program& what, std::function<bool (Statement, int)> with, int level) -> void {
  for(auto item : what) {
    if(!item) continue;

    // bottom up
    walkUp(item().content, with, level+1);
    with(item, level);
  }
}

auto Plek::walkDown(const Program& what, std::function<bool (Statement, int)> with, int level) -> void {
  for(auto item : what) {
    if(!item) continue;

    // top down
    if(with(item, level)) {
      walkDown(item().content, with, level+1);
    }
  }
}

auto Plek::identifier(const string& name) -> Value {
  return findSymbol(name).value;
}

auto Plek::findSymbol(const string& symbolName) -> SymbolRef {
  auto scope = frames.right();

  do {
    //notice("search ", symbolName, " in ", scope->name);
    if(auto res = scope->symbolTable.find(symbolName)) return res();

    scope = scope->parent;
  } while(scope);

  //notice("could not find ", symbolName, "\n");
  return {SymbolRef::nothing()};
}

auto Plek::assign(const string& name, const Value& val) -> void {
  auto scope = frames.right();

  do {
    if(auto res = scope->symbolTable.find(name)) {
      scope->assign(name, val);
      return;
    }

    scope = scope->parent;
  } while(scope);


  frames.right()->assign(name, val);
  notice("implicit created var ", name);
}

auto Plek::invoke(const string& fullName, Statement args) -> Value {
  string id = {fullName, "#", args->size()};
  auto parts = id.split(".");
  auto lastId = parts.takeRight();
  SymbolRef fun{};
  bool found = false;

  Frame scope = frames.last();
  //notice("invoced ", id, " in ", scope->name);

  do {
    // try to walk upstream
    auto upScope = scope;
    for(auto el : parts) {
      if(auto nextScope = upScope->children.find(el)) {
        upScope = nextScope();
      }
    }
    
    // end of the road. do we find it?
    if(auto res = upScope->symbolTable.find(lastId)) {
      fun = res();
      scope = upScope;
      found = true;
      break;
    }

    // walk downstream
    scope = scope->parent;
  } while(scope);  
  
  if(!found) error("cannot call unknown ", id);

  //notice("we found ", id, " in ", scope->name);

  // finally invoke!
  auto padef = fun.ref->content[1];
  auto fscope = Frame::create(scope); // bug here - we use the wrong scope

  for(int i=0; i<args->size(); i++) {
    auto t = padef->content[i];
    auto v = args->content[i];

    //todo: handle decl types
    fscope->setVariable(t->value, v->value);
  }

  frames.append(fscope);
  excecuteBlock(fun.ref->content[2], fscope);
  frames.removeRight();
  return fscope->result;
}

auto Plek::scopePath() -> string {
  string res{""};

  for(auto f : frames) {
    if(f->name.size()>0)
      res.append('.').append(f->name);
  }
  return res.slice(1);
}