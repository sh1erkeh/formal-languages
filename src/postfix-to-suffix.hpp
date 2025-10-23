#pragma once

#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

#include "lexer.hpp"

class InfixToPostfixConverter {
public:
  std::vector<Token> Convert(const std::vector<Token> &tokens);

private:
  int Precedence(TokenType op);
};
