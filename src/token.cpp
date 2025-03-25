#include <token>
#include <callable-function>

std::string token_type_to_string(TokenType type) {
  switch (type) {
    case LEFT_PAREN: return "LEFT_PAREN";
    case RIGHT_PAREN: return "RIGHT_PAREN";
    case LEFT_BRACE: return "LEFT_BRACE";
    case RIGHT_BRACE: return "RIGHT_BRACE";
    case COMMA: return "COMMA";
    case NOT: return "NOT";
    case PLUS: return "PLUS";
    case SEMICOLON: return "SEMICOLON";
    case SLASH: return "SLASH";
    case STAR: return "STAR";
    case QUESTION: return "QUESTION";
    case COLON: return "COLON";
    case DOT: return "DOT";
    case MINUS: return "MINUS";
    case PERCENTAGE: return "PERCENTAGE";
    
    case AND_AND: return "AND_AND";
    case OR_OR: return "OR_OR";
    case BANG: return "BANG";
    case BANG_EQUAL: return "BANG_EQUAL";
    case EQUAL: return "EQUAL";
    case EQUAL_EQUAL: return "EQUAL_EQUAL";
    case GREATER: return "GREATER";
    case GREATER_EQUAL: return "GREATER_EQUAL";
    case LESS: return "LESS";
    case LESS_EQUAL: return "LESS_EQUAL";
    case LEFT_SHIFT: return "LEFT_SHIFT";
    case RIGHT_SHIFT: return "RIGHT_SHIFT";
    
    case IDENTIFIER: return "IDENTIFIER";
    case STRING: return "STRING";
    case NUMBER: return "NUMBER";
    
    case AND: return "AND";
    case CLASS: return "CLASS";
    case ELSE: return "ELSE";
    case FALSE: return "FALSE";
    case FUN: return "FUN";
    case FOR: return "FOR";
    case IF: return "IF";
    case NIL: return "NIL";
    case OR: return "OR";
    case PRINT: return "PRINT";
    case RETURN: return "RETURN";
    case SUPER: return "SUPER";
    case THIS: return "THIS";
    case TRUE: return "TRUE";
    case VAR: return "VAR";
    case WHILE: return "WHILE";
    case XOR: return "XOR";
    case BREAK: return "BREAK";
    case CONTINUE: return "CONTINUE";
    
    case OWO_EOF: return "OWO_EOF";

    default: return "UNKNOWN_TOKEN";
  }
}


Token::Token() : type(OWO_EOF), lexeme(""), object(nullptr), line(0) {}

Token::Token(TokenType type, std::string lexeme, std::any object, int line)
  : type(type), lexeme(lexeme), object(object), line(line) {}

std::ostream& operator<<(std::ostream& out, const std::any& obj) {
  if (obj.has_value()) {
    try {
      if (obj.type() == typeid(std::string)) {
        out << std::any_cast<std::string>(obj);
      } else if (obj.type() == typeid(double)) {
        out << std::any_cast<double>(obj);
      } else if (obj.type() == typeid(int)) {
        out << std::any_cast<int>(obj);
      } else if (obj.type() == typeid(bool)) {
        out << (std::any_cast<bool>(obj) ? "true" : "false");
      } else if (obj.type() == typeid(Callable)) {
        out << std::any_cast<Callable>(obj).to_string();
      } else if (obj.type() == typeid(CallableFunction)) {
        out << std::any_cast<CallableFunction>(obj).to_string();
      } else {
        out << "nil";
      }
    } catch (const std::bad_any_cast& e) {
      out << "<bad_any_cast>";
    }
  } else {
    out << "<none>";
  }

  return out;
}

std::ostream& operator<<(std::ostream& out, const Token& obj) {
  out << token_type_to_string(obj.type) << " " << obj.lexeme << " " << obj.object << " (line " << obj.line << ")";
  return out;
}