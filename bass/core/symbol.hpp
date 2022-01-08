enum class SymbolType : uint { Value, Reference, Map };
enum class SymbolMode : uint { Const, Var};

#define symbt(t) (Bass::SymbolType::t)

struct Symbol;
struct Symbol {
  SymbolType type;
  SymbolMode mode;

  Value value;
  Statement reference;
  map<string, Symbol> references;

  auto isReference() -> bool {
    return type == SymbolType::Map;
  }

  auto isProtected() -> bool {
    return mode == SymbolMode::Const;
  }

  static const Symbol nothing() {
    static Symbol nothing{SymbolType::Value, SymbolMode::Const, {nothing}};
    return nothing;
  }

  static Symbol newMap() {
    return Symbol{SymbolType::Map, SymbolMode::Var};
  }

  static Symbol newVar(Value val) {
    return {SymbolType::Value, SymbolMode::Var, val};   
  } 
};