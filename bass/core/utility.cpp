auto Plek::walkUp(const Program& what, std::function<bool (Statement, int)> with, int level) -> void {
  for(auto item : what) {
    if(!item) continue;

    // bottom up
    walkUp(item().content, with, level+1);
    with(item, level);
  }
}

auto Plek::walkDown(const Program& what, std::function<bool (Statement, int)> with, int level) -> void {
  for(auto item : what) {
    if(!item) continue;

    // top down
    if(with(item, level)) {
      walkDown(item().content, with, level+1);
    }
  }
}

auto Plek::identifier(const string& name, const Frame& scope) -> Value {
  if(auto res = scope->symbolTable.find(name)) {
    return res().value;
  }
  return {nothing};
}

auto Plek::invoke(const string& name, Statement args, const Frame& scope) -> Value {
  string id = {name, "#", args->size()};
  print(id, " invoked");
  //1. find or find not callable with this name
  //2. prepare custom scope with parameters
  //3. call with this scope!
  return {nothing};
}
