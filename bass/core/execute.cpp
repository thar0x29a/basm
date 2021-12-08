auto Plek::execute() -> bool {
  // Init
  frames.reset();
  frames.append(Frame::create(nullptr, "")); // root scope!
  architecture = new Architecture(*this);

  //try {
    //todo: better errorhandling. probl somewhere else. 

    for(auto& item : program) {
      excecuteBlock(item, frames.last());
    }
  //} catch(string e) {
  //  error(e);
  //}

  frames.removeRight();
  return true;
}

auto Plek::excecuteBlock(Statement stmt, Frame scope) -> bool {
  auto root = frames.first();
  auto path = scopePath();

  if(!stmt) return false;
  if(!scope) return false;
  
  if(!(stmt->is(st(Block)) || stmt->is(st(File)))) {
    error("AST: Block expected but got ", stmt);
  }

  print("Execute block -> ", scopePath(), "\n");
  Parser::debug(stmt->all());
  print("___\n");/**/

  bool doElse = false;  //todo: state machine ...

  for(auto& item : stmt->all()) {
    switch(item->type) {
      case st(Block): {
        excecuteBlock(item, scope); 
        break;
      }

      case st(CmdPrint): {
        for(auto cnt : item->all()) {
          evaluate(cnt);
          if(cnt->result) print(cnt->result);
        }
        break;
      }

      case st(CmdInclude): {
        auto file = item->leftValue().getString();
        if(!file) return true;
        if(load(file)) {
          stmt().append(program.takeRight());
          excecuteBlock(stmt().content.last(), scope);
        }
        break;
      }

      case st(Namespace): {
        if(!item->left() || !item->right()) throw "Broken AST #57";

        evaluate(item->left(), EvaluationMode::LeftSide);
        string name = item->leftResult().getString();

        auto subscope = Frame::create(scope, name);
          scope->addScope(subscope);
          frames.append(subscope);
        excecuteBlock(item->right(), subscope);
        frames.removeRight();        
        break;
      }

      case st(Label): {
        evaluate(item, EvaluationMode::LeftSide);
        scope->setConstant(item->result.getString(), {pc()});
        break;
      }

      case st(DeclConst): {
        if(!item->left() || !item->right()) throw "Broken AST #71";
        
        evaluate(item->right());
        evaluate(item->left(), EvaluationMode::LeftSide);

        string name = item->leftResult().getString();
        scope->setConstant(name, item->rightResult());

        break;
      }

      case st(DeclVar): {
        if(!item->left() || !item->right()) throw "Broken AST #81";
        
        evaluate(item->right());
        evaluate(item->left(), EvaluationMode::LeftSide);

        string name = item->leftResult().getString();
        scope->setVariable(name, item->rightResult());

        break;
      }

      case st(Assignment): {
        if(!item->left() || !item->right()) throw "Broken AST #91";
        evaluate(item->right());
        evaluate(item->left(), EvaluationMode::LeftSide);

        assign(
          item->leftResult().getString(),
          item->rightResult()
        );
        break;
      }

      case st(Macro): {
        if(!item->left()) throw "Broken AST #118";

        auto name = item->left()->value.getString();
        scope->setMacro(name, item);

        if(path.size()>0) name = {path, ".", name};
        break;
      }

      case st(Call): {
        if(!item->left()) throw "Broken AST #129";
        invoke(item->value, item->left());
        break;
      }

      case st(Return): {
        evaluate(item);
        scope->result = item->leftResult();
        break;
      }

      case st(Else): {
        if(doElse!=true) break;
        
        auto subscope = Frame::create(scope);
        frames.append(subscope);
        excecuteBlock(item->left(), subscope);
        frames.removeRight();
        break;
      }

      case st(ElseIf): {
        if(doElse!=true) break;
        // if not, fall thought
      }

      case st(If): {
        evaluate(item->left());
        auto res = item->leftResult();
        bool result = res.isTrue();
        
        if(result==true) {
          // invoke
          auto subscope = Frame::create(scope);
          frames.append(subscope);
          excecuteBlock(item->right(), subscope);
          frames.removeRight();
        }
        else {
          doElse = true;
          continue;
        }
        break;
      }

      case st(While): {
        bool result = false;

        do {
          evaluate(item->left());
          auto res = item->leftResult();
          result = res.isTrue();
        
          if(result==true) {
            auto subscope = Frame::create(scope);
            frames.append(subscope);
            excecuteBlock(item->right(), subscope);
            frames.removeRight();
          }
        } while(result==true);
        break;
      }

      case st(Raw): {
        assemble(item);
        break;
      }

      default: 
        notice("Unhandled element ", item);
    }

    doElse = false;
  }

  return true;
}