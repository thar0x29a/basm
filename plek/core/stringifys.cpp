template<> struct nall::stringify<Bass::Value::Custom> {
  stringify(Bass::Value::Custom source) {
    _text = { source.value() };
  }

  auto data() const -> const char* { return _text.data(); }
  auto size() const -> uint { return _text.size(); }

  string _text;
};

template<> struct nall::stringify<Bass::Value> {
  stringify(Bass::Value source) {
    if(!source) _text = {"nothing"};
    else if(source.isInt()) _text = { source.getInt() };
    else if(source.isFloat()) _text = { source.getFloat() };
    else if(source.isString()) _text = { source.getString() };
    else if(source.isNothing()) _text = { "NULL" };
    else if(source.isCustom()) _text = { source.getCustom() };
    //else if(source.isBool()) _text = (source.getBool()) ? "true" : "false";
    else _text =  {"`", source.type().name(), "`"};
  }

  auto data() const -> const char* { return _text.data(); }
  auto size() const -> uint { return _text.size(); }
  
  string _text;
};

template<> struct nall::stringify<Bass::Result> {
  stringify(Bass::Result source) {
    if(!source) _text = {"nullptr"};
    else if(source.isInt()) _text = { source.getInt() };
    else if(source.isFloat()) _text = { source.getFloat() };
    else if(source.isString()) _text = { source.getString() };
    else if(source.isSymbol()) _text = { "Symbol" };
    else if(source.isNothing()) _text = { "NULL" };
    else if(source.isCustom()) _text = { source.getCustom() };
    //else if(source.isBool()) _text = (source.getBool()) ? "true" : "false";
    else _text =  {"`", source.type().name(), "`"};
  }

  auto data() const -> const char* { return _text.data(); }
  auto size() const -> uint { return _text.size(); }
  
  string _text;
};

template<> struct nall::stringify<Bass::Statement> {
  stringify(Bass::Statement source) {
    //if(source->type == st(Value)) _text = make_string(source->value);
    //else {
      uint k = (uint)source->type;
      _text = { Bass::StmtNames[k] };
    //}
  }

  auto data() const -> const char* { return _text.data(); }
  auto size() const -> uint { return _text.size(); }
  
  string _text;
};

template<> struct nall::stringify<Bass::Frame> {
  stringify(Bass::Frame source) {
    auto data = source();
    _text = { "Scope: ", data.name, 
            (data.temporary) ? "tmp" : "", 
            "(", data.symbolTable.size(),") {\n" 
          };

    for(auto el : data.symbolTable) {
      _text.append("  - ", el.key, "\n");
    }
    _text.append("}");
  }

  auto data() const -> const char* { return _text.data(); }
  auto size() const -> uint { return _text.size(); }
  
  string _text;
};
