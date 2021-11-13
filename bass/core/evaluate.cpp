auto Plek::evaluate(Statement what, Evaluation mode) -> bool {
  auto scope = frames.right();
  
  walkUp({what}, [&](Statement stmt, int level) {
    if(stmt->leaf) {
      if(stmt->type == st(Identifier)) {
        stmt->result = identifier(stmt->value.getString());
      }
      else {
        stmt->result = stmt->value;
      }
      return true;
    }

    try {
      switch(stmt->type) {
        case st(Add):
        case st(Sub):
        case st(Mul):
        case st(Div):
        case st(CmpEqual):
        case st(CmpMore):
        case st(CmpLess):
        case st(CmpEqualMore):
        case st(CmpEqualLess):
        case st(CmpNotEqual):
          stmt->result = calculate(stmt);
          break;
        case st(Call):
          stmt->result = invoke(stmt->value, stmt->left());
          break;
        case st(Negative):
          stmt->result = stmt->leftResult().negate();
          break;
        case st(Grouped):
          stmt->result = stmt->leftResult();
          break;
        case st(Assignment):
          stmt->result = stmt->rightResult();
          assign(
            stmt->leftValue().getString(),
            stmt->rightResult()
          );
        default:
          //notice(stmt);
          break;
      }
    } catch(string e) {
      error(e);
    }

    return true;
  });

  return true;
}

auto Plek::calculate(Statement stmt) -> Value {
  Value result;
  for(auto item : stmt->content) {
    if(!item->result) throw string{"Parameter had not been solved: ", item, " ", item->value, " -> ", item->result};
    if(!result) { result = item->result; continue; }

    // todo: handle with visitor patterns
    if(result.type() != item->result.type()) {
      throw string{"incompatible types: ", result, ":", item->result};
    }

    if(result.isInt()) result = calculate(stmt->type, result.getInt(), item->result.getInt());
    else if(result.isFloat()) result = calculate(stmt->type, result.getFloat(), item->result.getFloat());
    //else if(result.isString()) { }
    else error("Type not supported"); 
  }

  return result;
}

template <typename T>
auto Plek::calculate(StmtType type, const T& a, const T& b) -> Value {
  Value result{nothing};
  
  if(type == st(Add))      result = Value{a+b};
  else if(type == st(Sub)) result = Value{a-b};
  else if(type == st(Mul)) result = Value{a*b};
  else if(type == st(Div)) result = Value{a/b};

  else if(type == st(CmpEqual))     result = Value{(int64_t)(a==b)};
  else if(type == st(CmpMore))      result = Value{(int64_t)(a>=b)};
  else if(type == st(CmpLess))      result = Value{(int64_t)(a<=b)};
  else if(type == st(CmpEqualMore)) result = Value{(int64_t)(a>=b)};
  else if(type == st(CmpEqualLess)) result = Value{(int64_t)(a<=b)};
  else if(type == st(CmpNotEqual))  result = Value{(int64_t)(a!=b)};

  else error("unknown operation");

  return result;
}