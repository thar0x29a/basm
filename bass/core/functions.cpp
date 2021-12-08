auto Plek::initFunctions() -> void {
  // returns bass version as string
  coreFunctions.insert("version#0", [&](Statement stmt) {
    return Value{appVersion};
  });

  auto arch = [&](Statement args) {  
    string name{};
    evaluate(args);

    if(args->left()) {
      name = args->leftResult().getString();
    }

    if(name=="" || name=="none") {
      architecture = new Architecture{*this};
      notice("architecture reseted");
    } else {
      notice("switch architecture to ", name);
    }

    return Value{nothing};
  };
  coreFunctions.insert("arch#1", arch);
  coreFunctions.insert("architecture#1", arch);

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