auto Plek::initExecution() -> void {
  origin = 0;
  base = 0;
  endian = Endian::LSB;
  for(int i=0; i<256; i++) stringTable[i] = i;

  // init scopes and rootscope
  frames.reset();
  frames.append(Frame::create(nullptr, ""));

  // prepare architecture
  architecture = new Architecture(*this);
}

auto Plek::execute() -> bool {
  try {
    //todo: better errorhandling. probl somewhere else. 

    //Parser::debug(program);

    for(auto& item : program) {
      exBlock(item);
    }
  } catch(string e) {
    error(e);
  }
/*
  auto scope = frames.last();
  for(auto [key, val] : scope->symbolTable) {
    if(!val.isMap()) {
      notice(key, " = ", val.value);
    }
    else {
      notice(key, " = {");
      for(auto [k,v] : val.references) {
        if(v.type == symbt(Value)) {
          print("\t", k, ":", v.value,"\n");
        }
        else if(v.type == symbt(Reference)) {
          print("\t", k, " -> \n");
        }
      }
      notice("}");
    }
  }/**/

  frames.removeRight();
  return true;
}

auto Plek::exBlock(Statement stmt) -> ReturnState {
  if(!stmt) error("No Instructions found");
  ReturnState result = ReturnState::Default;
  int64_t oldOrigin = 0;

  for(uint i=0; i<stmt->size(); ++i) {
    oldOrigin = pc();
    result = exStatement(stmt->content[i]);

    if(result==ReturnState::Lookahead) result = exLookahead(stmt, i, oldOrigin);
    if(result==ReturnState::Continue) return ReturnState::Continue;
    if(result==ReturnState::Break) return ReturnState::Break;
    if(result==ReturnState::Return) return ReturnState::Return;
  }
 
  return ReturnState::Default;
}

auto Plek::exLookahead(Statement stmt, uint offset, int64_t oldOrigin) -> ReturnState {
  notice("Lookahead started!");
  ReturnState result = ReturnState::Default;
  int64_t oldBase = base;
  simulate = true; // switch off writing

  // run lookaheads still block is done
  for(uint i=offset+1; i<stmt->size(); ++i) {
    result = exStatement(stmt->content[i]);

    if(result==ReturnState::Continue) break;
    if(result==ReturnState::Break) break;
    if(result==ReturnState::Return) break;
  }

  // rollback
  origin = oldOrigin;
  base = oldBase;
  simulate = false;
  seek(pc());

  // rerun core instruction.
  result = exStatement(stmt->content[offset]);
  
  // still demands lookahead -> fail.
  if(result==ReturnState::Lookahead) {
    error("Unknown value (lookahead failed).");
  }

  return result;
}

auto Plek::exStatement(Statement item) -> ReturnState {
  ReturnState result = ReturnState::Default;

  switch(item->type) {
    case st(File):
    case st(Block): exBlock(item); break;
    case st(Break): return ReturnState::Break;
    case st(Continue): return ReturnState::Continue;
    case st(Namespace): exNamespace(item); break;
    case st(DeclConst): exConstDeclaration(item); break;
    case st(Label): exLabel(item); break;
    case st(DeclVar): exVarDeclaration(item); break;
    case st(Assignment): exAssign(item); break;
    case st(MapAssignment): exMapAssign(item); break;
    case st(Macro): exFunDeclaration(item); break;
    case st(Return): result = exReturn(item); break;
    case st(IfClause): result = exIfState(item); break;
    case st(While): exWhile(item); break;
    case st(Call): if(exCall(item)) break;
    case st(Raw): result = exAssembly(item); break;
    default: warning("todo: ", item);
  }

  return result;
}

auto Plek::exConstDeclaration(Statement stmt) -> bool {
  auto left = evaluateLHS(stmt->left());
  auto right = evaluateRHS(stmt->right());

  frames.last()->setConstant(left.getString(), right);
  return true;
}

auto Plek::exVarDeclaration(Statement stmt) -> bool {
  auto left = evaluateLHS(stmt->left());
  auto right = evaluateRHS(stmt->right());

  frames.last()->setVariable(left.getString(), right);
  return true;
}

auto Plek::exFunDeclaration(Statement stmt) -> bool {
  frames.last()->setMacro({stmt});
  return true;
}


auto Plek::exCall(Statement stmt) -> bool {
  if(!stmt->left()) throw "Broken AST #79";
  try {
    invoke(stmt->value, stmt->left());
    return true;
  } catch(string e) {
    notice(e);
    return false;
  }
}

auto Plek::exReturn(Statement stmt) -> ReturnState {
  auto scope = frames.last();
  scope->result = evaluateRHS(stmt->left());
  return ReturnState::Return;
}

auto Plek::exNamespace(Statement stmt) -> bool {
  if(!stmt->left() || !stmt->right()) throw "Broken AST #103";
  auto left = evaluateLHS(stmt->left());

  string name = left.getString();
  auto scope = frames.last();
  auto subscope = Frame::create(scope, name);

  scope->addScope(subscope);
  frames.append(subscope);
    exBlock(stmt->right());
  frames.removeRight(); 

  return true;
}

auto Plek::exLabel(Statement stmt) -> bool {  
  auto left = evaluateLHS(stmt->left());
  
  if(simulate == true) {
    frames.last()->setVariable(left.getString(), {pc()});
  }
  else {
    frames.last()->setConstant(left.getString(), {pc()});
  }

  return true;
}

