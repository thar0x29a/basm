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

struct FrameElement;
using Frame = shared_pointer<FrameElement>;

struct FrameElement {
  // content
  map<string, SymbolRef> symbolTable;
  Value result;
  const string name;
  bool temporary;

  // references
  Frame parent = nullptr;
  map<string, Frame> children;

  FrameElement(const Frame parent) 
    : parent(parent), name("#"), temporary(true) {};
  FrameElement(const Frame parent, const string& name) 
    : parent(parent), name(name), temporary(false) {};
  
  auto setConstant(const string& name, const Value& val) -> void;
  auto setVariable(const string& name, const Value& val) -> void;
  auto setMacro(const string& name, Statement def) -> void;

  auto assign(const string& name, const Value& val) -> void;
  auto invoke(const string& name, Statement args) -> Value;

  auto addScope(const Frame frm) -> void;
};
