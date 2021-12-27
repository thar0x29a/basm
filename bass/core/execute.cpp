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

    for(auto& item : program) {
      exBlock(item);
    }
  } catch(string e) {
    error(e);
  }

  auto scope = frames.last();
  for(auto [key, val] : scope->symbolTable) {
    notice(key, " = ", val.value);
  }

  frames.removeRight();
  return true;
}

auto Plek::exBlock(Statement stmt) -> bool {
  // i feel like that wintergartan guy for doing this
  for(auto item : stmt->all())
  switch(item->type) {
    case st(File): 
    case st(Block): exBlock(item); break;
    case st(DeclConst): exConstDeclaration(item); break;
    case st(DeclVar): exVarDeclaration(item); break;
    default: warning("todo: ", item);
  }
 
  return true;
}

auto Plek::exConstDeclaration(Statement stmt) -> bool {
  auto scope = frames.last();
  auto left = evaluateLHS(stmt->left());
  auto right = evaluateRHS(stmt->right());

  scope->setConstant(left.getString(), right);
  return true;
}

auto Plek::exVarDeclaration(Statement stmt) -> bool {
  auto scope = frames.last();
  auto left = evaluateLHS(stmt->left());
  auto right = evaluateRHS(stmt->right());

  scope->setVariable(left.getString(), right);
  return true;
}
