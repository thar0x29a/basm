auto Plek::evaluate(Statement what, EvaluationMode mode) -> bool {
  auto scope = frames.right();
  
  walkUp({what}, [&](Statement stmt, int level) {
    if(stmt->leaf) {
      if(stmt->type == st(Identifier)) {
        if(mode != EvaluationMode::LeftSide) {
          stmt->result = identifier(stmt->value.getString());
        } else {
          stmt->result = stmt->value;  
        }
      }
      else {
        // should not be needed due on leafs result should be value allready.
        // but we make sure.
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
        case st(Reference):
          stmt->result = identifier(stmt->leftResult().getString());
          break;
        case st(Evaluation):
        case st(Grouped):
        case st(Label):
          stmt->result = stmt->leftResult();
          break;
        case st(Assignment):
          stmt->result = stmt->rightResult();
          assign(
            stmt->leftValue().getString(),
            stmt->rightResult()
          );
        case st(DeclVar):
        case st(DeclConst):
        case st(List):
        case st(CmdPrint):
        case st(CmdArch):
        case st(CmdInclude):
        case st(Return):
          // handled in excecuteBlock
          break;
        default:
          notice("unknown: ", stmt);
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
    if(result.isString()) {
      // as long the target is a string, value can handle it.
    }
    else if(result.type() != item->result.type()) {
      throw string{"incompatible types: ", result, ":", item->result};
    }

    if(result.isInt()) result = calculate(stmt->type, result.getInt(), item->result.getInt());
    else if(result.isFloat()) result = calculate(stmt->type, result.getFloat(), item->result.getFloat());
    else if(result.isString()) { 
      string a = result.getString();
      string b = item->result.getString();

      if(stmt->type == st(Add)) result = {string{a,b}};
      else error("Type not supported");
    }
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

auto Plek::handleDirective(string name, Statement items) -> bool {
  uint dataLength = 0;
  for(auto d : directives.EmitBytes) {
    if(d.token == name) {
      notice("found directive ", name);
      dataLength = d.dataLength;
      break;
    }
  }

  if(dataLength==0) return false;
  
  for(auto el : items->all()) {
    if(el->type != st(Raw)) {
      evaluate(el);
      if(!el->result.isInt()) continue;
      write(el->result.getInt(), dataLength);
    }
  }

  return true;
}