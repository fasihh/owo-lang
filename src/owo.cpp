#include <owo>
#include <iostream>
#include <fstream>
#include <scanner>

void owo::run(const std::string& source, const int mode) {
  Scanner scanner = Scanner(source);
  const std::vector<std::unique_ptr<Token>>& tokens = scanner.scan_tokens();

  if (owo::had_error) return;

  for (const auto& token : tokens)
    std::cout << *token << std::endl;
  std::cout << mode << std::endl;
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

  owo::run(content, 0);

  if (owo::had_error)
    exit(64);
  if (owo::had_runtime_error)
    exit(70);
}

void owo::run_prompt() {
  std::string input_buffer;
  while (true) {
    std::cout << ">>> ";
    std::getline(std::cin, input_buffer);

    if (input_buffer == "exit")
      break;

    owo::run(input_buffer, 1);
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

void owo::error(std::unique_ptr<Token> token, std::string message) {
  if (token->type == OWO_EOF)
    owo::report(token->line, "at end", message);
  else
    owo::report(token->line, "at '" + token->lexeme + "'", message);
}

bool owo::had_error = false;
bool owo::had_runtime_error = false;