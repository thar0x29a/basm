struct MacroStatement {
  Statement ref;
  const int NAME_INDEX = 0;
  const int LIST_INDEX = 1;
  const int CODE_INDEX = 2;

  MacroStatement(Statement source) : ref(source) {
    if(!ref->content[NAME_INDEX]) {
      throw string{"found broken macro def: invalid name"};
    }
    if(!ref->content[LIST_INDEX] || ref->content[LIST_INDEX]->type != st(DecList)) {
      throw string{"found broken macro def: invalid argument declaration"};
    }
    if(!ref->content[CODE_INDEX] || ref->content[CODE_INDEX]->type != st(Block)) {
      throw string{"found broken macro def: no code block"};
    }
  }

  auto getName() -> string { return ref->leftValue().getString(); }
  auto getArguments() -> Program { return ref->content[LIST_INDEX]->content; }
  auto getArgCount() -> uint { return ref->content[LIST_INDEX]->content.size(); }
  auto getCode() -> Statement { return ref->content[CODE_INDEX]; }
};