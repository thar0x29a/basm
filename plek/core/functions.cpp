auto Plek::initFunctions() -> void {
  // returns bass version as string
  coreFunctions.insert("version#0", [&](Statement stmt) {
    return Result{appVersion};
  });

  auto arch = [&](Statement args) {  
    string name{};
    if(args->left()) {
      auto res = evaluateRHS(args->left());
      name = res.getString();
    }

    if(name=="" || name=="none") {
      architecture = new Architecture{*this};
      debug("architecture reseted");
    } else {
      architecture = new Table{*this, readArchitecture(name)};
      debug("switch architecture to ", terminal::color::green(name), " using the table engine");
    }

    return Result{nothing};
  };

  coreFunctions.insert("arch#1", arch);
  coreFunctions.insert("architecture#1", arch);

  auto includeFun = [&](Statement args) {
    auto res = evaluateRHS(args->left());
    vector<Frame> buffer;

    if(!res) error("Invalid parameter");

    // custom scope parameter
    if(args->right()) {
      auto place = args->rightValue();
      if(!place || !place.isString()) error("Invalid second parameter.");
      
      string p = place.getString();
      
      if(p.equals("GLOBAL")) {
        while(frames.size()>1) buffer.append( frames.takeLast() );
      }
      else if(p.equals("PARENT")) {
        buffer.append( frames.takeLast() );
      }
      else {
        error("Invalid second parameter: ", p);
      }
    }

    // include    
    auto file = res.getString();
    if(load(file)) {
      // loaded file got added to the main program.
      // we will remove it from there, since we just execute it once
      exBlock(program.takeRight());
    }

    // restore scopes, if any
    while(buffer.size()>0) {
      frames.append( buffer.takeLast() );
    }

    return Result{nothing};
  };

  coreFunctions.insert("include#1", includeFun);
  coreFunctions.insert("include#2", includeFun);

  coreFunctions.insert("ds#1", [&](Statement args) {
    auto res = evaluateRHS(args->left());
    if(!res.isInt()) error("wrong parameter type1");
    
    origin += res.getInt();
    seek(origin);

    return Result{nothing};
  });

  auto originFun = [&](Statement args) {  
    if(args->left()) {
      auto res = evaluateRHS(args->left());
      if(!res.isInt()) error("wrong parameter type2");
      
      origin = res.getInt();
      seek(origin);
    }

    return Result{(int64_t)origin};
  };
  coreFunctions.insert("origin#0", originFun);
  coreFunctions.insert("origin#1", originFun);

  auto baseFun = [&](Statement args) {  
    if(args->left()) {
      auto res = evaluateRHS(args->left());
      if(!res.isInt()) error("wrong parameter type3");
      
      base = res.getInt() - origin;
    }

    return Result{(int64_t)base};
  };
  coreFunctions.insert("base#0", baseFun);
  coreFunctions.insert("base#1", baseFun);

  coreFunctions.insert("assert#1", [&](Statement args) {
    auto res = evaluateRHS(args->left());
    if(!res.isTrue()) error("assertion failed");

    return Result{nothing};
  });

  coreFunctions.insert("endian#1", [&](Statement args) {
    auto res = evaluateRHS(args->left());
    string arg = res.getString();

    if(arg == "lsb") endian = Endian::LSB;
    else if(arg == "msb") endian = Endian::MSB;
    else error("invalid endian. allowed: lsb|msb");

    return Result{nothing};
  });

  auto fill = [&](Statement args) {  
    uint length{0};
    uint with{0};

    auto lr = evaluateRHS(args->left());
    if(!lr.isInt()) error("wrong parameter type4");
    length = lr.getInt();

    if(args->right()) {
      auto rr = evaluateRHS(args->right());
      if(!rr.isInt()) error("wrong parameter type5");
      with = rr.getInt();
    }

    while(length--) write(with);
    return Result{nothing};
  };

  coreFunctions.insert("fill#1", fill);
  coreFunctions.insert("fill#2", fill);

  coreFunctions.insert("pc#0", [&](Statement args) {
    return Result{pc()};
  });

  // main print command
  coreFunctions.insert("print#*", [&](Statement args) {
    string text{};
    for(auto cnt : args->all()) {
      Result res = evaluateRHS(cnt);
      if(res) text.append(res);
    }
    print(text);
    return Result{nothing};
  });

  coreFunctions.insert("error#*", [&](Statement args) {
    string text{};
    for(auto cnt : args->all()) {
      Result res = evaluateRHS(cnt);
      if(res) text.append(res);
    }
    error(text);
    return Result{nothing};
  });

  coreFunctions.insert("warning#*", [&](Statement args) {
    string text{};
    for(auto cnt : args->all()) {
      Result res = evaluateRHS(cnt);
      if(res) text.append(res);
    }
    warning(text);
    return Result{nothing};
  });

  coreFunctions.insert("notice#*", [&](Statement args) {
    string text{};
    for(auto cnt : args->all()) {
      Result res = evaluateRHS(cnt);
      if(res) text.append(res);
    }
    notice(text);
    return Result{nothing};
  });


  coreFunctions.insert("Type.new#1", [&](Statement args) {
    if(args->size()!=1) error("wrong parameter count");
    
    auto arg = evaluateRHS(args->left());
    string name;

    if(arg.isString()) name = arg.getString();
    else if(arg.isSymbol()) {
      auto s = arg.getSymbol();
      if(!s.isMap()) error("Invalid parameter.");

      auto field = s.get("name");
      if(!field.isString()) error("Invalid parameter!");
      name = field.getString();
    }
    else error("Invalid parameter..");

    return Result{Value::Custom{name}};
  });

  coreFunctions.insert("Type.define#1", [&](Statement args) {
    if(args->size()!=1) error("wrong parameter count");
    if(!args->leftValue().isString()) error("Typename expected");

    auto arg = evaluateRHS(args->left());
    auto name = arg.getString();

    // check / create entry in global lookup
    if(auto res = customTypes.find(name)) {
      return Result{res};
    }
    else {
      debug("Define type ", name);
      Symbol m = Symbol::newMap();
      m.references.insert("name", Symbol::newConst( Result{name} ));
      
      customTypes.insert(name, m);
      return Result{m};
    }
  });

  coreFunctions.insert("Type.of#1", [&](Statement args) {
    if(args->size()!=1) error("wrong parameter count");

    auto source = evaluateRHS(args->left());
    string res;

    if(!source) error("Invalid parameter");
    else if(source.isInt()) res = "integer";
    else if(source.isFloat()) res = "float";
    else if(source.isString()) res = "string";
    else if(source.isNothing()) res = "null";
    else if(source.isCustom()) {
      res = source.getCustom().name;
    }
    else if(source.isSymbol()) {
      auto s = source.getSymbol();
      if(s.isReference()) res = "function";
      else if(s.isMap()) res = "array";
    }
    else res = "unknown";

    return Result{res};
  });

  coreFunctions.insert("Array.new#*", [&](Statement args) {
    Symbol m = Symbol::newMap();
    int i = 0;
    
    for(auto cnt : args->all()) {
      Result res = evaluateRHS(cnt);
      m.references.insert(
        {i++},
        Symbol::newVar(res)
      );
    }

    return Result{m};
  });

  coreFunctions.insert("Array.size#1", [&](Statement args) {
    if(args->size()!=1) error("wrong parameter count");
    
    auto p = evaluateRHS(args->left());
    if(!p.isSymbol()) error("wrong parameter type");
    
    auto s = p.getSymbol();
    if(!s.isMap()) error("wrong parameter type");

    return Result{  (int64_t)s.references.size() };
  });

  /* "filename", (offset), (length) -> specs */
  auto include = [&](Statement args) {
    if(!args->leftValue().isString()) error("Filename expected");
    auto filename = args->leftValue().getString();

    if(!file::exists(filename)) error("File not found '", filename, "'");  
    auto fp = file::open(filename, file::mode::read);

    uint offset = 0;
    uint length = fp.size();

    // offset
    if(args->size()>=2) {
      auto ofs = args->all()[1]->value;
      if(!ofs.isInt()) error("Second parameter needs to be Int");
      
      auto tmp = ofs.getInt();
      offset = tmp;

      if(length-offset <=0 ) error("Offset is too big: ", tmp, ">=", fp.size());
      length -= offset;
    }

    // length
    if(args->size()==3) {
      auto len = args->all()[2]->value;
      if(!len.isInt()) error("Second parameter needs to be Int");
      
      auto tmp = len.getInt();
      if(tmp > length) error("Length is too big: ", offset, "+", tmp, ">=", fp.size());
      length = tmp;
    }

    // copy
    int64_t oldpc = pc();
    fp.seek(offset);
    for(uint todo=length; !fp.end() && todo; todo--) write(fp.read());

    // return some specs
    auto result = Symbol::newMap();
    result.references.insert({"offset"}, Symbol::newVar({(int64_t)oldpc}));
    result.references.insert({"size"}, Symbol::newVar({(int64_t)length}));
  
    return Result{result};
  };

  coreFunctions.insert("File.include#1", include);
  coreFunctions.insert("File.include#2", include);
  coreFunctions.insert("File.include#3", include);

  auto output = [&](Statement args) {
    if(!args->leftValue().isString()) error("Filename expected");
    auto filename = args->leftValue().getString();
    bool create = false;

    if(args->size()==2) {
      auto b = evaluateRHS(args->right());
      create = b.isTrue();
    }

    if(!create && !file::exists(filename)) error("File not found '", filename, "'"); 

    target(filename, create);
    return Result{nothing};
  };

  coreFunctions.insert("output#1", output);
  coreFunctions.insert("output#2", output);

  coreFunctions.insert("File.size#1", [&](Statement args) {
    if(!args->leftValue().isString()) error("Filename expected");
    auto filename = args->leftValue().getString();
    if(!file::exists(filename)) error("File not found '", filename, "'");

    auto fp = file::open(filename, file::mode::read);
    return Result{(int64_t)fp.size()};
  });

  coreFunctions.insert("File.exists#1", [&](Statement args) {
    if(!args->leftValue().isString()) error("Filename expected");
    auto filename = args->leftValue().getString();

    if(file::exists(filename)) return Result{(int64_t)1};
    return Result{(int64_t)0};
  });

  // read? 

  // copy?

  // delete?

  // enqueue
  coreFunctions.insert("enqueue#0", [&](Statement args) {
    auto result = Symbol::newMap();
    result.references.insert({"pc"}, Symbol::newConst({(int64_t)pc()}));
    result.references.insert({"base"}, Symbol::newConst({(int64_t)base}));
    result.references.insert({"origin"}, Symbol::newConst({(int64_t)origin}));

    return Result{result};
  });

  // dequeue
  coreFunctions.insert("dequeue#1", [&](Statement args) {
    auto res = evaluateRHS(args->left());
    if(!res.isSymbol()) error("Invalid state");

    auto map = res.getSymbol();
    if(auto tmp = map.get("base")) base = tmp.getInt();
    if(auto tmp = map.get("origin")) origin = tmp.getInt();

    return Result{nothing};
  });

  // String size
  coreFunctions.insert("String.size#1", [&](Statement args) {
    auto res = evaluateRHS(args->left());
    if(!res.isString()) error("No string found");

    int64_t len = res.getString().size();
    return Result{len};
  });

  // is 'null'?
  coreFunctions.insert("isNull#1", [&](Statement args) {
    auto res = evaluateRHS(args->left());

    if(res.isNothing()) return Result{(int64_t)1};
    else return Result{(int64_t)0};
  });

  coreFunctions.insert("logLevel#1", [&](Statement args) {
    auto res = evaluateRHS(args->left());
    int64_t old = (uint)log_level;

    if(!res.isInt()) error("Invalid parameter type.");

    switch(res.getInt()) {
      case 0: log_level = LogLevel::LEVEL_ALL; break;
      case 1: log_level = LogLevel::LEVEL_DEBUG; break;
      case 2: log_level = LogLevel::LEVEL_NOTICE; break;
      case 3: log_level = LogLevel::LEVEL_WARN; break;
      case 4: log_level = LogLevel::LEVEL_ERROR; break;
      case 5: log_level = LogLevel::LEVEL_FATAL; break;
      case 6: log_level = LogLevel::LEVEL_OFF; break;
      default:
        error("Invalid parameter.");
    }

    return Result{old};
  });
}