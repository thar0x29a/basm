struct ForStatement {
  Statement& ref;
  const int VAR_INDEX = 0;
  const int SOURCE_INDEX = 1;
  const int CODE_INDEX = 2;

  ForStatement(Statement& source) : ref(source) {
    if(source->type != st(For)) 
      throw string{"Object is no For-Loop"};

    if(source->all().size()!=3) 
      throw string{"Incomplete For Object"};
  }

  auto getVariable() -> Statement { return ref->content[VAR_INDEX]->left(); }
  auto getSource() -> Statement { return ref->content[SOURCE_INDEX]; }
  auto getCode() -> Statement { return ref->content[CODE_INDEX]; }

  auto isVarConstant() -> bool {
    return ref->content[VAR_INDEX]->type == st(DeclConst);
  }
};