#include "lexer.hpp"

void Lexer::Tokenize(const std::string &regex) {
  tokens_.clear();

  for (auto it = regex.begin(); it != regex.end(); it++) {
    if (std::isspace(*it)) {
      continue;
    }

    if (std::isalpha(*it)) {
      tokens_.push_back(SymbolToken(*it));
    } else if (*it == '+') {
      tokens_.push_back(OperatorToken(TokenType::Or));
    } else if (*it == '*') {
      tokens_.push_back(OperatorToken(TokenType::KleeneStar));
    } else if (*it == '.') {
      tokens_.push_back(OperatorToken(TokenType::Concat));
    } else if (*it == '(') {
      tokens_.push_back(OperatorToken(TokenType::LBrace));
    } else if (*it == ')') {
      tokens_.push_back(OperatorToken(TokenType::RBrace));
    } else if (*it == '1') {
      tokens_.push_back(OneToken());
    } else {
      throw std::runtime_error(std::string("Unexpected character: ") + *it);
    }
  }
}

std::vector<Token> Lexer::GetTokens() { return tokens_; }

bool Lexer::DoesNeedConcat(TokenType first, TokenType second) {
  return (first == TokenType::Symbol || first == TokenType::RBrace ||
          first == TokenType::KleeneStar || first == TokenType::One) &&
         (second == TokenType::Symbol || second == TokenType::LBrace ||
          second == TokenType::One);
}

void Lexer::AddConcatenationOperators() {
  std::vector<Token> temp;

  for (size_t i = 0; i < tokens_.size(); i++) {
    temp.push_back(tokens_[i]);

    if (i < tokens_.size() - 1) {
      TokenType current_type = GetTokenType(tokens_[i]);
      TokenType next_type = GetTokenType(tokens_[i + 1]);

      if (DoesNeedConcat(current_type, next_type)) {
        temp.push_back(OperatorToken(TokenType::Concat));
      }
    }
  }

  tokens_ = std::move(temp);
}
