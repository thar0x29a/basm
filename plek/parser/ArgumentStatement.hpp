struct ArgumentStatement {
  const Statement& ref;

  ArgumentStatement(const Statement& source) : ref(source) {
    if(ref->size()==0) {
      throw string{"found broken argument definition "};
    }
  }

  auto isCustom() -> bool { return ref->content.size() == 2; }
  auto isConst() -> bool { return ref->type == st(ConstArgument); }
  auto isReference() -> bool { return ref->type == st(RefArgument); }

  auto getCustomType() -> string { return ref->rightValue().getString(); }
  auto getName() -> string { return ref->leftValue().getString(); }
};