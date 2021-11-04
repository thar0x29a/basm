#include "stringifys.cpp"

namespace Bass {
  #include "scanner.cpp"
  #include "parser.cpp"
  #include "execute.cpp"
  #include "evaluate.cpp"
  #include "utility.cpp"
  #include "frame.cpp"

  auto Plek::load(const string& filename) -> bool {
    if(!file::exists(filename)) {
      warning(stderr, "warning: source file not found: ", filename, "\n");
      return false;
    }

    //TODO: only load stuff once!

    uint fileNumber = sourceFilenames.size();
    sourceFilenames.append(filename);

    string data = file::read(filename);

    Scanner scanner(fileNumber, data);
    auto tokens = scanner.scanTokens();

    Parser parser(scanner);
    parser.parseAll();
    program.append(parser.first());

    notice("Done loading ", filename);
    return true;
  }


  template<typename... P> auto Plek::notice(P&&... p) -> void {
    string s{forward<P>(p)...};
    print(stderr, terminal::color::cyan("notice: "), s, "\n");
  }

  template<typename... P> auto Plek::warning(P&&... p) -> void {
    string s{forward<P>(p)...};
    print(stderr, terminal::color::yellow("warning: "), s, "\n");
    /*if(!strict) {
      printInstruction();
      return;
    }

    printInstructionStack();/**/
    struct BassWarning {};
    throw BassWarning();
  }

  template<typename... P> auto Plek::error(P&&... p) -> void {
    string s{forward<P>(p)...};
    print(stderr, terminal::color::red("error: "), s, "\n");
    //todo: printInstructionStack();

    struct BassError {};
    throw BassError();
  }
};

