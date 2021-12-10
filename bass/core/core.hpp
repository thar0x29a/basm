#include <tuple>

namespace Bass {

// Components
#include "value.hpp"
#include "../scanner/scanner.hpp"
#include "../parser/parser.hpp"
#include "../frame/frame.hpp"

// General
enum class EvaluationMode : uint { Default = 0, Strict, LeftSide };
enum class Endian : uint { LSB, MSB };

struct Tracker {
  bool enable = false;
  set<int64_t> addresses;
};

struct Directives {
private:
  struct _EmitBytesOp {
    string token;
    uint dataLength;
  };

public:
  vector<_EmitBytesOp> EmitBytes;

  Directives()
  : EmitBytes ({ {"db ", 1}, {"dw ", 2}, {"dl ", 3}, {"dd ", 4}, {"dq ", 8}})
  {}
    
  void add(string token, uint dataLength) {
    EmitBytes.append( {token, dataLength} );
  }
};

struct SourceCode {
  string filename;
  vector<Token> tokens;
  Statement entryPoint;
};

struct Architecture;

using CoreFunction = std::function<Value (Statement)>;
using string_vector = vector<string>;

struct Plek {
  protected:
    file_buffer targetFile;
    //vector<string> sourceFilenames;
    vector<SourceCode> sourceFiles;
    Program program;
    vector<Frame> frames;
    map<string, CoreFunction> coreFunctions;
    bool strict = false;            // strict mode
    int64_t origin = 0;                //file offset
    int64_t base = 0;                   //file offset to memory map displacement
    Endian endian = Endian::LSB;    //used for multi-byte writes (d[bwldq], etc)
    Directives directives;          //active directives
    Tracker tracker;                //used to track writes to detect overwrites
    shared_pointer<Architecture> architecture;
    friend class Architecture;

  public:
    const int64_t appVersion = 20;
    const string appLabel = {"v", appVersion, " plek 0"};

    auto load(const string& filename) -> bool;
    auto target(const string& filename, bool create) -> bool;
    auto pc() const -> int64_t;
    auto seek(uint offset) -> void;
    auto track(uint length) -> void;
    auto write(uint64_t data, uint bytelength = 1) -> void;

    template<typename... P> auto notice(P&&... p) -> void;
    template<typename... P> auto warning(P&&... p) -> void;
    template<typename... P> auto error(P&&... p) -> void;

  // execute.cpp
    auto initExecution() -> void;
    auto execute() -> bool;
    auto excecuteBlock(Statement, Frame scope) -> bool;
  
  // functions.cpp
    auto initFunctions() -> void;

  // evaluate.cpp
    auto evaluate(Statement, EvaluationMode mode = EvaluationMode::Default) -> bool;
    auto calculate(Statement) -> Value;
    template <typename T>
    auto calculate(StmtType type, const T& a, const T& b) -> Value;
    auto handleDirective(string, Statement) -> bool;

  // utility.cpp
    auto walkUp(const Program& what, std::function<bool (Statement, int)> with, int level = 0) -> void;
    auto walkDown(const Program& what, std::function<bool (Statement, int)> with, int level = 0) -> void;

    auto identifier(const string& name) -> Value;
    auto find(const string& symbolName) -> std::tuple<bool, Frame, string>;

    auto assign(const string& name, const Value& val) -> void;
    auto invoke(const string& name, Statement call) -> Value;

    auto scopePath() -> string;
    auto readArchitecture(const string& name) -> string;
};

// loaded here because of high dependencys
#include "../architecture/architecture.hpp"
#include "../architecture/table/table.hpp"
};