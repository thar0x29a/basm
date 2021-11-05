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
};

// Components
#include "scanner.hpp"
#include "parser.hpp"
#include "frame.hpp"

struct Plek {
  enum class Evaluation : uint { Default = 0, Strict = 1 };

  protected:
    vector<string> sourceFilenames;
    Program program;
    vector<Frame> frames;
    int64_t pc = 0;   // current offset in output file. wip
  

  public:
    auto load(const string& filename) -> bool;
    template<typename... P> auto notice(P&&... p) -> void;
    template<typename... P> auto warning(P&&... p) -> void;
    template<typename... P> auto error(P&&... p) -> void;

  // execute.cpp
    auto execute() -> bool;
    auto excecuteBlock(Statement, Frame scope) -> bool;
  
  // evaluate.cpp
    auto evaluate(Statement, Evaluation mode = Evaluation::Default) -> bool;
    auto calculate(Statement) -> Value;

  // utility.cpp
    auto walkUp(const Program& what, std::function<bool (Statement, int)> with, int level = 0) -> void;
    auto walkDown(const Program& what, std::function<bool (Statement, int)> with, int level = 0) -> void;
    
    auto identifier(const string& name) -> Value;
    auto findSymbol(const string& name) -> SymbolRef;
    auto invoke(const string& name, Statement call) -> Value;
};

};