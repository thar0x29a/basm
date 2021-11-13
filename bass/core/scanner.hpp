enum class TokenType : uint {
  TERMINAL, 
  // Single-character tokens.
  LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
  COMMA, DOT, MINUS, PLUS, COLON, SEMICOLON, SLASH, STAR,
  AND, PIPE, PERCENT, DOLLAR, WAVE, HASH,

  // One or two character tokens.
  BANG, BANG_EQUAL, EQUAL, EQUAL_EQUAL,
  GREATER, GREATER_EQUAL, LESS, LESS_EQUAL,

  // Keywords
  KW_GLOBAL, KW_PARENT, KW_EXIT, KW_MACRO,
  KW_CONST, KW_VAR, KW_DEF, KW_EVAL, KW_EXPR,
  KW_PRIMARY, KW_RETURN,
  KW_WHILE, KW_IF, KW_ELSE, KW_BREAK, KW_CONTINUE,

  // Commands
  CMD_PRINT, CMD_INCLUDE,

  // Literals.
  IDENTIFIER, STRING, INTEGER, FLOAT,
  END
};

struct FileLocation {
  uint fileId = UINT_MAX;
  uint line = UINT_MAX;
};

struct Token {
  FileLocation origin;
  TokenType type;
  string content;
  Value literal;
};

struct Scanner {
  Scanner(uint fileId, const string& source);
  auto scanTokens() -> vector<Token>;
  auto getTokens() -> vector<Token>& { return tokens; }

  auto debug() -> void;
  static auto debug(const vector<Token>& tokens) -> void;

protected:
  struct Keywords : map<string, TokenType> {
    Keywords() {
      insert("global", TokenType::KW_GLOBAL);
      insert("parent", TokenType::KW_PARENT);
      insert("exit", TokenType::KW_EXIT);
      insert("macro", TokenType::KW_MACRO);
      insert("return", TokenType::KW_RETURN);

      insert("constant", TokenType::KW_CONST);
      insert("const", TokenType::KW_CONST);
      insert("variable", TokenType::KW_VAR);
      insert("var", TokenType::KW_VAR);

      insert("define", TokenType::KW_DEF);
      insert("evaluate", TokenType::KW_EVAL);
      insert("expression", TokenType::KW_EXPR);

      insert("while", TokenType::KW_WHILE);
      insert("if", TokenType::KW_IF);
      insert("else", TokenType::KW_ELSE);
      insert("break", TokenType::KW_BREAK);
      insert("continue", TokenType::KW_CONTINUE);

      insert("print", TokenType::CMD_PRINT);
      insert("include", TokenType::CMD_INCLUDE);
    }
  };

  const uint uid;
  const Keywords keywords;
  vector<Token> tokens;
  const string& source;
  uint start = 0, current = 0, line = 0;

  auto scanToken() -> void;

  auto isAtEnd() -> bool;
  auto advance() -> char;

  auto addToken(TokenType type) -> void;
  auto addToken(TokenType type, any literal) -> void;

  auto match(char expected) -> bool;
  auto peek() -> char;
  auto peekNext() -> char;

  auto anString() -> void;
  auto anNumber() -> void;
  auto anIdentifier() -> void;

  auto isDigit(char c) -> bool;
  auto isNumber(char c) -> bool;
  auto isAlpha(char c) -> bool;
  auto isAlphaNumeric(char c) -> bool;

  template<typename... P> 
  auto error(P&&... p) -> void;
};