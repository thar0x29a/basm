#include "stringifys.cpp"

namespace Bass {
  #include "scanner.cpp"
  #include "parser.cpp"

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
    //Scanner::debug(tokens);

    Parser parser(scanner);
    parser.parseAll();
    program.append(parser.first());

    Parser::debug(program);

    print("Done loading ", filename, "\n");
    return true;
  }
};