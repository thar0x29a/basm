auto Plek::execute() -> bool {
  // Init
  frames.reset();
  frames.append(Frame::create("")); // root scope!

  //try {
    //todo: better errorhandling. probl somewhere else. 

    for(auto& item : program) {
      excecuteBlock(item, frames.last());
    }
  //} catch(string e) {
  //  error(e);
  //}

  for(auto frm : frames) {
    print(frm);
  }

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

  /*print("Execute block -> ", scopePath(), "\n");
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

      case st(Namespace): {
        if(!item->left() || !item->right()) throw "Broken AST #57";
        string name = item->leftValue().getString();
        auto subscope = Frame::create(name);
        frames.append(subscope);
        excecuteBlock(item->right(), subscope);
        frames.removeRight();        
        break;
      }

      case st(Label): {
        scope->setConstant(item->value.getString(), {pc()});
        break;
      }

      case st(DeclConst): {
        if(!item->left() || !item->right()) throw "Broken AST #71";
        evaluate(item);
        
        string name = item->leftValue().getString();
        if(scope->temporary==true) {
          scope->setConstant(name, item->rightResult());
        }
        else {
          if(path.size()>0) name = {path, ".", name};
          root->setConstant(name, item->rightResult());
        }

        break;
      }

      case st(DeclVar): {
        if(!item->left() || !item->right()) throw "Broken AST #81";
        evaluate(item);

        string name = item->leftValue().getString();
        if(scope->temporary==true) {
          scope->setVariable(name, item->rightResult());
        }
        else {
          if(path.size()>0) name = {path, ".", name};
          root->setVariable(name, item->rightResult());
        }

        break;
      }

      case st(Assignment): {
        if(!item->left() || !item->right()) throw "Broken AST #91";
        evaluate(item);
        //todo: find the right one !

        assign(
          item->leftValue().getString(),
          item->rightResult()
        );
        break;
      }

      case st(Macro): {
        if(!item->left()) throw "Broken AST #101";

        auto name = item->left()->value.getString();
        if(path.size()>0) name = {path, ".", name};

        root->setMacro(name, item);
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