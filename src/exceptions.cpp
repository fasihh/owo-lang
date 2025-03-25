#include <exceptions>

RuntimeError::RuntimeError(const std::string& message, const Token* token) : std::runtime_error(message), token(token) {}

ReturnException::ReturnException(const std::any& value) : value(value) {}
