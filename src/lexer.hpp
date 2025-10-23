#pragma once

#include <cctype>
#include <iostream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

enum class TokenType : char {
  Symbol,
  Concat = '.',
  Or = '+',
  KleeneStar = '*',
  LBrace = '(',
  RBrace = ')',
  One = '1'
};

struct SymbolToken {
  char symbol;

  explicit SymbolToken(char s) : symbol(s) {}

  std::string ToString() const { return std::string(1, symbol); }

  TokenType GetType() const { return TokenType::Symbol; }
};

struct OneToken {
  std::string ToString() const { return "1"; }

  TokenType GetType() const { return TokenType::One; }
};

struct OperatorToken {
  TokenType operation_type;

  explicit OperatorToken(TokenType type) : operation_type(type) {}

  std::string ToString() const {
    return std::string(1, static_cast<char>(operation_type));
  }

  TokenType GetType() const { return operation_type; }
};

using Token = std::variant<SymbolToken, OneToken, OperatorToken>;

inline TokenType GetTokenType(const Token &token) {
  return std::visit([](const auto &t) { return t.GetType(); }, token);
}

inline std::string TokenToString(const Token &token) {
  return std::visit([](const auto &t) { return t.ToString(); }, token);
}

template <typename T> inline const T *GetIf(const Token &token) {
  return std::get_if<T>(&token);
}

class Lexer {
  std::vector<Token> tokens_;

  static bool DoesNeedConcat(TokenType first, TokenType second);

public:
  void AddConcatenationOperators();
  void Tokenize(const std::string &regex);
  std::vector<Token> GetTokens();
};
