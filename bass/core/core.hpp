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
  protected:
    vector<string> sourceFilenames;
    Program program;
    
  public:
    auto load(const string& filename) -> bool;
};

};