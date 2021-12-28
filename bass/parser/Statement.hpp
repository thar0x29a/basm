enum class StmtType : uint {
  File, Block, Grouped,
  Value, Identifier, Label, Evaluation, Reference,

  // declarations
  DeclConst, DeclVar, DecList,
  ConstArgument, VarArgument, RefArgument,
  Expr, Negative, Banged,
    Add, Sub, Mul, Div, 
  Macro, Call, List, Assignment,
  Return,
  While, If, ElseIf, Else, Break, Continue, 
  Namespace,
  
  CmpEqual, CmpLess, CmpMore, CmpEqualLess, CmpEqualMore, CmpNotEqual,
  
  CmdInclude, CmdPrint, CmdArch,
  
  Raw
};

const vector<string> StmtNames = {
  "File", "Block", "Grouped",
  "Value", "Identifier", "Label", "Evaluation", "Reference",
  "DeclConst", "DeclVar", "DecList",
  "ConstArgument", "VarArgument", "RefArgument",
  "Expr", "Negative", "Banged",
  "Add", "Sub", "Mul", "Div", 
  "Macro", "Call", "List", "Assignment",
  "Return",
  "While", "If", "ElseIf", "Else", "Break", "Continue",
  "Namespace",

  "CmpEqual", "CmpLess", "CmpMore", "CmpEqualLess", "CmpEqualMore", "CmpNotEqual",
  "CmdInclude", "CmdPrint", "CmdArch",

  "Raw"
};

struct StmtNode {
  StmtType type;
  FileLocation origin;
  Program content;
  
  Value value;

  bool leaf = true;
  bool strict = true;

  StmtNode(const Token& op) : value(op.literal), origin(op.origin), type(StmtType::Raw) {};
  StmtNode(const Token& op, const StmtType t) : value(op.literal), origin(op.origin), type(t) {};
  
  template <typename... Ts>
  StmtNode(const Token& op, const StmtType t, Ts... xs) 
  : value(op.literal), origin(op.origin), type(t), leaf(false) {
    content.append(Program{xs...});
  };

  template <typename... Ts>
  StmtNode(const Token& op, Ts... xs) 
  : value(op.literal), origin(op.origin), type(StmtType::Raw), leaf(false) {
    content.append(Program{xs...});
  };

  auto append(const Statement& item) -> void {
    leaf = false;
    content.append(item);
  }

  auto all() const -> Program { return content; }
  auto is(StmtType t) -> bool { return type == t; }
  auto isReference() -> bool { 
    return type == st(Identifier)
      || type == st(Evaluation);
  }

  auto left() const -> Statement {
    if(content.size()<1) return nullptr;
    return content[0]; 
  }
  auto right() const -> Statement {
    if(content.size()<2) return nullptr;
    return content[1]; 
  }

  auto leftValue() const -> Value {
    if(content.size()<1) return {nothing};
    return content[0]->value; 
  }
  auto rightValue() const -> Value {
    if(content.size()<2) return {nothing};
    return content[1]->value;
  }

  auto size() const -> uint { return content.size(); }
};