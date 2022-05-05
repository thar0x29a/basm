struct Value;

struct Value : public any {
  struct Custom {
    string name;
    shared_pointer<Value> value;

    Custom(string name) : name(name), value(shared_pointer<Value>::create()) {}
    Custom(string name, Value val) : name(name), value(shared_pointer<Value>::create(val)) {}
  };

  auto isInt() -> bool { return type() == typeid(int64_t); };
  auto isFloat() -> bool { return type() == typeid(double); };
  auto isString() -> bool { return type() == typeid(string); };
  auto isNothing() -> bool { return type() == typeid(nothing); };
  auto isCustom() -> bool { return type() == typeid(Custom); };
  auto isCustom(string name) -> bool { 
    return isCustom() ? name.equals(getCustom().name) : false;
  }

  auto getInt() -> int64_t { return get<int64_t>(); };
  auto getFloat() -> double { return get<double>(); };
  auto getCustom() -> Custom { return get<Custom>(); };
  auto getString() -> string { 
    if(isString()) return get<string>(); 
    else if(isInt()) return { getInt() };
    else if(isFloat()) return { getFloat() };
    else if(isCustom()) return getCustom().name;
    else return { "null" };
    //else throw "unknown state";
  };

  auto negate() -> Value {
    if(isInt()) return {getInt() * -1};
    else if(isFloat()) return {getFloat() * -1};
    //TODO: custom negation?
    else throw string{"cannot negate ", type().name()};
  };

  auto isTrue() -> bool {
    if(type() == typeid(nothing)) return false;
    else if(isInt() && getInt()==0) return false;
    else if(isFloat() && getFloat()==0) return false;
    else if(isString() && getString().length()==0) return false;
    return true;
  }
};