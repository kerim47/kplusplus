#include <exception>
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <map>
#include <chrono>
#include <thread>
#include <unistd.h>

bool IsNullOrWhitespace(const std::string& str) {
  return str.empty() || std::all_of(str.begin(), str.end(), [](unsigned char c) {
    return std::isspace(c);
  });
}

namespace Status {
  enum class WARN {
    IsNullOrWhitespace = 1
  };

  enum class ERROR {
    InvalidExpression = 1
  };

  enum class ALL {
    ERROR,
    WARN
  };
}

class MyException {
public:
  static std::string Exception(Status::ALL errorCode, const std::string& message);
};

std::string MyException::Exception(Status::ALL errorCode, const std::string& message) {
  switch (errorCode) {
    case Status::ALL::ERROR:
      throw std::runtime_error("Error: " + message);
    case Status::ALL::WARN:
      throw std::runtime_error("Warning: " + message);
    default:
      throw std::runtime_error("Unknown error occurred.");
  }
}

enum class SyntaxKind {
  NumberToken,
  PlusToken,
  MinusToken,
  StarToken,
  SlashToken,
  OpenParenthesisToken,
  CloseParenthesisToken,
  BadToken,
  EndOfFileToken
};

std::string GetSyntaxKindName(SyntaxKind kind) {
  static const std::map<SyntaxKind, std::string> syntaxKindNames = {
    { SyntaxKind::NumberToken, "NumberToken" },
    { SyntaxKind::PlusToken, "PlusToken" },
    { SyntaxKind::MinusToken, "MinusToken" },
    { SyntaxKind::StarToken, "StarToken" },
    { SyntaxKind::SlashToken, "SlashToken" },
    { SyntaxKind::OpenParenthesisToken, "OpenParenthesisToken" },
    { SyntaxKind::CloseParenthesisToken, "CloseParenthesisToken" },
    { SyntaxKind::BadToken, "BadToken" },
    { SyntaxKind::EndOfFileToken, "EndOfFileToken" }
  };

  auto it = syntaxKindNames.find(kind);
  if (it != syntaxKindNames.end()) {
    return it->second;
  }

  return "Unknown";
}

class SyntaxToken {
public:
  SyntaxToken(SyntaxKind kind, int position, std::string text, int value)
    : m_kind(kind), m_position(position), m_text(text), m_value(value) {}

  SyntaxKind getKind() const {
    return m_kind;
  }

  int getPosition() const {
    return m_position;
  }

  std::string getText() const {
    return m_text;
  }

  int getValue() const {
    return m_value;
  }

private:
  SyntaxKind m_kind;
  int m_position;
  std::string m_text;
  int m_value;
};

class Lexer {
public:
  Lexer(std::string text);

  SyntaxToken NextToken();
  char getCurrent();

private:
  std::string m_text;
  int m_position;
  char m_current;

  void Next();
};

Lexer::Lexer(std::string text)
  : m_text(text), m_position(0), m_current(text.empty() ? '\0' : text[0]) {}

char Lexer::getCurrent() {
  return m_current;
}

void Lexer::Next() {
  m_position++;
  m_current = (m_position >= static_cast<int>(m_text.size())) ? '\0' : m_text[m_position];
}

bool TryParseInt(const std::string& str, int& value) {
  std::istringstream iss(str);
  return (iss >> value) ? true : false;
}

SyntaxToken Lexer::NextToken() {
  int start = m_position;
  int length = 0;
  int obj_value = 0;

  if (m_position >= std::size(m_text)) {
    static SyntaxToken token(SyntaxKind::EndOfFileToken, start, "\0", obj_value);
    return token;
  }

  if (std::isdigit(getCurrent())) {
    while (std::isdigit(getCurrent())) {
      Next();
    }
    length = m_position - start;
    std::string text = m_text.substr(start, length);
    TryParseInt(text, obj_value);

    return SyntaxToken(SyntaxKind::NumberToken, start, text, obj_value);
  }

  if (std::isspace(getCurrent())) {
    while (std::isspace(getCurrent())) {
      Next();
    }
    length = m_position - start;
    std::string text = m_text.substr(start, length);
    TryParseInt(text, obj_value);
    static SyntaxToken token(SyntaxKind::NumberToken, start, text, obj_value);
    return token;
  }

  if (m_current == '+') {
    Next(); // Döngüyü kontrol etmek için Next() çağrısı ekleniyor
    static SyntaxToken token(SyntaxKind::PlusToken, start, "+", obj_value);
    return token;
  }
  else if (m_current == '-') {
    Next(); // Döngüyü kontrol etmek için Next() çağrısı ekleniyor
    static SyntaxToken token(SyntaxKind::MinusToken, start, "-", obj_value);
    return token;
  }
  else if (m_current == '*') {
    Next(); // Döngüyü kontrol etmek için Next() çağrısı ekleniyor
    static SyntaxToken token(SyntaxKind::StarToken, start, "*", obj_value);
    return token;
  }
  else if (m_current == '/') {
    Next(); // Döngüyü kontrol etmek için Next() çağrısı ekleniyor
    static SyntaxToken token(SyntaxKind::SlashToken, start, "/", obj_value);
    return token;
  }
  else if (m_current == '(') {
    Next(); // Döngüyü kontrol etmek için Next() çağrısı ekleniyor
    static SyntaxToken token(SyntaxKind::OpenParenthesisToken, start, "(", obj_value);
    return token;
  }
  else if (m_current == ')') {
    Next(); // Döngüyü kontrol etmek için Next() çağrısı ekleniyor
    static SyntaxToken token(SyntaxKind::CloseParenthesisToken, start, ")", obj_value);
    return token;
  }
  static SyntaxToken token(SyntaxKind::BadToken, start, m_text.substr(m_position - 1, 1), obj_value);
  return token;
}

int main() {
  while (true) {
    std::cout << "> ";
    std::string line;
    std::getline(std::cin, line);

    if (IsNullOrWhitespace(line)) {
      throw MyException::Exception(Status::ALL::WARN, "String is null or whitespace");
    }

    Lexer lexer{line};
    while (true) {
      auto token = lexer.NextToken();
      if (token.getKind() == SyntaxKind::EndOfFileToken) {
        break;
      }
      std::cout << GetSyntaxKindName(token.getKind()) << ":" << token.getText() << std::endl;
      if (token.getValue() != 0) {
        std::cout << token.getValue();
      }
    }
  }

  return 0;
}

