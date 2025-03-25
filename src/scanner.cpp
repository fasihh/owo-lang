#include <map>
#include <owo>
#include <scanner>

std::map<std::string, TokenType> keywords{
  { "if", IF },
  { "for", FOR },
  { "fun", FUN },
  { "and", AND_AND },
  { "or", OR_OR },
  { "nil", NIL },
  { "var", VAR },
  { "else", ELSE },
  { "break", BREAK },
  { "continue", CONTINUE },
  { "this", THIS },
  { "true", TRUE },
  { "class", CLASS },
  { "false", FALSE },
//   { "print", PRINT },
  { "super", SUPER },
  { "while", WHILE },
  { "return", RETURN },
};

Scanner::Scanner(const std::string& source) : source(source) {}

std::string Scanner::get(size_t i, size_t j) { return source.substr(i, j-i); }
bool Scanner::at_end() { return current >= source.size(); }
char Scanner::advance() { return source[current++]; }
bool Scanner::is_digit(char c) { return c >= '0' && c <= '9'; }
bool Scanner::is_alpha(char c) { return (c <= 'Z' && c >= 'A') || (c <= 'z' && c >= 'a') || c == '_'; }
bool Scanner::is_alpha_numeric(char c) { return is_digit(c) || is_alpha(c); }
void Scanner::add_token(TokenType type) { add_token(type, nullptr); }
void Scanner::add_token(TokenType type, std::any object) { tokens.push_back(std::make_unique<Token>(type, get(start, current), object, line)); }
char Scanner::peek() { return at_end() ? '\0' : source[current]; }
char Scanner::peek_next() { return current + 1 >= source.size() ? '\0' : source[current + 1]; }

bool Scanner::match(char expected) {
    if (at_end()) return false;
    if (source[current] != expected) return false;
    current++;
    return true;
}

void Scanner::string() {
    while (peek() != '"' && !at_end()) {
        if (peek() == '\n') line++;
        advance();
    }

    if (at_end()) {
        owo::error(line, "Unterminated string");
        return;
    }

    advance();
    std::string value = get(start+1, current-1);
    add_token(STRING, value);
}

void Scanner::number() {
    while (is_digit(peek())) advance();

    if (peek() == '.' && is_digit(peek_next())) {
        advance();
        while (is_digit(peek())) advance();
    }

    std::string value = get(start, current);
    add_token(NUMBER, std::stod(value));
}

void Scanner::identifier() {
    while (is_alpha_numeric(peek())) advance();

    std::string text = get(start, current);
    auto it = keywords.find(text);
    TokenType type = (it != keywords.end()) ? it->second : IDENTIFIER;
    add_token(type);
}

void Scanner::scan_token() {
    char c = advance();
    switch (c) {
    case '(': return add_token(LEFT_PAREN);
    case ')': return add_token(RIGHT_PAREN);
    case '{': return add_token(LEFT_BRACE);
    case '}': return add_token(RIGHT_BRACE);
    case ',': return add_token(COMMA);
    case '.': return add_token(DOT);
    case '-': return add_token(MINUS);
    case '+': return add_token(PLUS);
    case ';': return add_token(SEMICOLON);
    case '*': return add_token(STAR);
    case '?': return add_token(QUESTION);
    case ':': return add_token(COLON);
    case '^': return add_token(XOR);
    case '~': return add_token(NOT);
    case '%': return add_token(PERCENTAGE);
    case '&': return add_token(AND);
    case '|': return add_token(OR);
    case '!': return add_token(match('=') ? BANG_EQUAL : BANG);
    case '=': return add_token(match('=') ? EQUAL_EQUAL : EQUAL);
    case '<': return add_token(match('=') ? LESS_EQUAL : match('<') ? LEFT_SHIFT : LESS);
    case '>': return add_token(match('=') ? GREATER_EQUAL : match('>') ? RIGHT_SHIFT : GREATER);
    case '/':
        if (match('/'))
            while (peek() != '\n' && !at_end()) advance();
        else
            add_token(SLASH);
        break;
    case ' ':
        // ignore whitespace
    case '\r':
        // ignore carriage reverse thing
    case '\t':
        break;
    case '\n':
        line++;
        break;
    case '"': return string();
    default:
        if (is_digit(c)) return number();
        else if (is_alpha(c)) return identifier();
        owo::error(line, "Unexpected character: " + std::string(1, c));
        return;
    }
}

const std::vector<std::unique_ptr<Token>>& Scanner::scan_tokens() {
    while (!at_end()) {
        start = current;
        scan_token();
    }

    tokens.push_back(std::make_unique<Token>(OWO_EOF, "", nullptr, line));
    return tokens;
}