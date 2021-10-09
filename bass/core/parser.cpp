Parser::Parser(Scanner& scanner) 
: tokens(scanner.getTokens()) {
  scopes.append( Statement::create(StmtType::File) );
}

auto Parser::parseAll() -> bool {
  ip = 0;

  while(!isAtEnd()) parse();

  if(peek().type == tt(END)) {
    return true;
  } else {
    print("ERROR: Unclean end of file!\n");
    return false;
  }
}

auto Parser::parse() -> bool {
  t = advance();

  try {
    Statement result = nullptr;
    Statement curr_scope = scopes.right();

    if(t.type == tt(TERMINAL)) {
      return true;
    }
    else if(t.type == tt(KW_CONST)) {
      result = constant();
    }
    else if(t.type == tt(KW_VAR)) {
      result = variable();
    }
    else if(t.type == tt(KW_MACRO)) {
      result = macro();
    }
    else if(t.type == tt(LEFT_BRACE)) {
      result = block();
    }
    else if(t.type == tt(RIGHT_BRACE)) {
      if(scopes.size()<=1) throw string{"you cannot close non existent blocks"};
      auto last = scopes.takeRight();
      return true;
    }
    else if(t.type == tt(IDENTIFIER) && check(tt(EQUAL))) {
      back(); // yes i know ..
      result = assignment();
    }
    else if(t.type == tt(IDENTIFIER) && check(tt(LEFT_PAREN))) {
      result = call();
    }
    else if(t.type == tt(IDENTIFIER) && check(tt(COLON))) {
      result = label();
    }
    else if(t.type == tt(CMD_PRINT)) {
      result = cmdPrint();
    }
    else if(t.type == tt(CMD_INCLUDE)) {
      result = cmdInclude();
    }
    else {
      result = alien();
    }

    if(result) curr_scope().append(result);
    return true;
  }
  catch(const string& msg) {
    print("ERROR '", msg, "' in line ", previous().origin.line+1, ". Found '", peek().content, "'\n");
    synchronize();
  }
  catch(...) {
    print("Bad exception handling!\n");
    synchronize();
  }

  return false;
}

// ._."._."._."._."._."._."._."._."._."._."._."._."._."._."

auto Parser::constant() -> const Statement {
  auto name = consume(tt(IDENTIFIER), "expected name");
  consume(tt(EQUAL), "expected '='");
  return Statement::create(t, StmtType::DecConst, Statement::create(name), expression());
}

auto Parser::variable() -> const Statement {
  auto name = consume(tt(IDENTIFIER), "expected name");
  consume(tt(EQUAL), "expected '='");
  return Statement::create(t, StmtType::DecVar, Statement::create(name), expression());
}

auto Parser::alien() -> const Statement {
  auto start = previous();
  auto node = Statement::create(start);
  node().strict = false;

  while(!isAtEnd()) {
    if(peek().type == tt(TERMINAL)) break;
    if(peek().origin.line > start.origin.line) break;

    auto more = Statement::create(advance());
    more().strict = false;
    node().append(more);
  }
    
  return node;
}

auto Parser::macro() -> const Statement {
  auto name = consume(tt(IDENTIFIER), "expected macro name");
  auto list = defList();
    
  consume(tt(LEFT_BRACE), "expected macro block");
  auto black = block();

  return Statement::create(t, StmtType::Macro, Statement::create(name), list, black);
}

auto Parser::call() -> const Statement {
  auto result = Statement::create(previous(), StmtType::Call);
  result().append(list());
  return result;
}

auto Parser::list() -> const Statement {
  auto start = consume(tt(LEFT_PAREN), "expected list opening");
  auto expr = Statement::create(start, StmtType::List);
    
  while(!check(tt(RIGHT_PAREN))) {
    expr().append(expression());
    if(!check(tt(COMMA))) break;
    advance(); // ,
  }

  consume(tt(RIGHT_PAREN), "expected end of list");
  return expr;
}  

auto Parser::defList() -> const Statement {
  auto start = consume(tt(LEFT_PAREN), "expected list opening");
  auto expr = Statement::create(start, StmtType::DecList);
    
  while(!check(tt(RIGHT_PAREN))) {
    expr().append(argument());
    if(!check(tt(COMMA))) break;
    advance(); // ,
  }

  consume(tt(RIGHT_PAREN), "expected end of list");
  return expr;
}

auto Parser::argument() -> const Statement {
  if(match(tt(KW_CONST), tt(KW_VAR))) {
    return Statement::create(previous(), identifier());
  }
  return symbol();
}

auto Parser::label() -> const Statement {
  auto newt = previous();
  advance(); // :
  return Statement::create(newt, StmtType::Label);
}

auto Parser::assignment() -> const Statement {
  auto name = consume(tt(IDENTIFIER), "expected name");
  consume(tt(EQUAL), "expected '='");
  
  return Statement::create(
          previous(), 
          StmtType::Assignment,
          Statement::create(name),
          expression());
}

auto Parser::block() -> const Statement {
  auto res = Statement::create(previous(), StmtType::Block);
  scopes.append(res);
  return res;
}

