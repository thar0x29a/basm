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

auto Plek::identifier(const string& identName) -> Value {
  auto [found, scope, name] = find(identName);
  if(!found) return {nothing};

  auto res = scope->symbolTable.find(name)();
  return res.value;
}

auto Plek::find(const string& symbolName) -> std::tuple<bool, Frame, string> {
  auto parts = symbolName.split(".");
  auto lastId = parts.takeRight();
  //SymbolRef symbol{};
  bool found = false;

  Frame scope = frames.last();
  do {
    // try to walk upstream
    auto upScope = scope;
    for(auto el : parts) {
      if(auto nextScope = upScope->children.find(el)) {
        upScope = nextScope();
      } else {
        upScope = scope;
        break;
      }
    }
    
    // end of the road. do we find it?
    if(auto res = upScope->symbolTable.find(lastId)) {
      //symbol = res();
      scope = upScope;
      found = true;
      break;
    }

    // walk downstream
    scope = scope->parent;
  } while(scope);  
  
  // return stuff, if successfull or not, to caller
  return std::make_tuple(found, scope, lastId);
}

auto Plek::assign(const string& assName, const Value& val) -> void {
  auto [found, scope, name] = find(assName);
  if(found) {
    scope->assign(assName, val);
  }
  else {
    //todo: still contains dots -> error! No implicit namespaces!
    frames.right()->assign(assName, val);
    notice("implicit created var ", name);
  }
}

auto Plek::invoke(const string& fullName, Statement args) -> Value {
  string id = {fullName, "#", args->size()};
  string gId = {fullName, "#*"};

  // fitting or general build in function?
  if(auto fun = coreFunctions.find(id)) {
    return fun()(args);
  }
  else if(auto fun = coreFunctions.find(gId)) {
    return fun()(args);
  }

  // incode function?
  auto [found, scope, name] = find(id);
  if(!found) {
    throw string{"cannot call unknown ", id};
  }

  // get function handle
  auto fun = scope->symbolTable.find(name)();

  // invoke
  auto padef = fun.ref->content[1];
  auto fscope = Frame::create(scope);

  for(int i=0; i<args->size(); i++) {
    auto t = padef->content[i];
    auto v = args->content[i];

    if(v->type == st(Identifier)) {
      if(t->type == st(RefArgument)) {
        v->result = v->value;
      } else {
        evaluate(v);
      }
    }

    if(!v->result || v->result.isNothing()) warning("Parameter ", i+1, " is not set");
    
    if(t->type == st(ConstArgument)) {
      fscope->setConstant(t->result, v->result);
    }
    else {
      fscope->setVariable(t->result, v->result);
    }
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

auto Plek::readArchitecture(const string& name) -> string {
  string location{Path::userData(), "bass/architectures/", name, ".arch"};
  if(!file::exists(location)) location = {Path::program(), "architectures/", name, ".arch"};
  if(!file::exists(location)) error("unknown architecture: ", name);
  return string::read(location);
}