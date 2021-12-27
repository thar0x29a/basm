auto Plek::evaluateLHS(Statement stmt) -> Result {
  if(!stmt) throw string{"Syntax error!"};

  Result res{nothing};
  switch(stmt->type) {
    case st(Identifier): res = stmt->value; break;
    default: error("LHS cannot handle ", stmt);
  }

  return res;
}

auto Plek::evaluateRHS(Statement stmt) -> Result {
  if(!stmt) throw string{"Syntax error!"};

  Result res{nothing};
  switch(stmt->type) {
    case st(Value): res = stmt->value; break;
    case st(Assignment): res = evalAssign(stmt); break;
    case st(Identifier): res = evalIdentifier(stmt); break;
    case st(Call): res = evalCall(stmt); break;
    default: error("LHS cannot handle ", stmt);
  }

  return res;
}

auto Plek::evalAssign(Statement stmt) -> Result {
  auto left = evaluateLHS(stmt->left());
  auto right = evaluateRHS(stmt->right());

  assign(left.getString(), right);
  return right;
}

auto Plek::evalIdentifier(Statement stmt) -> Result {
  Result tmp{nothing};
  string identName{stmt->value.getString()};

  auto [found, scope, name, res] = find(identName);
  if(found) {
    if(res.isReference()) tmp = res;
    else tmp = res.value;
  }
  return tmp;
}

auto Plek::evalCall(Statement stmt) -> Result {
  return invoke(stmt->value, stmt->left());
}

auto Plek::calculate(Statement stmt) -> Result {
  Result result;
  /*for(auto item : stmt->content) {
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
  }/**/

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
  /*uint dataLength = 0;
  for(auto d : directives.EmitBytes) {
    if(d.token == name) {
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
  }/**/

  return true;
}