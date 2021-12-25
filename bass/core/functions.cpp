auto Plek::initFunctions() -> void {
  // returns bass version as string
  coreFunctions.insert("version#0", [&](Statement stmt) {
    return Value{appVersion};
  });

  auto arch = [&](Statement args) {  
    string name{};
    evaluate(args);

    if(args->leftResult()) {
      name = args->leftResult().getString();
    }

    if(name=="" || name=="none") {
      architecture = new Architecture{*this};
      notice("architecture reseted");
    } else {
      architecture = new Table{*this, readArchitecture(name)};
      notice("switch architecture to ", terminal::color::green(name), " using the table engine");
    }

    return Value{nothing};
  };

  coreFunctions.insert("arch#1", arch);
  coreFunctions.insert("architecture#1", arch);

  coreFunctions.insert("include#1", [&](Statement args) {
    auto scope = frames.right();
    evaluate(args);
    auto file = args->leftResult().getString();
    if(file) {
      if(load(file)) {
        // loaded file got added to the main program.
        // we will remove it from there, since we just execute it once
        excecuteBlock(program.takeRight(), scope);
      }
    }
    return Value{nothing};
  });

  coreFunctions.insert("ds#1", [&](Statement args) {
    evaluate(args->left());
    if(!args->leftResult().isInt()) error("wrong parameter type1");
    auto val = args->leftResult().getInt();
    origin += val;
    seek(origin);

    return Value{nothing};
  });

  auto originFun = [&](Statement args) {  
    if(args->left()) {
      evaluate(args->left());
      if(!args->leftResult().isInt()) error("wrong parameter type2");
      auto val = args->leftResult().getInt();
      origin = val;
    }

    return Value{(int64_t)origin};
  };
  coreFunctions.insert("origin#0", originFun);
  coreFunctions.insert("origin#1", originFun);

  auto baseFun = [&](Statement args) {  
    if(args->left()) {
      evaluate(args->left());
      if(!args->leftResult().isInt()) error("wrong parameter type3");
      auto val = args->leftResult().getInt();
      base = val;
    }

    return Value{(int64_t)base};
  };
  coreFunctions.insert("base#0", baseFun);
  coreFunctions.insert("base#1", baseFun);

  coreFunctions.insert("assert#1", [&](Statement args) {
    evaluate(args->left());
    auto res = args->leftResult();
    if(!res.isTrue()) error("assertion failed");

    return Value{nothing};
  });

  coreFunctions.insert("endian#1", [&](Statement args) {
    string arg = args->leftValue().getString();
    if(arg == "lsb") endian = Endian::LSB;
    if(arg == "msb") endian = Endian::MSB;

    return Value{nothing};
  });

  auto fill = [&](Statement args) {  
    uint length{0};
    uint with{0};

    evaluate(args);
    if(!args->leftResult() || !args->leftResult().isInt()) error("wrong parameter type4");
    length = args->leftResult().getInt();

    if(args->right()) {
      if(!args->rightResult().isInt()) error("wrong parameter type5");
      with = args->rightResult().getInt();
    }

    while(length--) write(with);
    return Value{nothing};
  };

  coreFunctions.insert("fill#1", fill);
  coreFunctions.insert("fill#2", fill);

  coreFunctions.insert("pc#0", [&](Statement args) {
    return Value{pc()};
  });

  // main print command
  coreFunctions.insert("print#*", [&](Statement args) {
    string text{};
    for(auto cnt : args->all()) {
      evaluate(cnt);
      if(cnt->result) text.append(cnt->result);
    }
    print(text);
    return Value{nothing};
  });

  coreFunctions.insert("error#*", [&](Statement args) {
    string text{};
    for(auto cnt : args->all()) {
      evaluate(cnt);
      if(cnt->result) text.append(cnt->result);
    }
    error(text);
    return Value{nothing};
  });

  coreFunctions.insert("warning#*", [&](Statement args) {
    string text{};
    for(auto cnt : args->all()) {
      evaluate(cnt);
      if(cnt->result) text.append(cnt->result);
    }
    warning(text);
    return Value{nothing};
  });

  coreFunctions.insert("notice#*", [&](Statement args) {
    string text{};
    for(auto cnt : args->all()) {
      evaluate(cnt);
      if(cnt->result) text.append(cnt->result);
    }
    notice(text);
    return Value{nothing};
  });
}