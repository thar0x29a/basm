auto FrameElement::setConstant(const string& name, Result val) -> void {
  if(auto res = symbolTable.find(name)) {
    if(res->type == symbt(Const)) throw string{"constant cannot be modified '", name, "'"};
  }
  if(val.isSymbol()) symbolTable.insert(name, val.getSymbol());
  else symbolTable.insert(name, {symbt(Const), val});
}

auto FrameElement::setVariable(const string& name, Result val) -> void {
  if(auto res = symbolTable.find(name)) {
    if(res->type == symbt(Const)) throw string{"constant cannot be modified '", name, "'"};
  }
  if(val.isSymbol()) symbolTable.insert(name, val.getSymbol());
  else symbolTable.insert(name, {symbt(Var), val});
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

auto FrameElement::assign(const string& name, Result val) -> void {;
  setVariable(name, val);
}

auto FrameElement::addScope(const Frame frm) -> void {
  children.insert(frm->name, frm);
}

