auto FrameElement::setConstant(const string& name, const Value& val) -> void {
  if(auto res = symbolTable.find(name)) {
    if(res().type == SymbolRef::SymbolType::Const) throw string{"constant cannot be modified '", name, "'"};
  }
  symbolTable.insert(name, {SymbolRef::SymbolType::Const, val});
}

auto FrameElement::setVariable(const string& name, const Value& val) -> void {
  if(auto res = symbolTable.find(name)) {
    if(res().type == SymbolRef::SymbolType::Const) throw string{"constant with the same name cannot be modified '",name,"'"};
  }  
  symbolTable.insert(name, {SymbolRef::SymbolType::Var, val});
}

auto FrameElement::setMacro(const string& name, Statement def) -> void {
  string id = {name, "#", def->content[1]->size()};
  symbolTable.insert(id, {SymbolRef::SymbolType::Callable, nothing, def});
}

auto FrameElement::assign(const string& name, const Value& val) -> void {
  if(auto res = symbolTable.find(name)) {
    if(res().type == SymbolRef::SymbolType::Const) throw string{"constant cannot be modified '", name, "'"};
    auto type = res().type;
    symbolTable.remove(name);
    symbolTable.insert(name, {type, val});
  }
  setVariable(name, val);
}

auto FrameElement::addScope(const Frame frm) -> void {
  children.insert(frm->name, frm);
}

