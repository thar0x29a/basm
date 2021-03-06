struct Architecture {
  Architecture(Plek& self) : self(self) {
  }

  virtual ~Architecture() {
  }

  virtual auto assemble(const string& statement) -> bool {
    return false;
  }

  virtual auto assemble(Statement statement) -> bool {
    return false;
  }

  //
  auto pc() const -> int64_t {
    return self.pc();
  }

  auto endian() const -> Endian {
    return self.endian;
  }

  auto setEndian(Endian endian) -> void {
    self.endian = endian;
  }

  auto directives() -> Directives& {
    return self.directives;
  }

  auto readArchitecture(const string& s) -> string {
    return self.readArchitecture(s);
  }

  auto evaluate(const string& expression, EvaluationMode mode = EvaluationMode::Default) -> int64_t {
    self.notice(expression);
    throw string{"Not implemented!"};
    return 0;
  }

  auto evaluate(Statement expression, EvaluationMode mode = EvaluationMode::Default) -> int64_t {
    return self.evaluateRHS(expression).getInt();
  }

  auto write(uint64_t data, uint length = 1) -> void {
    return self.write(data, length);
  }

  template<typename... P> auto notice(P&&... p) -> void {
    return self.notice(forward<P>(p)...);
  }

  template<typename... P> auto warning(P&&... p) -> void {
    return self.warning(forward<P>(p)...);
  }

  template<typename... P> auto error(P&&... p) -> void {
    return self.error(forward<P>(p)...);
  }

  Plek& self;
};
