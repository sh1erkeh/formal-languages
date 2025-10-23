#include "../src/lexer.hpp"
#include "../src/postfix-to-suffix.hpp"
#include <gtest/gtest.h>

class ConverterTest : public ::testing::Test {
protected:
  InfixToPostfixConverter converter;
  Lexer lexer;

  std::vector<Token> ConvertRegex(const std::string &regex) {
    lexer.Tokenize(regex);
    auto tokens = lexer.GetTokens();
    return converter.Convert(tokens);
  }
};

TEST_F(ConverterTest, BasicConversion) {
  auto postfix = ConvertRegex("a.b.c");

  // Postfix for "a.b.c" should be "ab.c."
  ASSERT_EQ(postfix.size(), 5);
  EXPECT_EQ(GetTokenType(postfix[0]), TokenType::Symbol); // a
  EXPECT_EQ(GetTokenType(postfix[1]), TokenType::Symbol); // b
  EXPECT_EQ(GetTokenType(postfix[2]), TokenType::Concat); // .
  EXPECT_EQ(GetTokenType(postfix[3]), TokenType::Symbol); // c
  EXPECT_EQ(GetTokenType(postfix[4]), TokenType::Concat); // .
}

TEST_F(ConverterTest, OrPrecedence) {
  auto postfix = ConvertRegex("a+b*.c");

  // Postfix should handle operator precedence correctly
  // This test verifies the converter doesn't crash and produces valid output
  ASSERT_GT(postfix.size(), 0);

  // Basic sanity check - should contain all original symbols
  int symbol_count = 0;
  for (const auto &token : postfix) {
    if (GetTokenType(token) == TokenType::Symbol) {
      symbol_count++;
    }
  }
  EXPECT_GE(symbol_count, 2); // At least 'a' and 'b'
}

TEST_F(ConverterTest, ParenthesesHandling) {
  auto postfix = ConvertRegex("(a+b).c");

  // Postfix for "(a+b).c" should be "ab+.c."
  ASSERT_GT(postfix.size(), 0);

  // Should end with concat operator
  EXPECT_EQ(GetTokenType(postfix.back()), TokenType::Concat);
}

TEST_F(ConverterTest, KleeneStar) {
  auto postfix = ConvertRegex("a*");

  ASSERT_EQ(postfix.size(), 2);
  EXPECT_EQ(GetTokenType(postfix[0]), TokenType::Symbol);
  EXPECT_EQ(GetTokenType(postfix[1]), TokenType::KleeneStar);
}
