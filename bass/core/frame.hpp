struct SymbolRef {
  enum class SymbolType : uint { Const, Var, Callable }; // and more ...
    
  SymbolType type;
  Value value;
  Statement ref;

  static const SymbolRef nothing() {
    static SymbolRef nothing{SymbolType::Const, {nothing}};
    return nothing;
  }
};

struct FrameElement {
  // content
  map<string, SymbolRef> symbolTable;
  Value result;
  
  auto setConstant(const string& name, const Value& val) -> void;
  auto setVariable(const string& name, const Value& val) -> void;
  auto setMacro(const string& name, Statement def) -> void;
  auto assign(const string& name, const Value& val) -> void;
};

using Frame = shared_pointer<FrameElement>;