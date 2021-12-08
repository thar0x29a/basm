auto Plek::initFunctions() -> void {
  // returns bass version as string
  coreFunctions.insert("version#0", [&](Statement stmt) {
    return Value{appVersion};
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