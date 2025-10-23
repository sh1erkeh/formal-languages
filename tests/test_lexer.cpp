#include "../src/lexer.hpp"
#include <gtest/gtest.h>

TEST(LexerTest, BasicSymbols) {
  Lexer lexer;
  lexer.Tokenize("a.b.c");
  auto tokens = lexer.GetTokens();

  ASSERT_EQ(tokens.size(), 5);
  EXPECT_EQ(GetTokenType(tokens[0]), TokenType::Symbol);
  EXPECT_EQ(GetTokenType(tokens[1]), TokenType::Concat);
  EXPECT_EQ(GetTokenType(tokens[2]), TokenType::Symbol);
  EXPECT_EQ(GetTokenType(tokens[3]), TokenType::Concat);
  EXPECT_EQ(GetTokenType(tokens[4]), TokenType::Symbol);
}

TEST(LexerTest, Operators) {
  Lexer lexer;
  lexer.Tokenize("(a+b+c)*");
  auto tokens = lexer.GetTokens();

  ASSERT_EQ(tokens.size(), 8);

  EXPECT_EQ(GetTokenType(tokens[0]), TokenType::LBrace);
  EXPECT_EQ(GetTokenType(tokens[1]), TokenType::Symbol);
  EXPECT_EQ(GetTokenType(tokens[2]), TokenType::Or);
  EXPECT_EQ(GetTokenType(tokens[3]), TokenType::Symbol);
  EXPECT_EQ(GetTokenType(tokens[4]), TokenType::Or);
  EXPECT_EQ(GetTokenType(tokens[5]), TokenType::Symbol);
  EXPECT_EQ(GetTokenType(tokens[6]), TokenType::RBrace);
  EXPECT_EQ(GetTokenType(tokens[7]), TokenType::KleeneStar);
}

TEST(LexerTest, Parentheses) {
  Lexer lexer;
  lexer.Tokenize("(a+b+c*)*");
  auto tokens = lexer.GetTokens();

  ASSERT_EQ(tokens.size(), 9);

  EXPECT_EQ(GetTokenType(tokens[0]), TokenType::LBrace);
  EXPECT_EQ(GetTokenType(tokens[1]), TokenType::Symbol);
  EXPECT_EQ(GetTokenType(tokens[2]), TokenType::Or);
  EXPECT_EQ(GetTokenType(tokens[3]), TokenType::Symbol);
  EXPECT_EQ(GetTokenType(tokens[4]), TokenType::Or);
  EXPECT_EQ(GetTokenType(tokens[5]), TokenType::Symbol);
  EXPECT_EQ(GetTokenType(tokens[6]), TokenType::KleeneStar);
  EXPECT_EQ(GetTokenType(tokens[7]), TokenType::RBrace);
  EXPECT_EQ(GetTokenType(tokens[8]), TokenType::KleeneStar);
}

TEST(LexerTest, Concat) {
  Lexer lexer;
  lexer.Tokenize("a(b+c)");
  auto tokens = lexer.GetTokens();

  ASSERT_EQ(tokens.size(), 6);
  EXPECT_EQ(GetTokenType(tokens[0]), TokenType::Symbol);
  EXPECT_EQ(GetTokenType(tokens[1]), TokenType::LBrace);
  EXPECT_EQ(GetTokenType(tokens[2]), TokenType::Symbol);
  EXPECT_EQ(GetTokenType(tokens[3]), TokenType::Or);
  EXPECT_EQ(GetTokenType(tokens[4]), TokenType::Symbol);
  EXPECT_EQ(GetTokenType(tokens[5]), TokenType::RBrace);
}
