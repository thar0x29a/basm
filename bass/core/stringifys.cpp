template<> struct nall::stringify<Bass::Value> {
  stringify(Bass::Value source) {
    if(!source) _text = {"nothing"};
    else if(source.isInt()) _text = { source.getInt() };
    else if(source.isFloat()) _text = { source.getFloat() };
    else if(source.isString()) _text = { source.getString() };
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
