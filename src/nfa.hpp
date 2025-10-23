#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "lexer.hpp"
#include "postfix-to-suffix.hpp"

class NFA {
  friend class NFAFactory;
  friend class NFAManualTest;

  struct NFAState {
    int id;
    bool is_final;
    std::unordered_map<char, std::vector<int>> transitions;

    explicit NFAState(int state_id, bool final = false);
  };

  static constexpr char kEpsilon = 0;

  int size_ = 0;
  int start_id_ = -1;
  int end_id_ = -1;

  std::set<char> alphabet_;
  std::vector<std::unique_ptr<NFAState>> states_;

  NFAState *GetState(int id);
  const NFAState *GetState(int id) const;

  void UnmarkState(int id);
  bool ContainsFinalState(const std::set<int> &states) const;

  NFAState *CreateState(bool is_final = false);
  void AddTransition(int from_id, char symbol, int to_id);

  std::set<int> EpsilonClosure(const std::set<int> &states) const;
  std::set<int> FindReachableInOneStep(const std::set<int> &states,
                                       char symbol) const;

public:
  NFA() = default;
  explicit NFA(int start_size);
  explicit NFA(const std::string &regex, bool is_postfix = false);

  NFA(NFA &&other) noexcept;
  NFA &operator=(NFA &&other) noexcept;

  NFA(const NFA &other);
  NFA &operator=(const NFA &other);

  ~NFA() = default;

  NFAState *GetStart() { return GetState(start_id_); }

  const NFAState *GetStart() const { return GetState(start_id_); }

  NFAState *GetEnd() { return GetState(end_id_); }

  const NFAState *GetEnd() const { return GetState(end_id_); }

  const std::vector<std::unique_ptr<NFAState>> &GetStates() const {
    return states_;
  }

  std::set<char> GetAlphabet() const { return alphabet_; }

  void Print() const;
  void ToDFA();
  void ToMinimal();
  void ToComplete();
  void ToComplement();

  NFA GetDFA() const;
  NFA GetMinimal() const;
  NFA GetComplete() const;
  NFA GetComplement() const;

  int ContainsPrefix(const std::string &str) const;
  std::string ToRegex() const;
};

class NFAFactory {
  static std::unordered_map<int, int> CopyStates(const NFA &from, NFA &into);

public:
  static NFA PostfixToNfa(const std::vector<Token> &postfix);
  static NFA CreateSymbolNfa(char symbol);
  static NFA CreateEpsilonNfa();
  static NFA ConcatNfas(const NFA &first, const NFA &second);
  static NFA UnionNfas(const NFA &first, const NFA &second);
  static NFA KleeneStarNfa(const NFA &nfa);
};

class RegexFactory {
public:
  static bool IsEmptyRegex(const std::string &r);
  static bool IsEpsilonRegex(const std::string &r);
  static std::string WrapIfNeeded(const std::string &r);
  static std::string UnionRegex(const std::string &a, const std::string &b);
  static std::string ConcatRegex(const std::string &a, const std::string &b);
  static std::string StarRegex(const std::string &r);
  static std::string StripOuterParensOnce(const std::string &s);
  static std::string SimplifyRegex(const std::string &inp);
};
