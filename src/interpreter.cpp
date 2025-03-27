#include <interpreter>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <callable-function>
#include <owo>

bool is_string(const std::any& obj) {
  return obj.type() == typeid(std::string);
}

bool is_double(const std::any& obj) {
  return obj.type() == typeid(double);
}

bool is_bool(const std::any& obj) {
  return obj.type() == typeid(bool);
}

double get_double(const std::any& obj) {
  return std::any_cast<double>(obj);
}

std::string get_string(const std::any& obj) {
  return std::any_cast<std::string>(obj);
}

bool get_bool(const std::any& obj) {
  return std::any_cast<bool>(obj);
}

std::string double_to_string(const double value) {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(15) << value;
  std::string str_value = oss.str();

  str_value.erase(str_value.find_last_not_of('0') + 1);
  if (str_value.back() == '.')
    str_value.pop_back();
  return str_value;
}

std::any Interpreter::evaluate(Expr& expr) {
  return expr.accept(*this);
}

nullptr_t Interpreter::execute(Stmt& stmt) {
  stmt.accept(*this);
  return nullptr;
}

void Interpreter::interpret(const std::vector<std::unique_ptr<Stmt>> &stmts) {
  try {
    for (const auto& stmt : stmts)
      execute(*stmt);
  } catch (const RuntimeError& error) {
    owo::runtime_error(error);
  }
}

void Interpreter::set_mode(const int mode) { this->mode = mode; }

bool Interpreter::is_equal(const std::any& left, const std::any& right) {
  if (is_double(left) && is_double(right))
    return get_double(left) == get_double(right);

  if (is_string(left) && is_string(right))
    return get_string(left) == get_string(right);

  if (is_bool(left) && is_bool(right))
    return get_bool(left) == get_bool(right);

  return !is_truthy(left) && !is_truthy(right);
}

bool Interpreter::is_truthy(const std::any& obj) {
  if (is_double(obj)) return get_double(obj) != 0.f;
  if (is_string(obj)) return get_string(obj).size() != 0;
  if (is_bool(obj)) return get_bool(obj);
  return false;
}

void Interpreter::check_number_operand(const Token* token, const std::any& op) {
  if (is_double(op)) return;
  throw RuntimeError("Operand must be of type number", token);
}

void Interpreter::check_number_operands(const Token* token, const std::any& left, const std::any& right) {
  if (is_double(left) && is_double(right)) return;
  throw RuntimeError("Operands must be of type number", token);
}

void Interpreter::execute_block(const std::vector<std::unique_ptr<Stmt>>& stmts, Environment* env) {
  Environment* previous = this->env;
  try {
    this->env = env;
    for (const auto& stmt : stmts)
      execute(*stmt);
  } catch (const ReturnException& ) {
    this->env = previous;
    throw;
  }
  this->env = previous;
}

Interpreter::Interpreter() : env(new Environment) {
  // make environment not take token itself
  // handle runtime error taking token elsewhere
  // maybe outside instead

  env->define(
    "print",
    Callable("<native_fn>", 1, [=](Interpreter& interpreter, const std::vector<std::any>& arguments) {
      for (const auto& arg : arguments)
        std::cout << arg << std::endl;
      return std::any(nullptr);
    }),
    nullptr
  );
}

Interpreter::~Interpreter() {
  delete env;
}

std::any Interpreter::visitBinaryExpr(Binary &expr) {
  std::any left = evaluate(*expr.left);
  std::any right = evaluate(*expr.right);

  // std::cout << *expr.op << std::endl;
  // std::cout << left << " " << right << std::endl;

  switch (expr.op->type) {
  case TokenType::PLUS:
    if (is_string(left) && is_string(right)) {
      return get_string(left) + get_string(right);
    } else if (is_double(left) && is_double(right)) {
      return get_double(left) + get_double(right);
    } else if (is_string(left) && is_double(right)) {
      return get_string(left) + double_to_string(get_double(right));
    } else if (is_double(left) && is_string(right)) {
      return double_to_string(get_double(left)) + get_string(right);
    }
    throw RuntimeError("Operands must be of type number and/or string", expr.op);
  
  case TokenType::MINUS:
    check_number_operands(expr.op, left, right);
    return get_double(left) - get_double(right);
  
  case TokenType::STAR:
    check_number_operands(expr.op, left, right);
    return get_double(left) * get_double(right);
  
  case TokenType::SLASH:
    check_number_operands(expr.op, left, right);
    return get_double(left) / get_double(right);
  
  case TokenType::PERCENTAGE:
    check_number_operands(expr.op, left, right);
    return static_cast<double>((int)get_double(left) % (int)get_double(right));
  
  case TokenType::GREATER:
    check_number_operands(expr.op, left, right);
    return get_double(left) > get_double(right);
  
  case TokenType::GREATER_EQUAL:
    check_number_operands(expr.op, left, right);
    return get_double(left) >= get_double(right);
  
  case TokenType::LESS:
    check_number_operands(expr.op, left, right);
    return get_double(left) < get_double(right);
  
  case TokenType::LESS_EQUAL:
    check_number_operands(expr.op, left, right);
    return get_double(left) <= get_double(right);
  
  case TokenType::EQUAL_EQUAL:
    return is_equal(left, right);
  
  case TokenType::BANG_EQUAL:
    return !is_equal(left, right);
  
  case TokenType::AND:
    check_number_operands(expr.op, left, right);
    return static_cast<double>((int)get_double(left) & (int)get_double(right));
  
  case TokenType::OR:
    check_number_operands(expr.op, left, right);
    return static_cast<double>((int)get_double(left) | (int)get_double(right));
  
  case TokenType::XOR:
    check_number_operands(expr.op, left, right);
    return static_cast<double>((int)get_double(left) ^ (int)get_double(right));
  
  case TokenType::LEFT_SHIFT:
    check_number_operands(expr.op, left, right);
    return static_cast<double>((int)get_double(left) << (int)get_double(right));
  
  case TokenType::RIGHT_SHIFT:
    check_number_operands(expr.op, left, right);
    return static_cast<double>((int)get_double(left) >> (int)get_double(right));
  
  case TokenType::AND_AND:
    return !is_truthy(left) ? false : is_truthy(right);
  
  case TokenType::OR_OR:
    return is_truthy(left) ? true : is_truthy(right);
  }

  return nullptr;
}

