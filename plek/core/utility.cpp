auto Plek::find(const string& symbolName) -> std::tuple<bool, Frame, string, Symbol> {
  auto parts = symbolName.split(".");
  auto lastId = parts.takeRight();
  
  Symbol symbol{};
  bool foundVal = false;
  Frame scope = frames.last();
  auto downScope = scope;

  do {
    auto upScope = downScope;
    bool foundPath = false;

    // try to walk upstream
    if(parts.size() > 0) {
      for(auto el : parts) {
        if(auto nextScope = upScope->children.find(el)) {
          upScope = nextScope();
          foundPath = true;
        } else {
          foundPath = false;
          break;
        }
      }
    }
    else {
      foundPath = true;
    }
    
    // end of the road. do we find it?
    if(foundPath==true) {
      if(auto res = upScope->symbolTable.find(lastId)) {
        symbol = res();
        scope = upScope;
        foundVal = true;
        break;
      }
      else {
        foundVal = false;
      }
    }

    // walk downstream
    downScope = downScope->parent;
  } while(downScope);  
  
  // return stuff, if successfull or not, to caller
  return std::make_tuple(foundVal, scope, lastId, symbol);
}

auto Plek::assign(Result dest, Result src) -> void {
  string destname = dest.getString();

  auto [found, scope, name, res] = find(destname);
  if(found) {
    if(res.isValue() && res.value.isCustom()) {
      string tname = res.value.getCustom().name;
      string aname = "";

      if(src.isCustom()) aname = src.getCustom().name;

      string fkey{"operator=", aname};
      notice("Custom assignment! ", fkey);

      // try to call overloaded function
      if(auto type = customTypes.find(tname)) {
        auto fun = type().get(fkey);
        if(!fun.isNothing()) {
          notice("Bingo! We can invoke this! ", fun);
          // return afterwards
        }
        else {
          notice("We cannot invoke this :/ ", fun);
        }
      } 
      else {
        error("Type ", tname, " got not properly defined.");
      }

      // 2. if not available, force value passing
    }

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
    ArgumentStatement pdef{padef[i]};
    auto v = args->content[i];
    auto name = pdef.getName();
    
    Result res = pdef.isReference() ? Result{v->value} : evaluateRHS(v);

    if(!res || res.isNothing()) warning("Parameter ", i+1, " is not set");
    if(pdef.isCustom()) {
      if(!res.isCustom()) 
        error("Parameter ", i+1, " is not an custom type.");
      if(!res.isCustom(pdef.getCustomType())) 
        error("Parameter ", i+1, " does not match the custom type. Found ", res.getCustom().name, " but expected ", pdef.getCustomType());
    }

    if(pdef.isConst()) fscope->setConstant(name, res);
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
    currentStmt = mimi.missing;
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