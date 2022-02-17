Parser::Parser(Scanner& scanner) 
: tokens(scanner.getTokens()) {
  scopes.append(Statement::create(peek(), StmtType::File));
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
  Token t = advance();

  try {
    Statement result = statement();
    if(result) scopes.right()->append(result);
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

auto Parser::statement() -> const Statement {
  auto t = previous();

  if(t.type == tt(TERMINAL)) return nullptr;
  else if(t.type == tt(KW_CONST)) return constant();
  else if(t.type == tt(KW_VAR)) return variable();
  else if(t.type == tt(KW_MACRO)) return macro();
  
  else if(t.type == tt(LEFT_BRACE)) return blockOrEval();

  else if(t.type == tt(KW_RETURN)) return _return();
  else if(t.type == tt(KW_IF)) return ifClause();
  else if(t.type == tt(KW_WHILE)) return _while();
  else if(t.type == tt(KW_CONTINUE)) return _continue();
  else if(t.type == tt(KW_BREAK)) return _break();
  else if(t.type == tt(KW_NAMESPACE)) return _namespace();

  else if(t.type == tt(MINUS)) return anonymousLabel();
  else if(t.type == tt(MINUSMINUS)) return anonymousLabel();
  else if(t.type == tt(PLUS)) return anonymousLabel();
  else if(t.type == tt(PLUSPLUS)) return anonymousLabel();

  else if(t.type == tt(IDENTIFIER)) {
    if(check(tt(EQUAL))) {
      back(); // yes i know ..
      return assignment();
    }
    else if(check(tt(LEFT_PAREN))) {
      return callOrAlien();
    }
    else if(check(tt(COLON))) {
      return label();
    }
    else if(check(tt(LEFT_BRACKET))) {
      return mapAssignOrAlien();
    }
    else return alien();
  }

  return alien();
}

// ._."._."._."._."._."._."._."._."._."._."._."._."._."._."

auto Parser::constant() -> const Statement {
  auto t = previous();
  auto name = identOrEval();
  consume(tt(EQUAL), "expected '='");
  return Statement::create(t, StmtType::DeclConst, name, expression());
}

auto Parser::variable() -> const Statement {
  auto t = previous();
  auto name = identOrEval();
  consume(tt(EQUAL), "expected '='");
  return Statement::create(t, StmtType::DeclVar, name, expression());
}

auto Parser::_return() -> const Statement {
  return Statement::create(previous(), StmtType::Return, expression());
}

auto Parser::alien() -> const Statement {
  auto start = previous();
  auto node = Statement::create(start);
  node->strict = false;

  while(!isAtEnd()) {
    if(peek().type == tt(TERMINAL)) break;
    if(peek().origin.line > start.origin.line) break;
    if(peek().type == tt(COMMA)) {  // why do we did th
      node->append( Statement::create(advance()) );
      continue;
    }
    node->append( primary(false) );
  }

  return node;
}

auto Parser::macro() -> const Statement {
  auto t = previous();
  auto name = identifier();
  auto list = defList();
    
  consume(tt(LEFT_BRACE), "expected macro block");
  auto black = block();

  return Statement::create(t, StmtType::Macro, name, list, black);
}

auto Parser::callOrAlien() -> const Statement {
  auto offs = ip;
  auto res = call();
  auto end1 = ip;
  // rewind and handle als alien data for fallback reasons
  ip = offs;
  res->append(alien());
  auto end2 = ip;

  // use the correct ending.
  ip = (end1>end2) ? end1 : end2;

  return res;
}

auto Parser::mapAssignOrAlien() -> const Statement {
  auto offs = ip;
  try {
    back();
    return mapAssign();
  } catch(string e) {
    ip = offs;
    print("Alien!\t", e, "\n");
    return alien();
  }
}

auto Parser::mapAssign() -> const Statement {
  // ioe[primary] = expr
  auto t = previous();
  auto name = identOrEval();
  auto res = Statement::create(t, StmtType::MapAssignment, name, mapKey());
  
  // =
  consume(tt(EQUAL), string{"Invalid Map-Assign.."});

  // RHS
  res->append(expression());
  return res;
}

auto Parser::mapKey() -> const Statement {
  Statement res;
  consume(tt(LEFT_BRACKET), string{"Invalid Map-Key"});
  res = primary();
  consume(tt(RIGHT_BRACKET), string{"Invalid Map-Key"});
  return res;
}

auto Parser::mapItem() -> const Statement {
  auto t = previous();
  auto name = identOrEval();

  return Statement::create(t, StmtType::MapItem,
    name,mapKey());
}

auto Parser::call() -> const Statement {
  auto result = Statement::create(previous(), StmtType::Call);
  result->append(list());
  return result;
}

auto Parser::list() -> const Statement {
  auto start = consume(tt(LEFT_PAREN), "expected list opening");
  auto expr = Statement::create(start, StmtType::List);

  while(!check(tt(RIGHT_PAREN))) {
    expr->append(expression());
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
    expr->append(argument());
    if(!check(tt(COMMA))) break;
    advance(); // ,
  }

  consume(tt(RIGHT_PAREN), "expected end of list");
  return expr;
}

auto Parser::argument() -> const Statement {
  if(match(tt(KW_CONST))) {
    return Statement::create(previous(), StmtType::ConstArgument, identifier());
  }
  if(match(tt(KW_VAR))) {
    return Statement::create(previous(), StmtType::VarArgument, identifier());
  }
  if(match(tt(KW_REF))) {
    return Statement::create(previous(), StmtType::RefArgument, identifier());
  }
  
  return Statement::create(previous(), StmtType::VarArgument, identifier());
}

auto Parser::label() -> const Statement {
  auto newt = previous();

  // step back in order to solve this properly
  back();
  auto value = identOrEval();

  advance(); // :
  return Statement::create(newt, StmtType::Label, value);
}

auto Parser::anonymousLabel() -> const Statement {
  return Statement::create(previous(), StmtType::LabelRef);
}

auto Parser::assignment() -> const Statement {
  auto t = previous();
  auto name = identOrEval();
  consume(tt(EQUAL), "expected '='");

  return Statement::create(
          previous(), // = token
          StmtType::Assignment,
          name,
          expression());
}

/**
 * It's difficult to see if something is an evaluation
 * or not. The only way to find out is to start parsing
 * and when it fails, its obviously not an evaluation :/
 *
 * This is the worst case of parsing. I know. But ppl want it.
 *
**/
auto Parser::blockOrEval() -> const Statement {
  auto p = ip;
  Statement eva{};

  try {
    back();
    eva = evaluation();
  } catch(...) {
    print("No eval\n");
  }

  if(eva) {
    if(check(tt(COLON))) {
      return Statement::create(advance(), StmtType::Label, eva);
    }
    else if(check(tt(EQUAL))) {
      auto eq = advance(); // = 
      return Statement::create(eq, StmtType::Assignment,
              eva, expression());
    }
      
    throw string{"unbound evaluation found."};
  }
  else {
    // it was not an evaluation.
    ip = p;
    return block();
  }
}

auto Parser::block() -> const Statement {
  auto block = Statement::create(previous(), StmtType::Block);
  scopes.append(block);

  try {
    while(!check(tt(RIGHT_BRACE))) {
      advance();
      auto more = statement();
      if(more) block->append(more);
    }
  }
  catch(string e) {
    scopes.removeRight();
    throw e;
  }

  scopes.removeRight();
  advance(); // }

  return block;
}

// ._."._."._."._."._."._."._."._."._."._."._."._."._."._."


auto Parser::ifClause() -> const Statement {
  auto res = Statement::create(previous(), StmtType::IfClause, _if());
  while(check(tt(KW_ELSE))) {
    advance();
    res->append(_else());
  }
  return res;
}

auto Parser::_if() -> const Statement {
  auto start = previous();
  auto expr = expression();
  consume(tt(LEFT_BRACE), "Expected Block after if");
  auto then = block();
  return Statement::create(start, StmtType::If, expr, then);
}
auto Parser::_else() -> const Statement {
  if(check(tt(KW_IF))) {
    advance(); // remove 'else'
    auto res = _if();
    res->type = StmtType::ElseIf;
    return res;
  } else {
    auto start = previous();
    //auto then = match(tt(LEFT_BRACE)) ? block() : expression();
    consume(tt(LEFT_BRACE), "Expected Block after else");
    auto then = block();
    return Statement::create(start, StmtType::Else, then);
  }
}
auto Parser::_while() -> const Statement {
  auto start = previous();
  auto expr = expression();
  auto then = match(tt(LEFT_BRACE)) ? block() : expression();
  return Statement::create(start, StmtType::While, expr, then);
}
auto Parser::_break() -> const Statement {
  return Statement::create(previous(), StmtType::Break);
}
auto Parser::_continue() -> const Statement {
  return Statement::create(previous(), StmtType::Continue);
}

auto Parser::_namespace() -> const Statement {
  auto start = previous();
  auto name = identOrEval();
  if(!match(tt(LEFT_BRACE))) return nullptr;
  return Statement::create(start, StmtType::Namespace, name, block());
}

// ._."._."._."._."._."._."._."._."._."._."._."._."._."._."

  auto Parser::cmdPrint() -> const Statement {
    auto start = previous();
    auto res = Statement::create(start, StmtType::CmdPrint);

    while(!isAtEnd()) {
      if(check(tt(TERMINAL))) break;
      if(peek().origin.line > start.origin.line) break;
      res().append(expression());

      if(!check(tt(COMMA))) break;
      advance(); // ,
    }

    //if(!check(tt(TERMINAL)) && !isAtEnd() && peek().origin.line<=start.origin.line) 
      //throw string{"corrupted parameter list"};

    return res;
  }

  auto Parser::cmdArch() -> const Statement {
    return Statement::create(previous(), StmtType::CmdArch, identifier());
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
    auto type = (op.type==tt(EQUAL_EQUAL)) ? StmtType::CmpEqual : StmtType::CmpNotEqual;
    expr = Statement::create(op, type, expr, comparison());
  }

  return expr;
}

auto Parser::comparison() -> const Statement {
  auto expr = term();

  while(match(tt(EQUAL), tt(GREATER), tt(LESS), tt(LESS_EQUAL), tt(GREATER_EQUAL))) {
    auto op = previous();
    auto type = StmtType::Assignment;

    if     (op.type==tt(GREATER))       type = StmtType::CmpMore;
    else if(op.type==tt(LESS))          type = StmtType::CmpLess;
    else if(op.type==tt(LESS_EQUAL))    type = StmtType::CmpEqualLess;
    else if(op.type==tt(GREATER_EQUAL)) type = StmtType::CmpEqualMore;

    expr = Statement::create(op, type, expr, term());
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
  if (match(tt(BANG))) {
    auto op = previous();
    return Statement::create(op, StmtType::Banged, unary());
  }

  if (match(tt(MINUS))) {
    auto op = previous();
    return Statement::create(op, StmtType::Negative, unary());
  }

  return primary();
}

auto Parser::primary(bool allfeatures) -> const Statement {
  if (match(tt(INTEGER), tt(FLOAT), tt(STRING))) {
    return Statement::create(previous(), StmtType::Value);
  }

  if(check(tt(LEFT_BRACE))) {
    return evaluation();
  }

  if(check(tt(IDENTIFIER))) {
    return symbol();
  }

  if(allfeatures) {
    // in possible assembly lines we do not allow the
    // full syntax since it would collide with the assembly one
    if(match(tt(LEFT_PAREN))) {
      auto prev = previous();
      auto expr = expression();
      consume(tt(RIGHT_PAREN), "Expect ')' after expression.");
      return Statement::create(prev, StmtType::Grouped, expr);
    }

    if(check(tt(LEFT_BRACKET))) {
      return reference();
    }

    throw string{"not expected"};
  }
  else {
    // not all features = everything else is raw
    return Statement::create(advance());
  }
}

// something that could result in an value
auto Parser::symbol() -> const Statement {
  auto name = consume(tt(IDENTIFIER), "expected identifier");

  if(check(tt(LEFT_PAREN))) {
    return call();
  }
  if(check(tt(LEFT_BRACKET))) {
    back(); // ... :/
    return mapItem();
  }
  else {
    back(); // yes, i know ..
    return identifier();
  }
}

auto Parser::identOrEval() -> const Statement {
  if(check(tt(LEFT_BRACE))) {
    return evaluation();
  }
  else return identifier();
}

auto Parser::identifier() -> const Statement {
  return Statement::create(
    consume(tt(IDENTIFIER), "expected identifier"),
    StmtType::Identifier
  );
}

auto Parser::evaluation() -> const Statement {
  auto start = consume(tt(LEFT_BRACE), "evaluation expected {");
  auto sub = term();
  consume(tt(RIGHT_BRACE), "evaluation expected }");

  return Statement::create(start, StmtType::Evaluation, sub);
}

auto Parser::reference() -> const Statement {
  auto start = consume(tt(LEFT_BRACKET), "reference expected [");
  auto sub = term();
  consume(tt(RIGHT_BRACKET), "reference expected ]");

  return Statement::create(start, StmtType::Reference, sub);
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
    if (previous().origin.line < peek().origin.line) return;

    //TODO: more smart ways to find the new start of an statement
    // including scopes!
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