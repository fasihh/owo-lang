#include <callable-function>
#include <exceptions>

CallableFunction::CallableFunction(const Function& declaration) 
  : declaration(declaration), Callable("<fn " + declaration.name->lexeme + ">", declaration.params.size()) {}

std::any CallableFunction::call(Interpreter& interpreter, const std::vector<std::any>& arguments) {
  std::unique_ptr<Environment> env = std::make_unique<Environment>(interpreter.env.get());
  for (size_t i = 0; i < this->arity(); ++i)
    env->define(declaration.params[i]->lexeme, arguments[i], declaration.params[i]);

  try {
    interpreter.execute_block(declaration.body, env);
  } catch (const ReturnException& ret) {
    return ret.value;
  }
  return std::any(nullptr);
}
