#include <owo>
#include <iostream>
#include <fstream>
#include <scanner>
#include <parser>
#include <ast-printer>

void owo::run(const std::string& source, Interpreter& interpreter, const int mode) {
  Scanner scanner = Scanner(source);
  const std::vector<std::unique_ptr<Token>>& tokens = scanner.scan_tokens();
  Parser parser = Parser(tokens);
  const std::vector<std::unique_ptr<Stmt>>& stmts = parser.parse();

  if (owo::had_error) return;

  interpreter.set_mode(mode);
  interpreter.interpret(stmts);
}

void owo::run_file(const std::string& path) {
  std::ifstream file(path, std::ios::binary);

  if (!file)
    throw std::runtime_error("Failed to open file: " + std::string(path));

  file.seekg(0, std::ios::end);
  size_t size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::string content(size, '\0');

  if (!file.read(&content[0], size))
    throw std::runtime_error("Error reading file: " + std::string(path));

  Interpreter interpreter;
  owo::run(content, interpreter, 0);

  if (owo::had_error)
    exit(64);
  if (owo::had_runtime_error)
    exit(70);
}

void owo::run_prompt() {
  Interpreter interpreter;
  std::string input_buffer;
  while (true) {
    std::cout << ">>> ";
    std::getline(std::cin, input_buffer);

    if (input_buffer == "exit")
      break;

    try {
      owo::run(input_buffer, interpreter, 1);
    } catch (const std::runtime_error& ) {}
    owo::had_error = false;
  }
}

void owo::error(int line, std::string message) {
  owo::report(line, "", message);
}

void owo::report(int line, std::string where, std::string message) {
  std::cout << "[line " << line << "] Error " << where << ": " << message << std::endl;
  owo::had_error = true;
}

void owo::error(const Token* token, std::string message) {
  if (token->type == OWO_EOF)
    owo::report(token->line, "at end", message);
  else
    owo::report(token->line, "at '" + token->lexeme + "'", message);
}

void owo::runtime_error(const RuntimeError& error) {
  std::cout << error.what() << "\n[line " << error.token->line << "]" << std::endl;
  owo::had_runtime_error = true;
}

bool owo::had_error = false;
bool owo::had_runtime_error = false;