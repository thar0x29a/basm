#include "scanner.cpp"

namespace Bass {
  auto Plek::load(const string& filename) -> bool {
    if(!file::exists(filename)) {
      print(stderr, "warning: source file not found: ", filename, "\n");
      return false;
    }

    uint fileNumber = sourceFilenames.size();
    sourceFilenames.append(filename);

    string data = file::read(filename);

    Scanner scanner(fileNumber, data);
    auto tokens = scanner.scanTokens();

    Scanner::debug(tokens);

    //Analyzer analyzer(tokens);
    //analyzer.analyzeAll();
    //mainProgram.append(analyzer.getProgramm());

    return true;
  }
};