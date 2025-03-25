#include <parser>
#include <owo>

Parser::Parser(const std::vector<std::unique_ptr<Token>>& tokens) : tokens(tokens) {}

bool Parser::match(std::vector<TokenType> types) {
  for (TokenType type : types) {
    if (check(type)) {
      advance();
      return true;
    }
  }
  return false;
}

bool Parser::check(TokenType type) { return at_end() ? false : peek()->type == type; }
bool Parser::at_end() { return peek()->type == OWO_EOF; }

const Token* Parser::advance() {
  if (!at_end()) current++;
  return peek();
}

const Token* Parser::peek() { return tokens[current].get(); }
const Token* Parser::previous() { return tokens[current-1].get(); }

const Token* Parser::consume(TokenType type, const std::string& message) {
  if (check(type)) return advance();
  throw error(peek(), message);
}

ParseError Parser::error(const Token* token, const std::string& message) {
  owo::error(token, message);
  return ParseError(message);
}

void Parser::synchronize() {
  advance();

  while (!at_end()) {
    if (previous()->type == SEMICOLON) return;
    switch(peek()->type) {
      case CLASS:
      case FUN:
      case VAR:
      case FOR:
      case IF:
      case WHILE:
      case PRINT:
      case RETURN:
        return;
    }
    advance();
  }
}

std::unique_ptr<Expr> Parser::finish_call(std::unique_ptr<Expr> callee) {
  std::vector<std::unique_ptr<Expr>> arguments;

  if (!check(RIGHT_PAREN)) {
    do {
      if (arguments.size() >= 255)
        error(peek(), "Can't have more than 255 arguments.");
      arguments.push_back(expression());
    } while (match({ COMMA }));
  }

  const Token *paren = consume(RIGHT_PAREN, "Expect ')' after arguments.");

  return std::make_unique<Call>(std::move(callee), paren, std::move(arguments));
}

std::vector<std::unique_ptr<Expr>> Parser::comma() {
  std::vector<std::unique_ptr<Expr>> expressions;

  do {
    std::unique_ptr<Expr> expr = expression();
    expressions.push_back(std::move(expr));
  } while (match({ COMMA }));

  return std::move(expressions);
}

std::unique_ptr<Expr> Parser::expression() {
  return assignment();
}

std::unique_ptr<Expr> Parser::assignment() {
  std::unique_ptr<Expr> expr = ternary();

  if (match({ EQUAL })) {
    const Token* equals = previous();
    std::unique_ptr<Expr> value = assignment();

    if (auto* variable = dynamic_cast<Variable*>(expr.get()))
      return std::make_unique<Assign>(variable->label, std::move(value));

    error(equals, "Invalid assignment target.");
  }

  return expr;
}

