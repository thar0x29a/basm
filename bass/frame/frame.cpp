auto FrameElement::setConstant(const string& name, const Value& val) -> void {
  if(auto res = symbolTable.find(name)) {
    if(res->type == symbt(Const)) throw string{"constant cannot be modified '", name, "'"};
  }
  symbolTable.insert(name, {symbt(Const), val});
}

auto FrameElement::setConstant(const string& name, Symbol value) -> void {
  if(auto res = symbolTable.find(name)) {
    if(res->type == symbt(Const)) throw string{"constant cannot be modified '", name, "'"};
  }
  symbolTable.insert(name, value);
}

auto FrameElement::setVariable(const string& name, const Value& val) -> void {
  if(auto res = symbolTable.find(name)) {
    if(res->type == symbt(Const)) throw string{"constant with the same name cannot be modified '",name,"'"};
  }  
  symbolTable.insert(name, {symbt(Var), val});
}

auto FrameElement::setVariable(const string& name, Symbol value) -> void {
  if(auto res = symbolTable.find(name)) {
    if(res->type == symbt(Const)) throw string{"constant with the same name cannot be modified '",name,"'"};
  }  
  symbolTable.insert(name, value);
}

auto FrameElement::setMacro(MacroStatement def) -> void {
  string id = def.getName();
  auto map = Symbol::asMap();
  
  if(auto res = symbolTable.find(id)) {
    if(res->type == symbt(Map)) map = res();
  }

  map.references.insert({def.getArgCount()}, def.ref);
  symbolTable.insert(def.getName(), map);
}

auto FrameElement::assign(const string& name, const Value& val) -> void {
  if(auto res = symbolTable.find(name)) {
    if(res->type == symbt(Const)) throw string{"constant cannot be modified '", name, "'"};
    auto type = res->type;
    symbolTable.remove(name);
    symbolTable.insert(name, {type, val});
  }
  else setVariable(name, val);
}

auto FrameElement::assign(const string& name, Symbol value) -> void {
  if(auto res = symbolTable.find(name)) {
    if(res->type == symbt(Const)) throw string{"constant cannot be modified '", name, "'"};
  }

  symbolTable.insert(name, value);
}

auto FrameElement::addScope(const Frame frm) -> void {
  children.insert(frm->name, frm);
}

