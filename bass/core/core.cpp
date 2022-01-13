#include "stringifys.cpp"

namespace Bass {
  #include "../scanner/scanner.cpp"
  #include "../parser/parser.cpp"
  #include "execute.cpp"
  #include "functions.cpp"
  #include "evaluate.cpp"
  #include "utility.cpp"
  #include "../frame/frame.cpp"
  #include "../architecture/table/table.cpp"

  auto Plek::init() -> void {
    initFunctions();
    initExecution();
  }

  auto Plek::target(const string& filename, bool create) -> bool {
    if(targetFile) targetFile.close();
    if(!filename) return true;

    //cannot modify a file unless it exists
    if(!file::exists(filename)) create = true;

    if(!targetFile.open(filename, create ? file::mode::write : file::mode::modify)) {
      print(stderr, "warning: unable to open target file: ", filename, "\n");
      return false;
    }

    tracker.addresses.reset();
    return true;
  }

  /**
   * If filename is valid, and not in cache, it get loaded, scanned, parsed,
   * and will be appended to the end of our programm. 
  **/
  auto Plek::load(const string& filename) -> bool {
    if(!file::exists(filename)) {
      warning(stderr, "warning: source file not found: ", filename, "\n");
      return false;
    }

    for(auto item : sourceFiles) {
      if(item.filename != filename) continue;
      notice(filename, " was found in cache.");
      program.append(item.entryPoint);
      return true;
    }

    uint fileNumber = sourceFiles.size();

    string data = file::read(filename);
    Scanner scanner(fileNumber, data);
    auto tokens = scanner.scanTokens();
    //scanner.debug(tokens);

    Parser parser(scanner);
    parser.parseAll();
    auto entry = parser.first();
    
    program.append(entry);
    sourceFiles.append({filename, tokens, entry});

    notice("Done loading ", filename);
    return true;
  }

  auto Plek::pc() const -> int64_t {
    return origin + base;
  }

  auto Plek::seek(uint offset) -> void {
    if(!targetFile) return;
    //if(writePhase()) 
    targetFile.seek(offset);
  }

  auto Plek::write(uint64_t data, uint length) -> void {
    //if(writePhase()) {
      if(targetFile) {
        track(length);
        if(endian == Endian::LSB) targetFile.writel(data, length);
        if(endian == Endian::MSB) targetFile.writem(data, length);
      } else if(!isatty(fileno(stdout))) {
        if(endian == Endian::LSB) for(uint n : range(length)) fputc(data >> n * 8, stdout);
        if(endian == Endian::MSB) for(uint n : reverse(range(length))) fputc(data >> n * 8, stdout);
      }
    //}
    origin += length;
  }

  auto Plek::track(uint length) -> void {
    if(!tracker.enable) return;
    uint64_t address = targetFile.offset();
    for(auto n : range(length)) {
      if(tracker.addresses.find(address + n)) {
        error("overwrite detected at address 0x", hex(address + n), " [0x", hex(base + address + n), "]");
      }
      tracker.addresses.insert(address + n);
    }
  }

  template<typename... P> auto Plek::notice(P&&... p) -> void {
    string s{forward<P>(p)...};
    print(stderr, terminal::color::cyan("notice: "), s, "\n");
  }

  template<typename... P> auto Plek::warning(P&&... p) -> void {
    string s{forward<P>(p)...};
    print(stderr, terminal::color::yellow("warning: "), s, "\n");
    if(!strict) {
      //printInstruction();
      return;
    }

    //printInstructionStack();
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