#include <ast-printer>
#include <sstream>
#include <iomanip>
#include <iostream>

std::string AstPrinter::parenthesize(const std::string& name, const std::vector<Expr*>& exprs) {
  std::stringstream ss;
  ss << "(" << name;
  for (auto& expr : exprs)
    ss << " " << expr->accept(*this);
  ss << ")";
  return ss.str();
}

std::string AstPrinter::print(Expr& expr) {
  return expr.accept(*this);
}

void AstPrinter::visitBinaryExpr(Binary& expr) {
  const std::vector<Expr*> exprs{expr.left.get(), expr.right.get()};
  result_expr = parenthesize(expr.op->lexeme, exprs);
}

void AstPrinter::visitAssignExpr(Assign& expr) {
  const std::vector<Expr*> exprs{expr.value.get()};
  result_expr = parenthesize("=" + expr.name->lexeme, exprs);
}

void AstPrinter::visitGroupingExpr(Grouping& expr) {
  const std::vector<Expr*> exprs{expr.expression.get()};
  result_expr = parenthesize("group", exprs);
}

void AstPrinter::visitLiteralExpr(Literal& expr) {
  if (expr.value.has_value()) {
    try {
      if (expr.value.type() == typeid(std::string))
        result_expr = std::any_cast<std::string>(expr.value);
      else if (expr.value.type() == typeid(int))
        result_expr = std::to_string(std::any_cast<int>(expr.value));
      else if (expr.value.type() == typeid(double)) {
        double value = std::any_cast<double>(expr.value);
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(15) << value;
        std::string str_value = oss.str();

        str_value.erase(str_value.find_last_not_of('0') + 1);
        if (str_value.back() == '.')
          str_value.pop_back();

        result_expr = str_value;
      }
      else
        result_expr = "<unknown>";
    } catch (std::bad_any_cast& ) {
      result_expr = "<invalid>";
    }
  } else {
    result_expr = "nil";
  }
}

void AstPrinter::visitUnaryExpr(Unary& expr) {
  const std::vector<Expr*> exprs{expr.right.get()};
  result_expr = parenthesize(expr.op->lexeme, exprs);
}

// void AstPrinter::visitCallExpr(Call& expr) {
//   std::vector<Expr*> exprs{expr.callee.get()};
//   for (auto& arg : expr.args)
//     exprs.push_back(arg.get());
//   result_expr = parenthesize("call", exprs);
// }

void AstPrinter::visitVariableExpr(Variable& expr) {
  result_expr = parenthesize(expr.label->lexeme, {});
}

void AstPrinter::visitTernaryExpr(Ternary& expr) {
  const std::vector<Expr*> exprs{expr.condition.get(), expr.true_case.get(), expr.false_case.get()};
  result_expr = parenthesize("?:", exprs);
}
