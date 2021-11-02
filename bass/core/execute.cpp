auto Plek::execute() -> bool {
  //Todo: init

  for(auto& item : program) {
    excecuteBlock(item);
  }
    
  return true;
}

auto Plek::excecuteBlock(Statement stmt) -> bool {
  if(!stmt) return false;
  
  if(!(stmt->is(st(Block)) || stmt->is(st(File)))) {
    error("AST: Block expected but got ", stmt);
  }

  print("BLOCK\n");
  Parser::debug(stmt->all());
  print("___\n");

  for(auto& item : stmt->all()) {
    switch(item->type) {
      case st(Block): excecuteBlock(item); break;
      
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
          excecuteBlock(stmt().content.last());
        }
        break;
      }

      case st(DeclConst): {
        if(!item->left() || !item->right()) throw "Broken AST #36";
        auto r = item->right();
        evaluate(r);
        setConstant(item->left()->value.getString(), r->result);
        break;
      }

      case st(DeclVar): {
        if(!item->left() || !item->right()) throw "Broken AST #36";
        auto r = item->right();
        evaluate(r);
        setVariable(item->left()->value.getString(), r->result);
        break;
      }

      default: 
        notice("Unhandled element ", item);
    }
  }
  return true;
}