#include "postfix-to-suffix.hpp"

std::vector<Token>
InfixToPostfixConverter::Convert(const std::vector<Token> &tokens) {
  std::vector<Token> postfix;
  std::stack<Token> op_stack;

  for (const auto &token : tokens) {
    TokenType type = GetTokenType(token);

    if (type == TokenType::Symbol || type == TokenType::One) {
      postfix.push_back(token);
    } else if (type == TokenType::LBrace) {
      op_stack.push(OperatorToken(TokenType::LBrace));
    } else if (type == TokenType::RBrace) {
      while (!op_stack.empty() &&
             GetTokenType(op_stack.top()) != TokenType::LBrace) {
        postfix.push_back(op_stack.top());
        op_stack.pop();
      }
      if (op_stack.empty()) {
        throw std::runtime_error("Mismatched parentheses");
      }
      op_stack.pop();
    } else {
      while (!op_stack.empty() &&
             Precedence(GetTokenType(op_stack.top())) >= Precedence(type)) {
        postfix.push_back(op_stack.top());
        op_stack.pop();
      }
      op_stack.push(token);
    }
  }

  while (!op_stack.empty()) {
    if (GetTokenType(op_stack.top()) == TokenType::LBrace) {
      throw std::runtime_error("Mismatched parentheses");
    }
    postfix.push_back(op_stack.top());
    op_stack.pop();
  }
  return postfix;
}

int InfixToPostfixConverter::Precedence(TokenType op) {
  switch (op) {
  case TokenType::Or:
    return 1;
  case TokenType::Concat:
    return 2;
  case TokenType::KleeneStar:
    return 3;
  default:
    return 0;
  }
}
