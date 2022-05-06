struct Result : public Value {
  auto isSymbol() -> bool { return type() == typeid(Symbol); };
  auto getSymbol() -> Symbol { return get<Symbol>(); };

  template<typename T> auto operator=(const T& value) -> Result& {
    any::operator=(value);
    return *this;
  }
  
  auto operator=(const Value& source) -> Result& {
    Value::operator=(source);
    return *this;
  }
};