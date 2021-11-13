auto Plek::execute() -> bool {
  // Init
  frames.reset();
  frames.append(Frame::create()); // root scope!

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
  if(!stmt) return false;
  if(!scope) return false;
  
  if(!(stmt->is(st(Block)) || stmt->is(st(File)))) {
    error("AST: Block expected but got ", stmt);
  }

  /*print("BLOCK\n");
  Parser::debug(stmt->all());
  print("___\n");/**/

  bool doElse = false;  //todo: state machine ...

  for(auto& item : stmt->all()) {
    switch(item->type) {
      case st(Block): excecuteBlock(item, scope); break;

      case st(CmdPrint): {
        evaluate(item);
        for(auto cnt : item->all()) {
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

      case st(Label): {
        scope->setConstant(item->value.getString(), {pc()});
        break;
      }

      case st(DeclConst): {
        if(!item->left() || !item->right()) throw "Broken AST #36";
        evaluate(item);
        scope->setConstant(
          item->leftValue().getString(),
          item->rightResult()
        );
        break;
      }

      case st(DeclVar): {
        if(!item->left() || !item->right()) throw "Broken AST #36";
        evaluate(item);
        scope->setVariable(
          item->leftValue().getString(),
          item->rightResult()
        );
        break;
      }

      case st(Assignment): {
        if(!item->left() || !item->right()) throw "Broken AST #36";
        evaluate(item);
        assign(
          item->leftValue().getString(),
          item->rightResult()
        );
        break;
      }

      case st(Macro): {
        if(!item->left()) throw "Broken AST #36";
        scope->setMacro(item->left()->value.getString(), item);
        break;
      }

      case st(Call): {
        if(!item->left()) throw "Broken AST #36";
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
        
        auto subscope = Frame::create();
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
          auto subscope = Frame::create();
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
            auto subscope = Frame::create();
            frames.append(subscope);
            excecuteBlock(item->right(), subscope);
            frames.removeRight();
          }
        } while(result==true);
        break;
      }

      default: 
        notice("Unhandled element ", item);
    }

    doElse = false;
  }

  return true;
}