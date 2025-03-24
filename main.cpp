#include <iostream>
#include <owo>

int main(int argc, char *argv[]) {
  try {
    if (argc > 2) {
      std::cout << "Usage: owo [script]" << std::endl;
      exit(64);
    } else if (argc == 2) {
      owo::run_file(argv[1]);
    } else {
      owo::run_prompt();
    }
  } catch (const std::runtime_error& error) {
    std::cout << error.what() << std::endl;
  }

  return 0;
}