auto Plek::exAssign(Statement stmt) -> bool {
  evalAssign(stmt);
  return true;
}

auto Plek::exMapAssign(Statement stmt) -> bool {
  MapAssignStatement ma{stmt};
  auto id = evaluateLHS(ma.getName());
  auto key = evaluateRHS(ma.getKey());
  auto right = evaluateRHS(ma.getRight());

  auto [found, scope, name, res] = find(id);
  if(found) {
    scope->setVariable(name, right, key.getString());
  }
  else {
    frames.last()->setVariable(name, right, key.getString());
  }

  return true;
}

auto Plek::exIfState(Statement stmt) -> ReturnState {
  auto retstate = ReturnState::Default;
  bool opened = false;

  for(auto item : stmt->all()) {
    switch(item->type) {
      case st(Else): {
        if(!opened) error("ElseIf without If.");
        retstate = exElse(item); 
        break;
      }
      case st(ElseIf):
        if(!opened) error("ElseIf without If.");
      case st(If): {
        opened = true;
        retstate = exIf(item);
        break;
      }

      default: throw string{"Unexpected Token in IfState"};
    }

    if(retstate != ReturnState::Running) return retstate;
  }
  return retstate;
}

/** @return true = clause solved **/
auto Plek::exIf(Statement stmt) -> ReturnState {
  auto left = evaluateRHS(stmt->left());
  bool result = left.isTrue();

  if(result==true) {
    // invoke
    auto scope = frames.last();
    auto subscope = Frame::create(scope);
    frames.append(subscope);
    auto retstate = exBlock(stmt->right());
    frames.removeRight();
  
    if(retstate==ReturnState::Return) {
      scope->result = subscope->result;
    }
    return retstate;
  }

  // if had not been solved
  return ReturnState::Running;
}

auto Plek::exElse(Statement stmt) -> ReturnState {
  auto scope = frames.last();
  auto subscope = Frame::create(scope);

  frames.append(subscope);
  auto retstate = exBlock(stmt->left());
  frames.removeRight();

  if(retstate==ReturnState::Return) {
    scope->result = subscope->result;
  }

  return retstate;
}

auto Plek::exWhile(Statement stmt) -> bool {
  auto scope = frames.last();
  bool result = false;
  
  do {
    auto res = evaluateRHS(stmt->left());
    result = res.isTrue();
        
    if(result==true) {
      auto subscope = Frame::create(scope);
      frames.append(subscope);
      auto retstate = exBlock(stmt->right());
      frames.removeRight();

      if(retstate==ReturnState::Continue) continue;
      if(retstate==ReturnState::Break) break;
      if(retstate==ReturnState::Return) {
        scope->result = subscope->result;
        scope->returned = true;
        return true;
      }
    }
  } while(result==true);
  
  return true;
}

auto Plek::exDirective(string name, Statement items) -> ReturnState {
  uint dataLength = 0;
  for(auto d : directives.EmitBytes) {
    if(d.token == name) {
      dataLength = d.dataLength;
      break;
    }
  }

  if(dataLength==0) return ReturnState::Running;
  
  for(auto el : items->all()) {
    if(el->type != st(Raw)) {
      auto res = evaluateRHS(el);
      if(res.isNothing()) {
        write(0, dataLength); // fake write
        return ReturnState::Lookahead;
      }
      handleDirectiveValue(res, dataLength);
    }
  }

  return ReturnState::Default;
}

auto Plek::exAssembly(Statement stmt) -> ReturnState {
  ReturnState result = ReturnState::Default;
  string name = {stmt->value.getString(), " "};
  
  // are we an call-fallback?
  auto pool = (stmt->is(st(Call))) ? stmt->right() : stmt;
  bool callAtemt = (stmt->is(st(Call))==true);

  // is this an directive?
  auto dirtate = exDirective(name, pool);
  if(dirtate != ReturnState::Running) return dirtate;

  //TODO: outsource fancy debug stuff
  string text{};
  for(auto& el : pool->all()) {
    auto res = evaluateRHS(el);
    string dbug = res.getString();
    if(el->type == st(Raw)) dbug = terminal::color::magenta(dbug);
    else if(el->type == st(Identifier)) dbug = terminal::color::blue(dbug);
    else if(el->type == st(Call)) dbug = {terminal::color::cyan(dbug), "(*)"};
    //else if(el->type == st(Reference)) dbug = {"[",terminal::color::yellow(dbug),"]"};
    else if(el->type == st(Evaluation)) dbug = terminal::color::cyan("{...}");
    else if(el->type == st(Value)) dbug = terminal::color::green(dbug);
    else dbug = terminal::color::red(dbug);
    text.append(dbug);
  }
  print(terminal::color::yellow(name), text, "\n");
 

  // prepare command to be passed to the assembler
  string cmd = name;
  for(auto el : pool->all()) {
    auto res = evaluateRHS(el);
    string dbug = res.getString();
    if(res.isNothing()) {
      result = ReturnState::Lookahead;
      dbug = {(int64_t)pc()};
    }
    cmd.append(dbug);
  }
  
  // table-chan is picky. clean up your instructions!
  cmd.trimRight(" ");
  print(cmd, "\n");

  // run it!
  if(architecture->assemble(cmd)) return result;
  else if(callAtemt) error("function call finally failed (no assembly possible)");
  else error("assembly failed for: ", cmd);

  return ReturnState::Error;
}