std::any Interpreter::visitAssignExpr(Assign& expr) {
  return env->assign(expr.name->lexeme, evaluate(*expr.value), expr.name);
}

std::any Interpreter::visitGroupingExpr(Grouping &expr) {
  return evaluate(*expr.expression);
}

std::any Interpreter::visitLiteralExpr(Literal &expr) {
  return expr.value;
}

std::any Interpreter::visitUnaryExpr(Unary &expr) {
  std::any right = evaluate(*expr.right);

  switch (expr.op->type) {
  case TokenType::MINUS:
    check_number_operand(expr.op, right);
    return -get_double(right);
  case TokenType::BANG:
    return !is_truthy(right);
  case TokenType::NOT:
    check_number_operand(expr.op, right);
    return static_cast<double>(~(int)get_double(right));
  }

  return nullptr;
}

std::any Interpreter::visitCallExpr(Call &expr) {
  std::any callee = evaluate(*expr.callee);

  std::vector<std::any> arguments;
  for (const auto& arg : expr.args)
    arguments.push_back(evaluate(*arg));

  if (callee.type() == typeid(Callable)) {
    Callable func = std::any_cast<Callable>(callee);
    if (arguments.size() != func.arity())
      throw RuntimeError("Expected " + std::to_string(func.arity()) + " arguments but got " + std::to_string(arguments.size()) + ".", expr.paren);
    return func.call(*this, arguments);

  } else if (callee.type() == typeid(CallableFunction)) {
    CallableFunction func = std::any_cast<CallableFunction>(callee);
    if (arguments.size() != func.arity())
      throw RuntimeError("Expected " + std::to_string(func.arity()) + " arguments but got " + std::to_string(arguments.size()) + ".", expr.paren);
    return func.call(*this, arguments);

  } else {
    throw RuntimeError("Can only call functions and classes.", expr.paren);
  }
}

std::any Interpreter::visitVariableExpr(Variable &expr) {
  return env->get(expr.label->lexeme, expr.label);
}

std::any Interpreter::visitTernaryExpr(Ternary &expr) {
  std::any condition = evaluate(*expr.condition);
  return evaluate(is_truthy(condition) ? *expr.true_case : *expr.false_case);
}

void Interpreter::visitExpressionStmt(Expression &stmt) {
  for (const auto& expr : stmt.expressions) {
    std::any value = evaluate(*expr);
    if (mode == 1)
      std::cout << value << std::endl;
  }
}

void Interpreter::visitVarStmt(Var &stmt) {
  for (const auto& [label, value] : stmt.variables)
    env->define(label->lexeme, value ? evaluate(*value) : nullptr, label);
}

void Interpreter::visitBlockStmt(Block &stmt) {
  Environment* new_env = new Environment(this->env);
  execute_block(stmt.statements, new_env);
  delete new_env;
}

void Interpreter::visitIfStmt(If &stmt) {
  if (is_truthy(evaluate(*stmt.condition)))
    execute(*stmt.if_case);
  else if (stmt.else_case)
    execute(*stmt.else_case);
}

void Interpreter::visitFunctionStmt(Function &stmt) {
  CallableFunction function = CallableFunction(stmt);
  env->define(stmt.name->lexeme, function, stmt.name);
}

void Interpreter::visitReturnStmt(Return &stmt) {
  throw ReturnException(stmt.value ? evaluate(*stmt.value) : nullptr);
}
