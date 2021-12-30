auto Plek::initExecution() -> void {
  // init scopes and rootscope
  frames.reset();
  frames.append(Frame::create(nullptr, ""));

  // prepare architecture
  architecture = new Architecture(*this);
}

auto Plek::execute() -> bool {
  initExecution();

  try {
    //todo: better errorhandling. probl somewhere else. 

    //Parser::debug(program);

    for(auto& item : program) {
      exBlock(item);
    }
  } catch(string e) {
    error(e);
  }

  /*auto scope = frames.last();
  for(auto [key, val] : scope->symbolTable) {
    if(!val.isReference()) {
      notice(key, " = ", val.value);
    }
    else {
      notice(key, " = {");
      for(auto [k,v] : val.references) {
        print("\t", k, ":", v,"\n");
      }
      notice("}");
    }
  }/**/

  frames.removeRight();
  return true;
}

auto Plek::exBlock(Statement stmt) -> bool {
  if(!stmt) error("No Instructions found");
  bool inIfClause = false;

  for(auto item : stmt->all()) {
    if(frames.last()->returned) break;

    switch(item->type) {
      case st(File):
      case st(Block): exBlock(item); break;
      case st(Namespace): exNamespace(item); break;
      case st(DeclConst): exConstDeclaration(item); break;
      case st(Label): exLabel(item); break;
      case st(DeclVar): exVarDeclaration(item); break;
      case st(Assignment): exAssign(item); break;
      case st(Macro): exFunDeclaration(item); break;
      case st(Return): exReturn(item); break;
      case st(IfClause): exIfState(item); break;
      case st(While): exWhile(item); break;
      case st(Call): exCall(item); break;
      default: warning("todo: ", item);
    }

    inIfClause = false;
  }
 
  return true;
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
    //todo: store error message
    warning(e);
  }
  return false;
}

auto Plek::exReturn(Statement stmt) -> bool {
  auto scope = frames.last();
  scope->result = evaluateRHS(stmt->left());
  scope->returned = true;
  return true;
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
  frames.last()->setConstant(left.getString(), {pc()});
  return true;
}

auto Plek::exAssign(Statement stmt) -> bool {
  evalAssign(stmt);
  return true;
}

auto Plek::exIfState(Statement stmt) -> bool {
  bool opened = false;
  for(auto item : stmt->all()) {
    switch(item->type) {
      case st(Else): {
        if(!opened) error("ElseIf without If.");
        exElse(item); 
        break;
      }
      case st(ElseIf):
        if(!opened) error("ElseIf without If.");
      case st(If): {
        opened = true;
        // true = keep looking for match
        if(exIf(item)==true) break;
      }

      default: return true;
    }
  }
  return true;
}

/** @return true = there is more possible | false = were done **/
auto Plek::exIf(Statement stmt) -> bool {
  auto left = evaluateRHS(stmt->left());
  bool result = left.isTrue();
        
  if(result==true) {
    // invoke
    auto scope = frames.last();
    auto subscope = Frame::create(scope);
    frames.append(subscope);
      exBlock(stmt->right());
    frames.removeRight();

    // forward possible returning state
    if(subscope->returned) {
      scope->result = subscope->result;
      scope->returned = true;
    }
    return false;
  }

  return true;
}

auto Plek::exElse(Statement stmt) -> bool {
  auto scope = frames.last();
  auto subscope = Frame::create(scope);
  frames.append(subscope);
    exBlock(stmt->left());
  frames.removeRight();

  if(subscope->returned) {
    scope->result = subscope->result;
    scope->returned = true;
  }

  return true;
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
        exBlock(stmt->right());
      frames.removeRight();

      if(subscope->returned) {
        scope->result = subscope->result;
        scope->returned = true;
        return true;
      }
    }
  } while(result==true);
  
  return true;
}