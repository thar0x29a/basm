struct MapAssignStatement {
  const Statement& ref;
  const int NAME_INDEX = 0;
  const int KEY_INDEX = 1;
  const int RIGHT_INDEX = 2;

  MapAssignStatement(const Statement& source) : ref(source) {
    if(!ref->content[NAME_INDEX]) {
      throw string{"found broken map assignment: invalid name"};
    }
    if(!ref->content[KEY_INDEX]) {
      throw string{"found broken map assignment: invalid key"};
    }
    if(!ref->content[RIGHT_INDEX]) {
      throw string{"found broken map assignment: no right side"};
    }
  }

  auto getName() -> Statement { return ref->content[NAME_INDEX]; }
  auto getKey() -> Statement { return ref->content[KEY_INDEX]; }
  auto getRight() -> Statement { return ref->content[RIGHT_INDEX]; }
};