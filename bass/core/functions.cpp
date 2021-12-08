auto Plek::initFunctions() -> void {
  // returns bass version as string
  coreFunctions.insert("version#*", [&](Statement stmt) {
    return Value{appVersion};
  });
}