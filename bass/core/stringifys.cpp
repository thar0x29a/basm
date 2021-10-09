template<> struct nall::stringify<Bass::Value> {
  stringify(Bass::Value source) {
    if(source.isInt()) _text = { source.getInt() };
    else if(source.isFloat()) _text = { source.getFloat() };
    else if(source.isString()) _text = { source.getString() };
    else _text = "TODO";
  }

  auto data() const -> const char* { return _text.data(); }
  auto size() const -> uint { return _text.size(); }
  
  string _text;
};

template<> struct nall::stringify<Bass::Statement> {
  stringify(Bass::Statement source) {
    uint k = (uint)source().type;
    _text = { Bass::StmtNames[k] };
  }

  auto data() const -> const char* { return _text.data(); }
  auto size() const -> uint { return _text.size(); }
  
  string _text;
};