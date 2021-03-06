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
  /** used on 'this' */
  shared_pointer<Symbol> context = nullptr;
  map<string, Frame> children;
  vector<Result> labels;
  int labelp{0};

  FrameElement(const Frame parent) 
    : parent(parent), name("#"), temporary(true) {};
  FrameElement(const Frame parent, const string& name) 
    : parent(parent), name(name), temporary(false) {};
  
  auto setConstant(const string& name, Result val) -> void;
  auto setVariable(const string& name, Result val) -> void;
  auto setVariable(const string& name, Result val, string id) -> void;
  auto setMacro(MacroStatement def) -> void;

  auto assign(const string& name, Result val) -> void;
  auto addScope(const Frame frm) -> void;

  auto addLabel(const Result&) -> void;
};