std::unique_ptr<Expr> Parser::ternary() {
  std::unique_ptr<Expr> expr = equality();

  if (match({ QUESTION })) {
    std::unique_ptr<Expr> true_case = expression();
    consume(COLON, "Expected ':' afer true branch.");
    std::unique_ptr<Expr> false_case = ternary();
    return std::make_unique<Ternary>(std::move(expr), std::move(true_case), std::move(false_case));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::equality() {
  std::unique_ptr<Expr> expr = bitwise();

  while (match({ BANG_EQUAL, EQUAL_EQUAL })) {
    const Token* op = previous();
    std::unique_ptr<Expr> right = bitwise();
    expr = std::make_unique<Binary>(std::move(expr), std::move(op), std::move(right));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::bitwise() {
  std::unique_ptr<Expr> expr = logical();

  while (match({ AND, OR, XOR, LEFT_SHIFT, RIGHT_SHIFT })) {
    const Token* op = previous();
    std::unique_ptr<Expr> right = logical();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::logical() {
  std::unique_ptr<Expr> expr = comparison();

  while (match({ AND_AND, OR_OR })) {
    const Token* op = previous();
    std::unique_ptr<Expr> right = comparison();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
  std::unique_ptr<Expr> expr = term();

  while (match({ GREATER, GREATER_EQUAL, LESS, LESS_EQUAL })) {
    const Token* op = previous();
    std::unique_ptr<Expr> right = term();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::term() {
  std::unique_ptr<Expr> expr = factor();

  while (match({ MINUS, PLUS })) {
    const Token* op = previous();
    std::unique_ptr<Expr> right = factor();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::factor() {
  std::unique_ptr<Expr> expr = unary();

  while (match({ STAR, SLASH, PERCENTAGE })) {
    const Token *op = previous();
    std::unique_ptr<Expr> right = unary();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::unary() {
  if (match({ BANG, MINUS, NOT })) {
    const Token* op = previous();
    std::unique_ptr<Expr> right = unary();
    return std::make_unique<Unary>(op, std::move(right));
  }

  return call();
}

std::unique_ptr<Expr> Parser::call() {
  std::unique_ptr<Expr> expr = primary();

  while(match({ LEFT_PAREN }))
    expr = finish_call(std::move(expr));

  return expr;
}

std::unique_ptr<Expr> Parser::primary() {
  if (match({ FALSE }))
    return std::make_unique<Literal>(false);
  if (match({ TRUE }))
    return std::make_unique<Literal>(true);
  if (match({ NIL }))
    return std::make_unique<Literal>(nullptr);
  if (match({ NUMBER }))
    return std::make_unique<Literal>(std::any_cast<double>(previous()->object));
  if (match({ STRING }))
    return std::make_unique<Literal>(std::any_cast<std::string>(previous()->object));
  if (match({ IDENTIFIER })) return std::make_unique<Variable>(previous());

  if (match({ LEFT_PAREN })) {
    std::unique_ptr<Expr> expr = expression();
    consume(RIGHT_PAREN, "Expect ')' after expression.");
    return std::make_unique<Grouping>(std::move(expr));
  }

  throw error(peek(), "Expect expression.");
}

std::unique_ptr<Stmt> Parser::expr_stmt() {
  std::vector<std::unique_ptr<Expr>> values = comma();
  consume(SEMICOLON, "Expect ';' after expression.");
  return std::make_unique<Expression>(std::move(values));
}

std::unique_ptr<Stmt> Parser::declaration() {
  try {
    if (match({ VAR })) return var_declaration();
    if (match({ FUN })) return func("function");
    if (match({ IF })) return if_stmt();
    // if (match({ WHILE })) return while_stmt();
    // if (match({ FOR })) return for_stmt();
    return statement();
  } catch (ParseError error) {
    synchronize();
    return nullptr;
  }
}

std::unique_ptr<Stmt> Parser::var_declaration() {
  std::vector<std::pair<const Token*, std::unique_ptr<Expr>>> variables;

  while (true) {
    consume(IDENTIFIER, "Expect variable name.");
    const Token* name = previous();

    std::unique_ptr<Expr> initializer;
    if (match({ EQUAL }))
      initializer = std::move(expression());
    
    variables.push_back({name, std::move(initializer)});

    if (!match({ COMMA }))
      break;
  }

  consume(SEMICOLON, "Expect ';' after variable declaration.");
  return std::make_unique<Var>(std::move(variables));
}

std::unique_ptr<Stmt> Parser::func(const std::string &kind) {
  consume(IDENTIFIER, "Expect " + kind + " name.");
  const Token* name = previous();
  consume(LEFT_PAREN, "Expect '(' after " + kind + " name.");
  std::vector<const Token*> params;
  if (!check(RIGHT_PAREN)) {
    do {
      if (params.size() >= 255)
        error(peek(), "Can't have more than 255 arguments.");
      consume(IDENTIFIER, "Expect parameter name.");
      const Token* param = previous();
      params.push_back(param);
    } while (match({ COMMA }));
  }
  consume(RIGHT_PAREN, "Expect ')' after " + kind + " parameter list.");

  consume(LEFT_BRACE, "Expect '{' before " + kind + " body.");
  std::vector<std::unique_ptr<Stmt>> body(std::move(block()));
  return std::make_unique<Function>(name, params, std::move(body));
}

std::unique_ptr<Stmt> Parser::statement() {
  if (match({ LEFT_BRACE })) return std::make_unique<Block>(std::move(block()));
  if (match({ RETURN })) return return_stmt();
  return expr_stmt();
}

std::unique_ptr<Stmt> Parser::if_stmt() {
  consume(LEFT_PAREN, "Expect '(' after 'if'.");
  std::unique_ptr<Expr> expr = expression();
  consume(RIGHT_PAREN, "Expect ')' after 'if' condition.");

  std::unique_ptr<Stmt> then_branch = statement();
  std::unique_ptr<Stmt> else_branch;
  if (match({ ELSE }))
    else_branch = statement();
  
  return std::make_unique<If>(std::move(expr), std::move(then_branch), std::move(else_branch));
}

std::unique_ptr<Stmt> Parser::return_stmt() {
  const Token* keyword = previous();
  std::unique_ptr<Expr> value = nullptr;

  if (!check(SEMICOLON))
      value = expression();

  consume(SEMICOLON, "Expect ';' after 'return' value.");
  return std::make_unique<Return>(keyword, std::move(value));
}

std::vector<std::unique_ptr<Stmt>> Parser::block() {
  std::vector<std::unique_ptr<Stmt>> statements;

  while (!check(RIGHT_BRACE) && !at_end())
    statements.push_back(std::move(declaration()));

  consume(RIGHT_BRACE, "Expect '}' after block.");
  return statements;
}

const std::vector<std::unique_ptr<Stmt>> &Parser::parse() {
  statements.clear();

  while (!at_end())
    statements.push_back(std::move(declaration()));

  return statements;
}
