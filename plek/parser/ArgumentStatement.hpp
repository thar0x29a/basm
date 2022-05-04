struct ArgumentStatement {
  const Statement& ref;

  ArgumentStatement(const Statement& source) : ref(source) {
    if(!ref->content.size()==0) {
      throw string{"found broken argument def"};
    }
  }

  auto isCustom() -> bool { return ref->content.size() == 2; }
  auto isReference() -> bool { return ref->type == st(RefArgument); }

  auto getCustomType() -> string { return ref->leftValue().getString(); }
  auto getName() -> string { 
    return isCustom()
      ? ref->rightValue().getString()
      : ref->leftValue().getString();
  }
};