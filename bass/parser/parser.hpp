#include "Statement.hpp"
#include "MacroStatement.hpp"
#include "MapAssignStatement.hpp"

struct Parser {
  Parser(Scanner& scanner);
  auto parseAll() -> bool;
  auto first() -> Statement { return scopes[0]; };
  static auto debug(const Program&) -> void;
  static auto debug(const Statement) -> void;

protected:
  uint ip = 0;
  const vector<Token>& tokens;
  Program scopes;

  auto parse() -> bool;
  auto statement() -> const Statement;

  auto constant() -> const Statement;
  auto variable() -> const Statement;
  auto alien() -> const Statement;

  auto macro() -> const Statement;
  auto _return() -> const Statement;
  auto callOrAlien() -> const Statement;
  auto call() -> const Statement;
  auto mapAssignOrAlien() -> const Statement;
  auto mapAssign() -> const Statement;
  auto mapKey() -> const Statement;
  auto mapItem() -> const Statement;
  auto list() -> const Statement;
  auto defList() -> const Statement;
  auto argument() -> const Statement;
  auto label() -> const Statement;
  auto anonymousLabel() -> const Statement;
//  auto labelRef() -> const Statement;
  auto assignment() -> const Statement;
  auto block() -> const Statement;
  auto blockOrEval() -> const Statement;

  auto ifClause() -> const Statement;
  auto _if() -> const Statement;
  auto _else() -> const Statement;
  auto _while() -> const Statement;
  auto _break() -> const Statement;
  auto _continue() -> const Statement;
  auto _namespace() -> const Statement;

  auto cmdPrint() -> const Statement;
  auto cmdArch() -> const Statement;
  auto cmdInclude() -> const Statement;

  auto expression() -> const Statement;
  auto equality() -> const Statement;
  auto comparison() -> const Statement;
  auto term() -> const Statement;
  auto factor() -> const Statement;
  auto unary() -> const Statement;
  auto primary(bool allfeatures = true) -> const Statement;
  auto symbol() -> const Statement;
  auto identifier() -> const Statement;
  auto evaluation() -> const Statement;
  auto reference() -> const Statement;
  auto identOrEval() -> const Statement;

  inline auto peek() -> const Token&;
  inline auto back() -> void;
  inline auto previous() -> const Token&;
  inline auto isAtEnd() -> bool;
  inline auto advance() -> const Token&;
  inline auto check(const TokenType& type) -> bool;
  inline auto check(const TokenType& type, uint ofs) -> bool;
  inline auto consume(const TokenType& type, const string& msg) -> const Token&;
  template <typename... Ts>
  inline auto match(Ts... xs) -> bool;
  auto synchronize() -> void;/**/
};