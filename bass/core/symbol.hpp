enum class SymbolType : uint { Const, Var, Map };
#define symbt(t) (Bass::SymbolType::t)

struct Symbol {
  SymbolType type;
  Value value;
  map<string,Statement> references;

  auto isReference() -> bool {
    return type == SymbolType::Map;
  }

  static const Symbol nothing() {
    static Symbol nothing{SymbolType::Const, {nothing}};
    return nothing;
  }

  static Symbol asMap() {
    return {SymbolType::Map};
  }
};