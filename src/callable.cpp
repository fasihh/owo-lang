#include <callable>

Callable::Callable(const std::string& name, const int n_args, const call_t call_fn) : name(name), n_args(n_args), call_fn(call_fn) {}

size_t Callable::arity() {
  return this->n_args;
}

std::any Callable::call(Interpreter &interpreter, const std::vector<std::any> &arguments) {
  return this->call_fn(interpreter, arguments);
}

const std::string& Callable::to_string() {
  return this->name;
}
