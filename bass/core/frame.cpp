auto FrameElement::setConstant(const string& name, const Value& val) -> void {
  if(auto res = symbolTable.find(name)) {
    if(res().type == SymbolRef::SymbolType::Const) throw "constant cannot be modified";
  }
  symbolTable.insert(name, {SymbolRef::SymbolType::Const, val});
}

auto FrameElement::setVariable(const string& name, const Value& val) -> void {
  print("-> var ", name, " = ", val, "\n");
  symbolTable.insert(name, {SymbolRef::SymbolType::Var, val});
}

auto FrameElement::setMacro(const string& name, Statement def) -> void {
  string id = {name, "#", def->content[1]->size()};
  symbolTable.insert(id, {SymbolRef::SymbolType::Callable, nothing, def});
}