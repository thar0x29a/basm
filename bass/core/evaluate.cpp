auto Plek::evaluate(Statement what, Evaluation mode) -> bool {
  walkUp({what}, [&](Statement stmt, int level) {
    if(stmt->result) return true;
    if(stmt->leaf) {
      if(stmt->type == st(Identifier)) {
        stmt->result = identifier(stmt->value.getString());
      } else {
        stmt->result =  stmt->value;
      }
      return true;
    }

    try {
      switch(stmt->type) {
        case st(Add):
        case st(Sub):
        case st(Mul):
        case st(Div):
          stmt->result = calculate(stmt);
          break;
        case st(Call):
          stmt->result = invoke(stmt->value, stmt->left());
          break;
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

    if(result.type() != item->result.type()) {
      throw string{"incompatible types: ", result, ":", item->result};
    }

    if(stmt->type == st(Add)) {
      if(result.isInt())        result = Value{result.getInt() + item->result.getInt()};
      else if(result.isFloat()) result = Value{result.getFloat() + item->result.getFloat()};
      else if(result.isString())result = Value{string{result.getString(), item->result.getString()}};
      else throw string{"cannot add this values"};
    }
    else if(stmt->type == st(Sub)) {
      if(result.isInt())        result = Value{result.getInt() - item->result.getInt()};
      else if(result.isFloat()) result = Value{result.getFloat() - item->result.getFloat()};
      else throw string{"cannot substract this values"};      
    }
    else if(stmt->type == st(Mul)) {
      if(result.isInt())        result = Value{result.getInt() * item->result.getInt()};
      else if(result.isFloat()) result = Value{result.getFloat() * item->result.getFloat()};
      else throw string{"cannot multiply this values"};      
    }
    else if(stmt->type == st(Div)) {
      if(result.isInt())        result = Value{result.getInt() / item->result.getInt()};
      else if(result.isFloat()) result = Value{result.getFloat() / item->result.getFloat()};
      else throw string{"cannot divide this values"};
    }
    else {
      error("unknown operation");
    }
  }

  return result;
}