Scanner::Scanner(const uint uid, const string& source) : source(source), uid(uid) {}

auto Scanner::scanTokens() -> vector<Token> {
  tokens.reset();
  tokens.append({{uid, 0}, TokenType::TERMINAL, "BEGIN", nothing});

  while(!isAtEnd()) {
      start = current;
      scanToken();
  }

  tokens.append({{uid, line}, TokenType::END, "EOF", nothing});
  return tokens;
}

auto Scanner::scanToken() -> void {
  char c = advance();
  switch (c) {
    case '(': addToken(TokenType::LEFT_PAREN); break;
    case ')': addToken(TokenType::RIGHT_PAREN); break;
    case '[': addToken(TokenType::LEFT_BRACKET); break;
    case ']': addToken(TokenType::RIGHT_BRACKET); break;
    case '{': addToken(TokenType::LEFT_BRACE); break;
    case '}': addToken(TokenType::RIGHT_BRACE); break;
    case ',': addToken(TokenType::COMMA); break;
    case '.': addToken(TokenType::DOT); break;
    case ':': addToken(TokenType::COLON); break;
    case ';': addToken(TokenType::TERMINAL); break;
    case '*': addToken(TokenType::STAR); break;
    case '~': addToken(TokenType::WAVE); break;
    case '#': addToken(TokenType::HASH); break;

    case '&': 
      addToken(match('&') ? TokenType::ANDAND : TokenType::AND); 
      break;
    case '|': 
      addToken(match('|') ? TokenType::PIPEPIPE : TokenType::PIPE); 
      break;

    case '-':
      addToken(match('-') ? TokenType::MINUSMINUS : TokenType::MINUS); 
      break;
    case '+':
      addToken(match('+') ? TokenType::PLUSPLUS : TokenType::PLUS); 
      break;
    
    case '!':
      addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
      break;
    case '=':
      if(match('=')) addToken(TokenType::EQUAL_EQUAL);
      else if(match('>')) addToken(TokenType::GREATER_EQUAL);
      else if(match('<')) addToken(TokenType::LESS_EQUAL);
      else addToken(TokenType::EQUAL);
      break;

    case '<':
      if(match('=')) addToken(TokenType::LESS_EQUAL);
      else if(match('<')) addToken(TokenType::LESS_LESS);
      else addToken(TokenType::LESS);
      break;
    case '>':
      if(match('=')) addToken(TokenType::GREATER_EQUAL);
      else if(match('>')) addToken(TokenType::GREATER_GREATER);
      else addToken(TokenType::GREATER);
      break;

    case '\'':
      if(peekNext()=='\'') {
        addToken(TokenType::INTEGER, (int64_t)advance());
        advance(); // ending '
        break;
      }

    case '/':
      if (match('/')) {
        while (peek() != '\n' && !isAtEnd()) advance();
      } else {
        addToken(TokenType::SLASH);
      }
      break;
    
    case '\n':
      line++;
      line_start = current;
      break;
    case ' ':
    case '\r':
    case '\t':
      break;

    case '"': anString(); break;

    case '%':
      if(isBinNumber(peek())) anBinary();
      else addToken(TokenType::PERCENT);
      break;

    case '$':
      if(isHexNumber(peek())) anHex();
      else addToken(TokenType::DOLLAR);
      break;

    default:
      if (isDigit(c)) {
        anNumber();
      }
      else if (isAlpha(c)) {
        anIdentifier();
      } else {
        error( (line+1), ": Unexpected character.", c);
      }
  }
}

template<typename... P> auto Scanner::error(P&&... p) -> void {
  string s{forward<P>(p)...};
  print(stderr, terminal::color::red("error: "), s, "\n");
}

auto Scanner::isAtEnd() -> bool {
  return current >= source.size();
}

auto Scanner::advance() -> char {
  return source[current++];
}

auto Scanner::addToken(TokenType type) -> void {
  string text = source.slice(start, current-start);
  tokens.append({{uid,line,start-line_start}, type, text, text});
}

auto Scanner::addToken(TokenType type, any literal) -> void {
  string text = source.slice(start, current-start);
  tokens.append({{uid,line,start-line_start}, type, text, literal});
}

auto Scanner::match(char expected) -> bool {
  if (isAtEnd()) return false;
  if (source[current] != expected) return false;

  current++;
  return true;
}