// ._."._."._."._."._."._."._."._."._."._."._."._."._."._."

  auto Parser::cmdPrint() -> const Statement {
    auto start = previous();
    auto res = Statement::create(start, StmtType::CmdPrint);

    while(!isAtEnd()) {
      if(peek().type == tt(TERMINAL)) break;
      if(peek().origin.line > start.origin.line) break;
      res().append(expression());

      if(peek().type != tt(COMMA)) break;
      advance(); // ,
    }

    if(!isAtEnd() && peek().origin.line<=start.origin.line) 
      throw string{"corrupted parameter list"};

    return res;
  }

  auto Parser::cmdInclude() -> const Statement {
    return Statement::create(previous(), StmtType::CmdInclude, primary());
  }


// ._."._."._."._."._."._."._."._."._."._."._."._."._."._."

auto Parser::expression() -> const Statement {
  return equality();
}

auto Parser::equality() -> const Statement {
  auto expr = comparison();
    
  while(match(tt(BANG_EQUAL), tt(EQUAL_EQUAL))) {
    auto op = previous();
    expr = Statement::create(op, StmtType::Expr, expr, comparison());
  }

  return expr;
}

auto Parser::comparison() -> const Statement {
  auto expr = term();

  while(match(tt(GREATER), tt(EQUAL), tt(LESS), tt(LESS_EQUAL))) {
    auto op = previous();
    expr = Statement::create(op, StmtType::Expr, expr, term());
  }

  return expr;
}

auto Parser::term() -> const Statement {
  auto expr = factor();

  while(match(tt(MINUS), tt(PLUS))) {
    auto op = previous();
    auto type = (op.type == tt(MINUS)) ? st(Sub) : st(Add);

    expr = Statement::create(op, type, expr, factor());
  }

  return expr;
}

auto Parser::factor() -> const Statement {
  auto expr = unary();

  while(match(tt(SLASH), tt(STAR))) {
    auto op = previous();
    auto type = (op.type == tt(STAR)) ? st(Mul) : st(Div);
    expr = Statement::create(op, type, expr, unary());
  }

  return expr;
}

auto Parser::unary() -> const Statement {
  if (match(tt(BANG), tt(MINUS))) {
    auto op = previous();
    return Statement::create(op, StmtType::Expr, unary());
  }

  return primary();
}

auto Parser::primary() -> const Statement {
  if (match(tt(INTEGER), tt(FLOAT), tt(STRING))) {
    return Statement::create(previous(), StmtType::Value);
  }

  if(check(tt(IDENTIFIER))) {
    return symbol();
  }

  throw string{"not expected"};
}

// something that could result in an value
auto Parser::symbol() -> const Statement {
  auto name = consume(tt(IDENTIFIER), "expected identifier");

  if(check(tt(LEFT_PAREN))) {
    return call();
  }
  else {
    back(); // yes, i know ..
    return identifier();
  }
}

auto Parser::identifier() -> const Statement {
  return Statement::create(
    consume(tt(IDENTIFIER), "expected identifier"),
    StmtType::Identifier
  );
}

// ._."._."._."._."._."._."._."._."._."._."._."._."._."._."


inline auto Parser::peek() -> const Token& {
  return tokens[ip]; 
}

inline auto Parser::back() -> void { ip--; }

inline auto Parser::previous() -> const Token& {
  return tokens[ip-1]; 
}

inline auto Parser::isAtEnd() -> bool {
  return (peek().type == tt(END));
}

inline auto Parser::advance() -> const Token& {
  if(!isAtEnd()) ip++;
  return previous(); 
}

inline auto Parser::check(const TokenType& type) -> bool {
  if(isAtEnd()) return false; 
  return peek().type == type; 
}

inline auto Parser::consume(const TokenType& type, const string& msg) -> const Token& {
  if(check(type)) return advance();
  throw msg;
}

template <typename... Ts>
inline auto Parser::match(Ts... xs) -> bool {
  if( (check(xs) || ...) ) {
    advance();
    return true;
  }
  return false;
}

auto Parser::synchronize() -> void {
  advance();

  while (!isAtEnd()) {
    if (previous().type == tt(TERMINAL)) return;
    if (previous().origin.line<peek().origin.line) return;

    //TODO: more smart ways to find the new start of an statement
    advance();
  }
}

// ._."._."._."._."._."._."._."._."._."._."._."._."._."._."

auto Parser::debug(const Program& prog) -> void {
  for(auto stmt : prog) {
    print(stmt().origin.line +1, "\t");
    debug(stmt);
    print("\n");    
  }
}

auto Parser::debug(Statement item) -> void {
  if(!item) return;

  if(item().leaf) {
    if(item().type == st(Raw)) {
      print("? ", item);
    }
    else {
      print("'", item,"'");
    }
  }
  else {
    print("[", item, " ");

    if(item().is(st(File)) || item().is(st(Block))) {
      for(auto el : item().all()) {
        if(!el) continue;
        print("\n\t", el().origin.line+1, "\t");
        debug(el);
      }
      print("\n\t");
    }
    else {
      for(auto el : item().all()) {
        if(!el) continue;
        print(", ");
        debug(el);
      }
    }

    print("]");
  }
}