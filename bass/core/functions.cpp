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
      notice("architecture reseted");
    } else {
      architecture = new Table{*this, readArchitecture(name)};
      notice("switch architecture to ", terminal::color::green(name), " using the table engine");
    }

    return Result{nothing};
  };

  coreFunctions.insert("arch#1", arch);
  coreFunctions.insert("architecture#1", arch);

  coreFunctions.insert("include#1", [&](Statement args) {
    auto scope = frames.right();
    auto res = evaluateRHS(args->left());

    if(res) {
      auto file = res.getString();
      if(load(file)) {
        // loaded file got added to the main program.
        // we will remove it from there, since we just execute it once
        exBlock(program.takeRight());
      }
    }
    return Result{nothing};
  });

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
    }

    return Result{(int64_t)origin};
  };
  coreFunctions.insert("origin#0", originFun);
  coreFunctions.insert("origin#1", originFun);

  auto baseFun = [&](Statement args) {  
    if(args->left()) {
      auto res = evaluateRHS(args->left());
      if(!res.isInt()) error("wrong parameter type3");
      
      base = res.getInt();
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
}