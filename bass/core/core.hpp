namespace Bass {

// General
struct Value : public any {
  auto isInt() -> bool { return type() == typeid(int64_t); };
  auto isFloat() -> bool { return type() == typeid(double); };
  auto isString() -> bool { return type() == typeid(string); };

  auto getInt() -> int64_t { return get<int64_t>(); };
  auto getFloat() -> double { return get<double>(); };
  auto getString() -> string { return get<string>(); };

  auto negate() -> Value {
    if(isInt()) return {getInt() * -1};
    else if(isFloat()) return {getFloat() * -1};
    else throw string{"cannot negate ", type().name()};
  };

  auto isTrue() -> bool {
    if(type() == typeid(nothing)) return false;
    else if(isInt() && getInt()==0) return false;
    else if(isFloat() && getFloat()==0) return false;
    else if(isString() && getString().length()==0) return false;
    return true;
  }
};

// Components
#include "scanner.hpp"
#include "parser.hpp"
#include "frame.hpp"

struct Plek {
  enum class Evaluation : uint { Default = 0, Strict = 1 };
  enum class Endian : uint { LSB, MSB };

  struct Tracker {
    bool enable = false;
    set<int64_t> addresses;
  };

  protected:
    file_buffer targetFile;
    vector<string> sourceFilenames;
    Program program;
    vector<Frame> frames;
    uint origin = 0;                //file offset
    int base = 0;                   //file offset to memory map displacement
    Endian endian = Endian::LSB;    //used for multi-byte writes (d[bwldq], etc)
    Tracker tracker;                //used to track writes to detect overwrites

  public:
    auto load(const string& filename) -> bool;
    auto target(const string& filename, bool create) -> bool;
    auto pc() const -> int64_t;
    auto seek(uint offset) -> void;
    auto track(uint length) -> void;
    auto write(uint64_t data, uint length) -> void;

    template<typename... P> auto notice(P&&... p) -> void;
    template<typename... P> auto warning(P&&... p) -> void;
    template<typename... P> auto error(P&&... p) -> void;

  // execute.cpp
    auto execute() -> bool;
    auto excecuteBlock(Statement, Frame scope) -> bool;
  
  // evaluate.cpp
    auto evaluate(Statement, Evaluation mode = Evaluation::Default) -> bool;
    auto calculate(Statement) -> Value;
    template <typename T>
    auto calculate(StmtType type, const T& a, const T& b) -> Value;

  // utility.cpp
    auto walkUp(const Program& what, std::function<bool (Statement, int)> with, int level = 0) -> void;
    auto walkDown(const Program& what, std::function<bool (Statement, int)> with, int level = 0) -> void;
    
    auto identifier(const string& name) -> Value;
    auto findSymbol(const string& name) -> SymbolRef;
    auto assign(const string& name, const Value& val) -> void;
    auto invoke(const string& name, Statement call) -> Value;
};

};