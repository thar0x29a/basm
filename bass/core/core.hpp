namespace Bass {

// General
struct Value : public any {
  auto isInt() -> bool { return type() == typeid(int64_t); };
  auto isFloat() -> bool { return type() == typeid(double); };
  auto isString() -> bool { return type() == typeid(string); };

  auto getInt() -> int64_t { return get<int64_t>(); };
  auto getFloat() -> double { return get<double>(); };
  auto getString() -> string { return get<string>(); };
};

// Components
#include "scanner.hpp"
#include "parser.hpp"

struct Plek {
  enum class Evaluation : uint { Default = 0, Strict = 1 };

  struct SymbolRef {
    enum class SymbolType : uint { Const, Var, Callable }; // and more ...
    
    SymbolType type;
    Value value;
    Statement ref;
  };

  struct FrameElement {
    enum class Level : uint {
      Inline,  //use deepest frame (eg for parameters)
      Active,  //use deepest non-inline frame
      Parent,  //use second-deepest non-inline frame
      Global,  //use root frame
    };

    // content
    map<string, SymbolRef> symbolTable;
  };
  using Frame = shared_pointer<FrameElement>;


  protected:
    vector<string> sourceFilenames;
    Program program;

  public:
    auto load(const string& filename) -> bool;
    template<typename... P> auto notice(P&&... p) -> void;
    template<typename... P> auto warning(P&&... p) -> void;
    template<typename... P> auto error(P&&... p) -> void;

    map<string, SymbolRef> symbolTable; // to be removed
    vector<Frame> frames;

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
    auto invoke(const string& name, Statement call) -> Value;

    auto setConstant(const string& name, const Value& val) -> void;
    auto setVariable(const string& name, const Value& val) -> void;
    auto setMacro(const string& name, Statement def) -> void;
};

};