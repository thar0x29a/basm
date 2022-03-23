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

auto Plek::constant(const string& name, const string& value) -> void {
  auto scope = frames.last();
  bool isNumber = true;
  bool isfloat = true;
  
  for(int i=0;i<value.length();i++) {
    char c = value[i];
    if(c=='.') { isNumber = false; continue; }
    if(c >= '0' && c <= '9') continue;
    
    isNumber = false;
    isfloat = false;
    break;
  }
  
  if(isNumber) scope->setConstant(name, {(int64_t)toInteger(value)});
  else if(isfloat) scope->setConstant(name, { (double)toReal(value)});
  else scope->setConstant(name, {value});
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
    throw string{"cannot call unknown function ", fullName};
  }

  // get function handle
  Statement fun;
  if(res.type != symbt(Map)) throw string{"cannot call ", fullName};
  if(auto funres = res.references.find({argc})) {
    fun = funres->reference;
  }
  else {
    throw string{"cannot call unknown ", fullName, "#", argc};
  }

  // invoke
  MacroStatement macro{fun};
  auto padef = macro.getArguments();
  auto fscope = Frame::create(scope);
  fscope->context = SymbolRef::create(res);

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

auto Plek::isDirective(const string& name) -> uint {
  uint dataLength = 0;
  for(auto d : directives.EmitBytes) {
    if(d.token == name) {
      dataLength = d.dataLength;
      break;
    }
  }
  return dataLength;
};
  

auto Plek::addMissing(const string& name) -> void {
  MissingSymbol mimi{
    name, currentStmt,
    clean_origin, base,
    endian,
    architecture,
    frames.last()
  };

  //notice("Added issue for ", name);
  missing.append(mimi);
}

auto Plek::testMissing(const MissingSymbol& mimi) -> void {
  //notice("test for missing ", mimi.identName);
  frames.append(mimi.frame);

  auto [found, scope, name, res] = find(mimi.identName);
  if(!found) {
    error("Cannot solve '", mimi.identName, "'.");
  }
  else {
    if(!res.isProtected()) {
      error("Lookahead is limited to const values.");
    }
    //notice(name, " is ", res.value);
  }

  frames.removeRight();
}

auto Plek::solveMissing(const MissingSymbol& mimi) -> void {
  //notice("try to handle ", mimi.identName);
  
  currentStmt = mimi.missing;
  origin = mimi.origin;
  base = mimi.base;
  endian = mimi.endian;
  architecture = mimi.architecture;

  frames.append(mimi.frame);
  seek(mimi.origin);

  /*warning("stmt:",currentStmt,
    ", origin:", origin, 
    ", base:",base, 
    ", endian:",(uint)endian, 
    ", offset:",mimi.offset);/**/

  // ex asm stmt
  auto ret_state = exAssembly(currentStmt);

  if(ret_state == ReturnState::Lookahead) {
    error("Cannot solve '", mimi.identName, "' !");
  }

  frames.removeRight();
}