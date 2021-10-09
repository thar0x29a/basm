struct StmtNode;
using Statement = shared_pointer<StmtNode>;
using Program = vector<Statement>;

enum class StmtType : uint {
  File, Block,
  Value, Identifier, Label,
  DecConst, DecVar, DecList,
  Expr, // Todo: subtypes
    Add, Sub, Mul, Div, 
  Macro, Call, List, Assignment,

  CmdInclude, CmdPrint,
  Raw
};

const vector<string> StmtNames = {
  "File", "Block", 
  "Value", "Identifier", "Label", 
  "DecConst", "DecVar", "DecList",
  "Expr",
  "Add", "Sub", "Mul", "Div", 
  "Macro", "Call", "List", "Assignment",

  "CmdInclude", "CmdPrint",
  "Raw"
};

struct StmtNode {
  StmtType type;
  FileLocation origin;
  Program content;
  
  Value value;
  Value result;

  bool leaf = true;
  bool strict = true;

  StmtNode(const StmtType t) : type(t) {};
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

  auto left() const -> Statement { return content[0]; }
  auto right() const -> Statement { return content[1]; }

  auto leftValue() const -> Value { return content[0]->value; }
  auto rightValue() const -> Value { return content[1]->value; }
};

struct Parser {
  Parser(Scanner& scanner);
  auto parseAll() -> bool;
  auto first() -> Statement { return scopes[0]; };
  static auto debug(const Program&) -> void;

protected:
  Token t;
  uint ip = 0;
  const vector<Token>& tokens;
  Program scopes;

  auto parse() -> bool;
  static auto debug(const Statement) -> void;

  auto constant() -> const Statement;
  auto variable() -> const Statement;
  auto alien() -> const Statement;

  auto macro() -> const Statement;
  auto call() -> const Statement;
  auto list() -> const Statement;
  auto defList() -> const Statement;
  auto argument() -> const Statement;
  auto label() -> const Statement;
  auto assignment() -> const Statement;
  auto block() -> const Statement;

  auto cmdPrint() -> const Statement;
  auto cmdInclude() -> const Statement;

  auto expression() -> const Statement;
  auto equality() -> const Statement;
  auto comparison() -> const Statement;
  auto term() -> const Statement;
  auto factor() -> const Statement;
  auto unary() -> const Statement;
  auto primary() -> const Statement;
  auto symbol() -> const Statement;
  auto identifier() -> const Statement;

  inline auto peek() -> const Token&;
  inline auto back() -> void;
  inline auto previous() -> const Token&;
  inline auto isAtEnd() -> bool;
  inline auto advance() -> const Token&;
  inline auto check(const TokenType& type) -> bool;
  inline auto check(const TokenType& type, uint ofs) -> bool;
  inline auto consume(const TokenType& type, const string& msg) -> const Token&;
  template <typename... Ts>
  inline auto match(Ts... xs) -> bool;
  auto synchronize() -> void;/**/
};

#define tt(t) (Bass::TokenType::t)
#define st(t) (Bass::StmtType::t)