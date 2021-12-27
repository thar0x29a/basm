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
      excecuteBlock(item, frames.last());
    }
  } catch(string e) {
    error(e);
  }

  frames.removeRight();
  return true;
}

auto Plek::excecuteBlock(Statement stmt, Frame scope) -> bool {
  // i feel like that wintergartan guy for doing this
  
  return true;
}