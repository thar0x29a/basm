enum class TokenType : uint {
  TERMINAL, 
  // Single-character tokens.
  LEFT_PAREN, RIGHT_PAREN, 
  LEFT_BRACKET, RIGHT_BRACKET, 
  LEFT_BRACE, RIGHT_BRACE,
  COMMA, DOT, MINUS, PLUS, COLON, SEMICOLON, SLASH, STAR, MINUSMINUS, PLUSPLUS,
  AND, PIPE, PERCENT, DOLLAR, WAVE, HASH,

  // One or two character tokens.
  BANG, BANG_EQUAL, EQUAL, EQUAL_EQUAL,
  GREATER, GREATER_EQUAL, LESS, LESS_EQUAL,
  LESS_LESS, GREATER_GREATER,

  // Keywords
  KW_GLOBAL, KW_PARENT, KW_EXIT, KW_MACRO,
  KW_CONST, KW_VAR, KW_DEF, KW_REF, KW_EXPR,
  KW_PRIMARY, KW_RETURN,
  KW_WHILE, KW_FOR, KW_IF, KW_ELSE, KW_BREAK, KW_CONTINUE,
  KW_NAMESPACE,

  // Commands
  CMD_PRINT, CMD_ARCH, CMD_INCLUDE,

  // Literals.
  IDENTIFIER, STRING, INTEGER, FLOAT,
  END
};

struct FileLocation {
  uint fileId = UINT_MAX;
  uint line = UINT_MAX;
  uint line_offset = UINT_MAX;

  auto toString() -> const string {
    return {fileId, ":", line, ":", line_offset};
  }
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
      insert("fun", TokenType::KW_MACRO);
      insert("return", TokenType::KW_RETURN);

      insert("constant", TokenType::KW_CONST);
      insert("const", TokenType::KW_CONST);
      insert("variable", TokenType::KW_VAR);
      insert("var", TokenType::KW_VAR);

      insert("reference", TokenType::KW_REF);
      insert("ref", TokenType::KW_REF);

      insert("define", TokenType::KW_DEF);
      insert("expression", TokenType::KW_EXPR);

      insert("while", TokenType::KW_WHILE);
      insert("for", TokenType::KW_FOR);
      insert("if", TokenType::KW_IF);
      insert("else", TokenType::KW_ELSE);
      insert("break", TokenType::KW_BREAK);
      insert("continue", TokenType::KW_CONTINUE);
      insert("namespace", TokenType::KW_NAMESPACE);

      //insert("print", TokenType::CMD_PRINT);
      //insert("arch", TokenType::CMD_ARCH);
      //insert("include", TokenType::CMD_INCLUDE);
    }
  };

  const uint uid;
  const Keywords keywords;
  vector<Token> tokens;
  const string& source;
  uint start = 0, current = 0, line = 0, line_start = 0;

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
  auto anHex() -> void;
  auto anBinary() -> void;
  auto anIdentifier() -> void;

  auto isDigit(char c) -> bool;
  auto isNumber(char c) -> bool;
  auto isHexNumber(char c) -> bool;
  auto isBinNumber(char c) -> bool;
  auto isAlpha(char c) -> bool;
  auto isAlphaNumeric(char c) -> bool;

  template<typename... P> 
  auto error(P&&... p) -> void;
};

#define tt(t) (Bass::TokenType::t)