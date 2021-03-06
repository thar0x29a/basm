enum class SymbolType : uint { Value, Reference, Map };
enum class SymbolMode : uint { Const, Var};

#define symbt(t) (Bass::SymbolType::t)

struct Symbol;
using SymbolRef = shared_pointer<Symbol>;

struct Symbol {
  SymbolType type;
  SymbolMode mode;

  Value value{nothing};
  Statement reference;
  map<string, Symbol> references;

  auto isValue() -> bool {
    return type == SymbolType::Value;
  }

  auto isReference() -> bool {
    return type == SymbolType::Reference;
  }

  auto isMap() -> bool {
    return type == SymbolType::Map;
  }

  auto isProtected() -> bool {
    return mode == SymbolMode::Const;
  }

  auto get(string key) -> Value {
    if(type != SymbolType::Map) throw string{"cannot access value as map"};
    if(auto res = references.find(key)) return res->value;
    else return {nothing};
  }

  static Symbol newMap() {
    return Symbol{SymbolType::Map, SymbolMode::Var};
  }

  static Symbol newVar(Value val) {
    return {SymbolType::Value, SymbolMode::Var, val};   
  }

  static Symbol newConst(Value val) {
    return {SymbolType::Value, SymbolMode::Const, val};   
  }
};