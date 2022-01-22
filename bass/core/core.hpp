#include <tuple>

namespace Bass {

// General
struct StmtNode;
using Statement = shared_pointer<StmtNode>;
using Program = vector<Statement>;
#define st(t) (Bass::StmtType::t)

#include "value.hpp"
#include "symbol.hpp"
#include "result.hpp"
#include "../scanner/scanner.hpp"
#include "../parser/parser.hpp"
#include "../frame/frame.hpp"

enum class EvaluationMode : uint { Default = 0, Strict, LeftSide };
enum class Endian : uint { LSB, MSB };
enum class ReturnState : uint { Running = 0, Default, Return, Break, Continue, Lookahead, Error };

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

using CoreFunction = std::function<Result (Statement)>;
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
    bool simulate = false;          // if set no output will be done.
    int64_t origin = 0;             // file offset
    int64_t base = 0;               // file offset to memory map displacement
    Endian endian = Endian::LSB;    // used for multi-byte writes (d[bwldq], etc)
    Directives directives;          // active directives
    Tracker tracker;                // used to track writes to detect overwrites
    int64_t stringTable[256];       // overrides for d[bwldq] text strings
    bool charactersUseMap = true;   // 0 = '*' parses as ASCII; 1 = '*' uses stringTable[]
    shared_pointer<Architecture> architecture;
    friend class Architecture;

  public:
    const int64_t appVersion = 20;
    const string appLabel = {"v", appVersion, " plek 220115"};

    auto init() -> void;
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
    auto exBlock(Statement) -> ReturnState;
    auto exLookahead(Statement, uint, int64_t) -> ReturnState;
    auto exStatement(Statement) -> ReturnState;
    auto exConstDeclaration(Statement) -> bool;
    auto exVarDeclaration(Statement) -> bool;
    auto exFunDeclaration(Statement) -> bool;
    auto exCall(Statement) -> bool;
    auto exReturn(Statement) -> ReturnState;
    auto exNamespace(Statement) -> bool;
    auto exLabel(Statement) -> bool;
    auto exAssign(Statement) -> bool;
    auto exMapAssign(Statement)-> bool;
    auto exIfState(Statement) -> ReturnState;
    auto exIf(Statement) -> ReturnState;
    auto exElse(Statement) -> ReturnState;
    auto exWhile(Statement) -> bool;
    auto exAssembly(Statement) -> ReturnState;

  // functions.cpp
    auto initFunctions() -> void;

  // evaluate.cpp
    auto evaluateLHS(Statement) -> Result;
    auto evaluateRHS(Statement) -> Result;
    auto calculate(Statement) -> Result;
    auto evalAssign(Statement) -> Result;
    auto evalIdentifier(Statement) -> Result;
    auto evalCall(Statement) -> Result;
    auto evalMapItem(Statement) -> Result;
    auto evalReference(Statement stmt) -> Result;
    template <typename T>
    auto calculate(StmtType type, const T& a, const T& b) -> Result;
    auto handleDirective(string, Statement) -> bool;
    auto handleDirectiveValue(Result value, uint dataLength) -> void;
  
  // utility.cpp
    auto find(const string& symbolName) -> std::tuple<bool, Frame, string, Symbol>;

    auto assign(const string& dest, Result src) -> void;
    auto constant(const string& name, const string& value) -> void;
    auto invoke(const string& name, Statement call) -> Result;

    auto readArchitecture(const string& name) -> string;
};

// loaded here because of high dependencys
#include "../architecture/architecture.hpp"
#include "../architecture/table/table.hpp"
};