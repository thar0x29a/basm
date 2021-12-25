struct MacroStatement {
  Statement ref;

  MacroStatement(Statement& source) : ref(source) {}

  auto getName() -> string { return ref->leftValue().getString(); }
  auto getArguments() -> Program { return ref->content[1]->content; }
  auto getCode() -> Statement { return ref->content[2]; }
};