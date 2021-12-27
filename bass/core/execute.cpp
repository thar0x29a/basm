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
      exBlock(item, frames.last());
    }
  } catch(string e) {
    error(e);
  }

  frames.removeRight();
  return true;
}

auto Plek::exBlock(Statement stmt, Frame scope) -> bool {
  // i feel like that wintergartan guy for doing this
  for(auto item : stmt->all())
  switch(item->type) {
    case st(File): 
    case st(Block): exBlock(item, scope); break;
    case st(DeclConst): exConstDeclaration(item, scope); break;
    default: warning("todo: ", item);
  }
 
  return true;
}

auto Plek::exConstDeclaration(Statement stmt, Frame scope) -> bool {
  if(!stmt->left() || !stmt->right()) throw string{"syntax error"};

  notice(stmt->left(), " = ", stmt->right());

  //1. solve left side down
  //2. solve right side down

  

  return true;
}
