#include <environment>
#include <token>
#include <exceptions>
#include <iostream>

Environment::Environment(Environment* enclosing) : enclosing(enclosing) {}

void Environment::define(const std::string& name, const std::any& value, const Token* token) {
  ValuesMap::iterator it = values.find(name);
  if (it == values.end()) {
    values[name] = value;
    return;
  }
  throw RuntimeError("Variable '" + name + "' has already been declared.", token);
}

std::any Environment::get(const std::string& name, const Token* token) {
  ValuesMap::iterator it = values.find(name);
  if (it != values.end())
    return it->second;
  if (enclosing)
    return enclosing->get(name, token);
  throw RuntimeError("Undefined variable '" + name + "'.", token);
}

std::any Environment::assign(const std::string& name, const std::any& value, const Token* token) {
  ValuesMap::iterator it = values.find(name);
  if (it != values.end()) {
    values[name] = value;
    return value;
  }
  if (enclosing)
    return enclosing->assign(name, value, token);
  throw RuntimeError("Undefined variable '" + name + "'.", token);
}

void Environment::show_all() {
  for (const auto& [key, value] : values)
    std::cout << key << " = " << value << std::endl;
}