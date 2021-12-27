auto Plek::find(const string& symbolName) -> std::tuple<bool, Frame, string, Symbol> {
  auto parts = symbolName.split(".");
  auto lastId = parts.takeRight();
  Symbol symbol{};
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
      symbol = res();
      scope = upScope;
      found = true;
      break;
    }

    // walk downstream
    scope = scope->parent;
  } while(scope);  
  
  // return stuff, if successfull or not, to caller
  return std::make_tuple(found, scope, lastId, symbol);
}

auto Plek::assign(const string& dest, Result src) -> void {
  auto [found, scope, name, res] = find(dest);
  if(found) {
    scope->assign(name, src);
  }
  else {
    //todo: still contains dots -> error! No implicit namespaces!
    frames.right()->assign(dest, src);
    notice("implicit created var ", dest);
  }
}

auto Plek::invoke(const string& fullName, Statement args) -> Result {
  string argc = {args->size()};
  string id = {fullName, "#", argc};
  string gId = {fullName, "#*"};

  // fitting or general build in function?
  if(auto fun = coreFunctions.find(id)) {
    return fun()(args);
  }
  else if(auto fun = coreFunctions.find(gId)) {
    return fun()(args);
  }

  // incode function?
  auto [found, scope, name, res] = find(fullName);
  if(!found) {
    throw string{"cannot call unknown ", fullName};
  }

  // get function handle
  Statement fun;
  if(res.type != symbt(Map)) throw string{"cannot call ", fullName};
  if(auto funres = res.references.find({argc})) {
    fun = funres();
  }
  else {
    throw string{"cannot call unknown ", fullName, "#", argc};
  }

  // invoke
  MacroStatement macro{fun};
  auto padef = macro.getArguments();
  auto fscope = Frame::create(scope);

  for(int i=0; i<args->size(); i++) {
    auto t = padef[i];
    auto v = args->content[i];
    auto name = t->value.getString();
    
    Result res = (t->is(st(RefArgument))) ? Result{v->value} : evaluateRHS(v);

    if(!res || res.isNothing()) warning("Parameter ", i+1, " is not set");

    if(t->type == st(ConstArgument)) fscope->setConstant(name, res);
    else fscope->setVariable(name, res);
  }

  frames.append(fscope);
  exBlock(macro.getCode());
  frames.removeRight();
  return fscope->result;
}

auto Plek::readArchitecture(const string& name) -> string {
  string location{Path::userData(), "bass/architectures/", name, ".arch"};
  if(!file::exists(location)) location = {Path::program(), "architectures/", name, ".arch"};
  if(!file::exists(location)) error("unknown architecture: ", name);
  return string::read(location);
}