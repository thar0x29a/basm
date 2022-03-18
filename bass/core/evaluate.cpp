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
    case st(CmpNotEqual):
    case st(LogicAnd):
    case st(LogicOr):
    case st(LogicShiftLeft):
    case st(LogicShiftRight):
    case st(LogicModulo): {
      res = calculate(stmt);
      break;
    }
    case st(LabelRef): {
      res = evaluateLabelRef(stmt);
      break;
    }
    case st(Negative): {
      res = evaluateRHS(stmt->left()).negate();
      break;
    }
    case st(Reference): {
      res = evalReference(stmt);
      break;
    }
    case st(Evaluation): {
      res = evalEvaluation(stmt);
      break;
    }
    case st(Grouped): {
      res = evaluateRHS(stmt->left());
      break;    
    }
    default: error("RHS cannot handle ", stmt);
  }

  return res;
}

auto Plek::evalReference(Statement stmt) -> Result {
  auto key = evaluateRHS(stmt->left());
  auto [found, scope, name, res] = find(key.getString());

  if(!found) error("Reference ", key, " not found.");
  return {res.value};
}

auto Plek::evaluateLabelRef(Statement stmt) -> Result {
  Result res{nothing};
  LabelRefStatement ref{stmt};
  int offs = ref.getOffset();
  auto scope = frames.last();
  int total = scope->labels.size();
  int current = scope->labelp;
  string emsg{"relative label access failed: not enought labels!"};

  if(offs<0) {
    if(current+offs < 0) error(emsg);
    res = scope->labels[current+offs];
  }
  else if(offs>0) {
    if(current+offs <= total) res = scope->labels[current+offs-1];
    else if(simulate) res = Value{pc()};
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
    if(res.isMap() || res.isReference()) tmp = res;
    else tmp = res.value;
  }
  else if(mode == EvaluationMode::Assembly) {
    addMissing(name);
   // return {(uint64_t)pc()};
  }
  return tmp;
}

auto Plek::evalEvaluation(Statement stmt) -> Result {
  Result tmp{nothing};
  tmp = evaluateRHS(stmt->left());

  if(tmp.isString()) {
    auto [found, scope, name, res] = find(tmp.getString());
    if(found) tmp = res.value;
  }
  return tmp;
};

auto Plek::evalCall(Statement stmt) -> Result {
  if(mode == EvaluationMode::Assembly) {
    mode = EvaluationMode::Default;
    auto res = invoke(stmt->value, stmt->left());
    mode = EvaluationMode::Assembly;
    return res;
  }
  return invoke(stmt->value, stmt->left());
}

auto Plek::evalMapItem(Statement stmt) -> Result {
  if(stmt->content.size()<2) throw string{"Incomplete map statement."};

  auto ref = evaluateRHS(stmt->content[0]);
  if(!ref.isSymbol()) throw string{stmt->content[0]->value, " is not an map."};

  auto key = evaluateRHS(stmt->content[1]).getString();
  return { ref.getSymbol().get(key) };
}

auto Plek::handleDirective(Result value, uint dataLength) -> void {
  if(value.isInt()) {
    write(value.getInt(), dataLength);
  }

  else if(value.isString()) {
    for(auto c : value.getString()) {
      if(charactersUseMap) write(stringTable[c], 1);
      else write(c, dataLength);
    }
  }

  else if(value.isSymbol()) {
    auto symb = value.getSymbol();
    if(symb.isMap()) {
      for(auto item : symb.references) {
        handleDirective({item.value}, dataLength);
      }
    }
    else if(symb.isValue()) {
      handleDirective({symb.value}, dataLength);
    }
    else {
      error("Directive cannot handle this symbol");
    }
  }

  else if(value.isNothing()) {
    // fake write, hopefully the value is present later on
    write(0, dataLength);
  }

  else {
    error("Directive cannot handle ", value, " for ", dataLength, "bytes");
  }
}


auto Plek::calculate(Statement stmt) -> Result {
  Result result;
  for(auto item : stmt->content) {
    Result ir = evaluateRHS(item);
    if(!ir || ir.isNothing()) {
      if(mode == EvaluationMode::Assembly) ir = (int64_t)pc();
      else throw string{"Parameter had not been solved: ", item, " ", item->value, " -> ", ir};
    }
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
  else if (typeid(T) == typeid(int64_t)) result = calculateLogic(type, a, b);
  else error("unknown operation");

  return result;
}

template<typename T>
auto Plek::calculateLogic(StmtType type, const T& a, const T& b) -> Result {
  error("Operation not allowed");
  return {nothing};
}

template<>
auto Plek::calculateLogic<int64_t>(StmtType type, const int64_t& a, const int64_t& b) -> Result {
  Result result{nothing};

  if(type == st(LogicAnd))             result = Result{(int64_t) (a&b)};
  else if(type == st(LogicOr))         result = Result{(int64_t) (a|b)};
  else if(type == st(LogicShiftLeft))  result = Result{(int64_t) (a<<b)};
  else if(type == st(LogicShiftRight)) result = Result{(int64_t) (a>>b)};
  else if(type == st(LogicModulo))     result = Result{(int64_t) (a%b)};
  else error("unknown operation");

  return {result};
}