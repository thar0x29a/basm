auto Plek::initExecution() -> void {
  // init scopes and rootscope
  frames.reset();
  frames.append(Frame::create(nullptr, ""));

  // prepare architecture
  architecture = new Architecture(*this);
}

auto Plek::execute() -> bool {
  initExecution();

  try {
    //todo: better errorhandling. probl somewhere else. 

    for(auto& item : program) {
      excecuteBlock(item, frames.last());
    }
  } catch(string e) {
    error(e);
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
    bool callAtemt = false;
    string errstr{};

    switch(item->type) {
      case st(File):
      case st(Block): {
        excecuteBlock(item, scope); 
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

      case st(Call): {
        if(!item->left()) throw "Broken AST #129";
        try {
          invoke(item->value, item->left());
          break;
        }
        catch(string e) {
          callAtemt = true;
          errstr = e;
        }
      }

      case st(Raw): {
        string name = {item->result.getString(), " "};
        string text{};

        // are we an call-fallback?
        auto pool = (item->type == st(Call)) ? item->right() : item;

        // fancy debug stuff
        for(auto& el : pool->all()) {
          string dbug = el->result.getString();
          if(el->type == st(Raw)) dbug = terminal::color::magenta(dbug);
          else if(el->type == st(Identifier)) dbug = terminal::color::blue(dbug);
          else if(el->type == st(Call)) dbug = {terminal::color::cyan(dbug), "(*)"};
          //else if(el->type == st(Reference)) dbug = {"[",terminal::color::yellow(dbug),"]"};
          else if(el->type == st(Evaluation)) dbug = terminal::color::cyan("{...}");
          else if(el->type == st(Value)) dbug = terminal::color::green(dbug);
          else dbug = terminal::color::red(dbug);
          text.append(dbug);
        }
        print(terminal::color::yellow(name), text, "\n");
        


        // and now solved!
        string cmd = name;
        for(auto el : pool->all()) {
          string dbug = el->result.getString();
          
          if(el->type != st(Raw)) {
            evaluate(el);
            if(el->result.isNothing()) {
              dbug = el->value.getString();
            } else {
              dbug = el->result.getString();
            }
            cmd.append(dbug);
          } else {
            cmd.append(dbug);
          }
        }
        print(cmd, "\n");

        // run it!
        if(architecture->assemble(cmd)) break;
        else error("assembly failed for: ", cmd);
      }

      default: {
        if(callAtemt==true) error(errstr);
        notice("Unhandled element ", item);
      }
    }

    doElse = false;
  }

  return true;
}