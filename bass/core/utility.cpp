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

auto Plek::findSymbol(const string& name) -> SymbolRef {
  auto scopes = frames;

  // hunt in this and parent scopes
  while(scopes.size()>0) {
    auto scope = scopes.takeRight();
    if(auto res = scope->symbolTable.find(name)) {
      return res();
    }
  }

  return {SymbolRef::nothing()};
}

auto Plek::assign(const string& name, const Value& val) -> void {
  auto scopes = frames;
  while(scopes.size()>0) {
    auto scope = scopes.takeRight();
    if(auto res = scope->symbolTable.find(name)) {
      scope->assign(name, val);
      return;
    }
  }

  frames.right()->assign(name, val);
  notice("implicit created var ", name, "\n");
}

auto Plek::invoke(const string& name, Statement args) -> Value {
  string id = {name, "#", args->size()};
  
  //1. find or find not callable with this name
  auto fun = findSymbol(id);
  if(fun.ref) {
    auto padef = fun.ref->content[1]; // it would be nice to cast this somehow ..

    //2. prepare custom scope with parameters
    auto scope = Frame::create();
    
    for(int i=0; i<args->size(); i++) {
      auto t = padef->content[i];
      auto v = args->content[i];

      //todo: handle decl types
      scope->setVariable(t->value, v->value);
    }

    //3. call with this scope!
    frames.append(scope);
    excecuteBlock(fun.ref->content[2], scope);
    frames.removeRight();
    return scope->result;
  }
  
  return {nothing};
}

