auto Plek::execute() -> bool {
  // Init
  frames.reset();
  frames.append(Frame{});

  for(auto& item : program) {
    excecuteBlock(item, frames.right());
  }

  frames.removeRight();
  return true;
}

auto Plek::excecuteBlock(Statement stmt, Frame scope) -> bool {
  if(!stmt) return false;
  
  if(!(stmt->is(st(Block)) || stmt->is(st(File)))) {
    error("AST: Block expected but got ", stmt);
  }

  print("BLOCK\n");
  Parser::debug(stmt->all());
  print("___\n");

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

      case st(DeclConst): {
        if(!item->left() || !item->right()) throw "Broken AST #36";
        evaluate(item);
        setConstant(
          item->left()->value.getString(),
          item->right()->result
        );
        break;
      }

      case st(DeclVar): {
        if(!item->left() || !item->right()) throw "Broken AST #36";
        evaluate(item);
        setVariable(
          item->left()->value.getString(),
          item->right()->result
        );
        break;
      }

      case st(Macro): {
        if(!item->left()) throw "Broken AST #36";
        setMacro(item->left()->value.getString(), item);
        break;
      }

      case st(Call): {
        if(!item->left()) throw "Broken AST #36";
        //TODO: also insert on evaluate!
        invoke(item->value, item->left());
        break;
      }

      default: 
        notice("Unhandled element ", item);
    }
  }
  return true;
}