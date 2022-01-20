auto Plek::evaluateLHS(Statement stmt) -> Result {
  if(!stmt) throw string{"Syntax error! LHS was empty."};

  Result res{nothing};
  switch(stmt->type) {
    case st(Identifier): res = stmt->value; break;
    case st(Evaluation): res = evaluateRHS(stmt->left()); break;
    default: error("LHS cannot handle ", stmt);
  }

  return res;
}

auto Plek::evaluateRHS(Statement stmt) -> Result {
  if(!stmt) throw string{"Syntax error! RHS was empty."};

  Result res{nothing};
  switch(stmt->type) {
    case st(Raw):
    case st(Value): res = stmt->value; break;
    case st(Assignment): res = evalAssign(stmt); break;
    case st(Identifier): res = evalIdentifier(stmt); break;
    case st(Call): res = evalCall(stmt); break;
    case st(MapItem): res = evalMapItem(stmt); break;
    case st(Add):
    case st(Sub):
    case st(Mul):
    case st(Div):
    case st(CmpEqual):
    case st(CmpMore):
    case st(CmpLess):
    case st(CmpEqualMore):
    case st(CmpEqualLess):
    case st(CmpNotEqual): {
      res = calculate(stmt);
      break;
    }
    case st(Negative): {
      res = evaluateRHS(stmt->left()).negate();
      break;
    }
    case st(Evaluation):
    case st(Grouped): {
      res = evaluateRHS(stmt->left());
      break;    
    }
    default: error("RHS cannot handle ", stmt);
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

auto Plek::evalMapItem(Statement stmt) -> Result {
  if(stmt->content.size()<2) throw string{"Incomplete map statement."};

  auto ref = evaluateRHS(stmt->content[0]);
  if(!ref.isSymbol()) throw string{stmt->content[0]->value, " is not an map."};

  auto key = evaluateRHS(stmt->content[1]).getString();
  return { ref.getSymbol().get(key) };
}

auto Plek::calculate(Statement stmt) -> Result {
  Result result;
  for(auto item : stmt->content) {
    Result ir = evaluateRHS(item);
    if(!ir) throw string{"Parameter had not been solved: ", item, " ", item->value, " -> ", ir};
    if(!result) { result = ir; continue; }

    if(result.isString()) {
      // as long the target is a string, value can append everything on it.
    }
    else if(result.type() != ir.type()) {
      throw string{"incompatible types: ", result, ":", ir};
    }

    if(result.isInt()) result = calculate(stmt->type, result.getInt(), ir.getInt());
    else if(result.isFloat()) result = calculate(stmt->type, result.getFloat(), ir.getFloat());
    else if(result.isString()) { 
      string a = result.getString();
      string b = ir.getString();

      if(stmt->type == st(Add)) result = Result{string{a,b}};
      else if(stmt->type == st(CmpEqual)) result = Result{(int64_t)(a==b)};
      else if(stmt->type == st(CmpNotEqual))  result = Result{(int64_t)(a!=b)};
      else error("Type not supported");
    }
    else error("Type not supported");
  }

  return result;
}

template <typename T>
auto Plek::calculate(StmtType type, const T& a, const T& b) -> Result {
  Result result{nothing};
  
  if(type == st(Add))      result = Result{a+b};
  else if(type == st(Sub)) result = Result{a-b};
  else if(type == st(Mul)) result = Result{a*b};
  else if(type == st(Div)) result = Result{a/b};

  else if(type == st(CmpEqual))     result = Result{(int64_t)(a==b)};
  else if(type == st(CmpMore))      result = Result{(int64_t)(a>b)};
  else if(type == st(CmpLess))      result = Result{(int64_t)(a<b)};
  else if(type == st(CmpEqualMore)) result = Result{(int64_t)(a>=b)};
  else if(type == st(CmpEqualLess)) result = Result{(int64_t)(a<=b)};
  else if(type == st(CmpNotEqual))  result = Result{(int64_t)(a!=b)};

  else error("unknown operation");

  return result;
}

auto Plek::handleDirective(string name, Statement items) -> bool {
  uint dataLength = 0;
  for(auto d : directives.EmitBytes) {
    if(d.token == name) {
      dataLength = d.dataLength;
      break;
    }
  }

  if(dataLength==0) return false;
  
  for(auto el : items->all()) {
    if(el->type != st(Raw)) {
      auto res = evaluateRHS(el);
      if(!res.isInt()) continue;
      write(res.getInt(), dataLength);
    }
  }

  return true;
}