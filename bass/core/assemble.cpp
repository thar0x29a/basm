auto Plek::assemble(Statement stmt) -> bool {
  string key = stmt->value.getString();
  notice(key);

  return false;
}