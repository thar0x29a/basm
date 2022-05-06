auto FrameElement::setConstant(const string& name, Result val) -> void {
  if(auto res = symbolTable.find(name)) {
    if(res->isProtected()) throw string{"constant cannot be modified '", name, "'"};
  }
  if(val.isSymbol()) symbolTable.insert(name, val.getSymbol());
  else symbolTable.insert(name, {SymbolType::Value, SymbolMode::Const, val});
}

auto FrameElement::setVariable(const string& name, Result val) -> void {
  if(auto res = symbolTable.find(name)) {
    if(res->isProtected()) throw string{"constant cannot be modified '", name, "'"};
  }
  if(val.isSymbol()) symbolTable.insert(name, val.getSymbol());
  else symbolTable.insert(name, {SymbolType::Value, SymbolMode::Var, val});
}

auto FrameElement::setVariable(const string& name, Result value, string key) -> void {
  Symbol val = (value.isSymbol()) ? value.getSymbol() : Symbol::newVar(value);

  if(auto res = symbolTable.find(name)) {
    if(res->isProtected()) throw string{"constant cannot be modified '", name, "'"};
    res->references.insert(key, val);
  }
  else {
    Symbol m = Symbol::newMap();
    m.references.insert(key, val);
    symbolTable.insert(name, m);
  }
}

auto FrameElement::setMacro(MacroStatement def) -> void {
  string id = def.getName();
  string key = {def.getArgCount()};
  Symbol map{SymbolType::Map, SymbolMode::Const};

  // check modes  
  if(auto res = symbolTable.find(id)) {
    if(res->type == symbt(Map)) {
      map = res();
      if(auto ex = map.references.find(key)) {
        if(ex->isProtected()) throw string{"Function  already exist '", name, "'"};    
      }
    }
    else if(res->isProtected()) throw string{"Function name is allready taken '", name, "'"};
  }

  Symbol entry{SymbolType::Reference, SymbolMode::Const, {nothing}, def.ref};
  map.references.insert(key, entry);
  symbolTable.insert(def.getName(), map);
}

auto FrameElement::assign(const string& name, Result val) -> void {
  // try to assign, not just to replace
  if(auto res = symbolTable.find(name)) {
    if(res->isProtected()) throw string{"constant cannot be modified '", name, "'"};
    res->value = val;
  }
  else {
    setVariable(name, val);
  }
}

auto FrameElement::addScope(const Frame frm) -> void {
  children.insert(frm->name, frm);
}

/**
 * Helper to manage label 'history' in combination of
 * lookaheads. Stores values since labels are meant to
 * be const anyway. 
**/
auto FrameElement::addLabel(const Result& val) -> void {
  if(labels.size()==labelp) {
    labels.append(val);
  }
  else if(labels.size()>labelp) {
    labels[labelp] = val;
  }
  else {
    throw string{"Scope contains an invalid label state."};
  }
  labelp++;
}