auto Scanner::peek() -> char {
  if (isAtEnd()) return '\0';
  return source[current];
}

auto Scanner::peekNext() -> char {
  if (current + 1 >= source.size()) return '\0';
  return source[current + 1];
}

auto Scanner::anString() -> void {
  char last = 0;
  while (!isAtEnd() && !(last!='\\' && peek()=='"')) {
    if (peek() == '\n') {
      line++;
      line_start = current;
    }
    last = advance();
  }

  if (isAtEnd()) {
    error(line, "Unterminated string.");
    return;
  }

  // The closing ".
  advance();

  // Trim the surrounding quotes.
  string value = source.slice(start + 1, current-start - 2);

  // supported special characters
  value = value.replace("\\\\", "\\"); // not really needed but .. well.
  value = value.replace("\\'", "\'");
  value = value.replace("\\\"", "\"");
  value = value.replace("\\n", "\n");
  value = value.replace("\\t", "\t");

  addToken(TokenType::STRING, value);
}

auto Scanner::anHex() -> void {
  while (isHexNumber(peek())) advance();

  string lit = source.slice(start, current-start).replace("'", "");
  addToken(TokenType::INTEGER, (int64_t)toHex(lit));
}

auto Scanner::anBinary() -> void {
  while (isBinNumber(peek())) advance();

  string lit = source.slice(start, current-start).replace("'", "");
  addToken(TokenType::INTEGER, (int64_t)toBinary(lit));
}

auto Scanner::anNumber() -> void {
  char type = 'i';

  if(peek()=='b' || peek()=='o' || peek()=='x') {
    type = advance();
    start+=2;
  }
   
  if(type=='b') {
    while (isBinNumber(peek())) advance();
  }
  else if(type=='x') {
    while (isHexNumber(peek())) advance();
  }
  else {
    while (isNumber(peek())) advance();
    
    // is floating point?
    if (peek() == '.' && isNumber(peekNext())) {
      advance(); // .
      while (isNumber(peek())) advance();
      type = 'f';
    }
  }

  string lit = source.slice(start, current-start).replace("'", "");

  switch(type) {
    case 'b': addToken(TokenType::INTEGER, (int64_t)toBinary(lit)); break;
    case 'o': addToken(TokenType::INTEGER, (int64_t)toOctal(lit)); break;
    case 'x': addToken(TokenType::INTEGER, (int64_t)toHex(lit));  break;
    case 'f': addToken(TokenType::FLOAT, (double)toReal(lit)); break;
    default:
      addToken(TokenType::INTEGER, (int64_t)toInteger(lit));
  }
}

auto Scanner::anIdentifier() -> void {
  while (isAlphaNumeric(peek()) || peek()=='.') advance();

  string text = source.slice(start, current-start);

  if(text == "true") addToken(TokenType::INTEGER, (int64_t)1);
  else if(text == "false") addToken(TokenType::INTEGER, (int64_t)0);
  else if(auto type = keywords.find(text)) addToken(type());
  else addToken(TokenType::IDENTIFIER, text);
}

auto Scanner::isDigit(char c) -> bool {
  return c >= '0' && c <= '9';
}

auto Scanner::isNumber(char c) -> bool {
  return c == '\'' || isDigit(c);
}

auto Scanner::isHexNumber(char c) -> bool {
  return isDigit(c) || 
        (c >= 'a' && c <= 'f') || 
        (c >= 'A' && c <= 'F') ||
        c == '\'';
}

auto Scanner::isBinNumber(char c) -> bool {
  return (c >= '0' && c <= '1') ||
        c == '\'';
}

auto Scanner::isAlpha(char c) -> bool {
  return (c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c == '_');
}

auto Scanner::isAlphaNumeric(char c) -> bool {
  return isAlpha(c) || isDigit(c);
}

auto Scanner::debug() -> void {
  Scanner::debug(tokens);
}

auto Scanner::debug(const vector<Token>& tokens) -> void {
  for(Token token : tokens) {
    if(token.type == TokenType::TERMINAL) {
      print(",\n");
      continue;
    }
      
    if(token.type == TokenType::END) {
      print(".\n");
      return;
    }

    print("[ ", token.content, " ]");
    if(token.literal.isInt()) print("=", token.literal, " ");
    if(token.literal.isFloat()) print("=", token.literal, " ");      
  }
}