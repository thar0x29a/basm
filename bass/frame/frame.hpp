struct FrameElement;
using Frame = shared_pointer<FrameElement>;

struct FrameElement {
  // content
  map<string, Symbol> symbolTable;
  Result result;
  const string name;
  bool temporary;
  bool returned = false;

  // references
  Frame parent = nullptr;
  map<string, Frame> children;

  FrameElement(const Frame parent) 
    : parent(parent), name("#"), temporary(true) {};
  FrameElement(const Frame parent, const string& name) 
    : parent(parent), name(name), temporary(false) {};
  
  auto setConstant(const string& name, Result val) -> void;
  auto setVariable(const string& name, Result val) -> void;
  auto setMacro(MacroStatement def) -> void;

  auto assign(const string& name, Result val) -> void;
  auto addScope(const Frame frm) -> void;
};
