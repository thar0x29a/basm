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
  if(auto res = symbolTable.find(name)) {
    return res().value;
  }
  return {nothing};
}

auto Plek::setConstant(const string& name, const Value& val) -> void {
  if(auto res = symbolTable.find(name)) {
    if(res().type == SymbolRef::SymbolType::Const) error("constant cannot be modified");
  }
  symbolTable.insert(name, {SymbolRef::SymbolType::Const, val});
}

auto Plek::setVariable(const string& name, const Value& val) -> void {
  symbolTable.insert(name, {SymbolRef::SymbolType::Var, val});
}