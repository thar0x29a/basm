struct LabelRefStatement {
  Statement& ref;

  LabelRefStatement(Statement& source) : ref(source) {
    if(source->type != st(LabelRef)) 
      throw string{"Object is no LabelRef"};
    if(!source->value.isString()) {
      throw string{"found broken LabelRef def: no content"};
    }
  }

  auto getOffset() -> int {
    string v = ref->value.getString();
    if(v.equals("-")) return -1;
    if(v.equals("+")) return 1;
    if(v.equals("--")) return -2;
    if(v.equals("++")) return 2;
    return 0;
  }

  auto value() -> string { return ref->value.getString(); }
};