#include "nfa.hpp"
#include <algorithm>

// void PrintTokens(const std::vector<Token>& tokens) {
//     for (const auto& token : tokens) {
//         std::cout << TokenToString(token) << " ";
//     }
//     std::cout << std::endl;
// }

NFA NFAFactory::CreateSymbolNfa(char symbol) {
  NFA nfa;

  auto start_state = nfa.CreateState(false);
  auto end_state = nfa.CreateState(true);

  nfa.start_id_ = start_state->id;
  nfa.end_id_ = end_state->id;
  nfa.AddTransition(start_state->id, symbol, end_state->id);

  return nfa;
}

std::unordered_map<int, int> NFAFactory::CopyStates(const NFA &from,
                                                    NFA &into) {
  std::unordered_map<int, int> state_id_map;

  for (const auto &state : from.states_) {
    int new_id = into.CreateState(state->is_final)->id;
    state_id_map[state->id] = new_id;

    if (state->id == from.start_id_) {
      into.start_id_ = new_id;
    }
    if (state->id == from.end_id_) {
      into.end_id_ = new_id;
    }
  }

  for (const auto &state : from.states_) {
    int new_from_id = state_id_map[state->id];

    for (const auto &trans : state->transitions) {
      for (int old_to_id : trans.second) {
        int new_to_id = state_id_map[old_to_id];
        into.AddTransition(new_from_id, trans.first, new_to_id);
      }
    }
  }
  into.alphabet_.insert(from.alphabet_.begin(), from.alphabet_.end());

  return state_id_map;
}

NFA NFAFactory::CreateEpsilonNfa() {
  NFA nfa;

  auto start_state = nfa.CreateState(false);
  auto end_state = nfa.CreateState(true);

  nfa.start_id_ = start_state->id;
  nfa.end_id_ = end_state->id;
  nfa.AddTransition(start_state->id, NFA::kEpsilon, end_state->id);

  return nfa;
}

NFA NFAFactory::ConcatNfas(const NFA &first, const NFA &second) {
  NFA result;

  auto first_map = CopyStates(first, result);
  auto second_map = CopyStates(second, result);

  result.start_id_ = first_map[first.start_id_];
  result.end_id_ = second_map[second.end_id_];

  result.AddTransition(first_map[first.end_id_], NFA::kEpsilon,
                       second_map[second.start_id_]);

  result.UnmarkState(first_map[first.end_id_]);

  return result;
}

NFA NFAFactory::UnionNfas(const NFA &first, const NFA &second) {
  NFA result;

  auto new_start = result.CreateState(false);
  auto new_end = result.CreateState(true);

  auto first_map = CopyStates(first, result);
  auto second_map = CopyStates(second, result);

  result.start_id_ = new_start->id;
  result.end_id_ = new_end->id;

  result.AddTransition(new_start->id, NFA::kEpsilon,
                       first_map[first.start_id_]);
  result.AddTransition(new_start->id, NFA::kEpsilon,
                       second_map[second.start_id_]);

  result.AddTransition(first_map[first.end_id_], NFA::kEpsilon, new_end->id);
  result.AddTransition(second_map[second.end_id_], NFA::kEpsilon, new_end->id);

  result.UnmarkState(first_map[first.end_id_]);
  result.UnmarkState(second_map[second.end_id_]);

  return result;
}

NFA NFAFactory::KleeneStarNfa(const NFA &nfa) {
  NFA result;

  auto new_start = result.CreateState(false);
  auto new_end = result.CreateState(true);

  auto nfa_map = CopyStates(nfa, result);

  result.start_id_ = new_start->id;
  result.end_id_ = new_end->id;

  result.AddTransition(new_start->id, NFA::kEpsilon, nfa_map[nfa.start_id_]);
  result.AddTransition(new_start->id, NFA::kEpsilon, new_end->id);

  result.AddTransition(nfa_map[nfa.end_id_], NFA::kEpsilon,
                       nfa_map[nfa.start_id_]);
  result.AddTransition(nfa_map[nfa.end_id_], NFA::kEpsilon, new_end->id);

  result.UnmarkState(nfa_map[nfa.end_id_]);

  return result;
}

NFA NFAFactory::PostfixToNfa(const std::vector<Token> &postfix) {
  std::stack<NFA> nfa_stack;

  for (const auto &token : postfix) {
    switch (GetTokenType(token)) {
    case TokenType::Symbol: {
      auto symbol_token = GetIf<SymbolToken>(token);
      if (symbol_token) {
        nfa_stack.push(CreateSymbolNfa(symbol_token->symbol));
      }
      break;
    }
    case TokenType::One: {
      nfa_stack.push(CreateEpsilonNfa());
      break;
    }
    case TokenType::Concat: {
      if (nfa_stack.size() < 2) {
        throw std::runtime_error("Insufficient operands for concatenation");
      }
      NFA right = std::move(nfa_stack.top());
      nfa_stack.pop();
      NFA left = std::move(nfa_stack.top());
      nfa_stack.pop();
      nfa_stack.push(ConcatNfas(left, right));
      break;
    }
    case TokenType::Or: {
      if (nfa_stack.size() < 2) {
        throw std::runtime_error("Insufficient operands for union");
      }
      NFA right = std::move(nfa_stack.top());
      nfa_stack.pop();
      NFA left = std::move(nfa_stack.top());
      nfa_stack.pop();
      nfa_stack.push(UnionNfas(left, right));
      break;
    }
    case TokenType::KleeneStar: {
      if (nfa_stack.empty()) {
        throw std::runtime_error("Insufficient operand for Kleene star");
      }
      NFA nfa = std::move(nfa_stack.top());
      nfa_stack.pop();
      nfa_stack.push(KleeneStarNfa(nfa));
      break;
    }
    default:
      throw std::runtime_error("Unexpected token in postfix expression");
    }
  }

  if (nfa_stack.size() != 1) {
    throw std::runtime_error("Invalid regex expression: stack has " +
                             std::to_string(nfa_stack.size()) + " elements");
  }

  return std::move(nfa_stack.top());
}

NFA::NFAState::NFAState(int state_id, bool final)
    : id(state_id), is_final(final) {}

NFA::NFAState *NFA::GetState(int id) {
  for (auto &state : states_) {
    if (state->id == id) {
      return state.get();
    }
  }
  return nullptr;
}

const NFA::NFAState *NFA::GetState(int id) const {
  for (auto &state : states_) {
    if (state->id == id) {
      return state.get();
    }
  }
  return nullptr;
}

NFA::NFAState *NFA::CreateState(bool is_final) {
  auto state = std::make_unique<NFAState>(size_++, is_final);
  NFAState *ptr = state.get();
  states_.push_back(std::move(state));
  return ptr;
}

void NFA::UnmarkState(int id) {
  for (auto &state : states_) {
    if (state->id == id) {
      state->is_final = false;
      break;
    }
  }
}

void NFA::AddTransition(int from_id, char symbol, int to_id) {
  NFAState *from_state = GetState(from_id);
  if (from_state) {
    from_state->transitions[symbol].push_back(to_id);
    if (symbol != kEpsilon) {
      alphabet_.insert(symbol);
    }
  }
}

std::set<int> NFA::EpsilonClosure(const std::set<int> &states) const {
  std::set<int> closure = states;
  std::queue<int> to_process;

  for (int state_id : states) {
    to_process.push(state_id);
  }

  while (!to_process.empty()) {
    int current_id = to_process.front();
    to_process.pop();

    const NFAState *current_state = GetState(current_id);
    if (!current_state) {
      continue;
    }

    auto epsilon_trans = current_state->transitions.find(kEpsilon);
    if (epsilon_trans != current_state->transitions.end()) {
      for (int next_id : epsilon_trans->second) {
        if (closure.find(next_id) == closure.end()) {
          closure.insert(next_id);
          to_process.push(next_id);
        }
      }
    }
  }

  return closure;
}

std::set<int> NFA::FindReachableInOneStep(const std::set<int> &states,
                                          char symbol) const {
  std::set<int> result;

  for (int state_id : states) {
    const NFAState *state = GetState(state_id);
    if (!state) {
      continue;
    }

    auto transitions = state->transitions.find(symbol);
    if (transitions != state->transitions.end()) {
      for (int next_id : transitions->second) {
        result.insert(next_id);
      }
    }
  }

  return result;
}

bool NFA::ContainsFinalState(const std::set<int> &states) const {
  for (int state_id : states) {
    const NFAState *state = GetState(state_id);
    if (state && state->is_final) {
      return true;
    }
  }
  return false;
}

NFA::NFA(int start_size) : size_(start_size) {}

NFA::NFA(const std::string &regex, bool is_postfix) {
  Lexer lexer;
  lexer.Tokenize(regex);

  if (!is_postfix) {
    lexer.AddConcatenationOperators();
    InfixToPostfixConverter converter;
    auto postfix = converter.Convert(lexer.GetTokens());
    *this = NFAFactory::PostfixToNfa(postfix);
  } else {
    *this = NFAFactory::PostfixToNfa(lexer.GetTokens());
  }
}

NFA::NFA(NFA &&other) noexcept
    : size_(other.size_), start_id_(other.start_id_), end_id_(other.end_id_),
      alphabet_(std::move(other.alphabet_)), states_(std::move(other.states_)) {
  other.size_ = 0;
  other.start_id_ = -1;
  other.end_id_ = -1;
}

NFA::NFA(const NFA &other)
    : size_(other.size_), start_id_(other.start_id_), end_id_(other.end_id_),
      alphabet_(other.alphabet_) {
  for (const auto &state : other.states_) {
    auto new_state = std::make_unique<NFAState>(state->id, state->is_final);
    new_state->transitions = state->transitions;
    states_.push_back(std::move(new_state));
  }
}

NFA &NFA::operator=(NFA &&other) noexcept {
  if (this != &other) {
    states_.clear();
    size_ = other.size_;
    start_id_ = other.start_id_;
    end_id_ = other.end_id_;
    states_ = std::move(other.states_);
    alphabet_ = std::move(other.alphabet_);
    other.size_ = 0;
    other.start_id_ = -1;
    other.end_id_ = -1;
  }
  return *this;
}

NFA &NFA::operator=(const NFA &other) {
  if (this != &other) {
    states_.clear();
    size_ = other.size_;
    start_id_ = other.start_id_;
    end_id_ = other.end_id_;
    alphabet_ = other.alphabet_;

    for (const auto &state : other.states_) {
      auto new_state = std::make_unique<NFAState>(state->id, state->is_final);
      new_state->transitions = state->transitions;
      states_.push_back(std::move(new_state));
    }
  }
  return *this;
}

void NFA::Print() const {
  std::cout << "Number of states: " << states_.size() << "\n";

  for (const auto &state : states_) {
    std::cout << "Id " << state->id;

    if (state->is_final) {
      std::cout << " (f)";
    }
    if (state->id == start_id_) {
      std::cout << " (s)";
    }
    std::cout << ":\n";

    if (state->transitions.empty()) {
      continue;
    }

    for (const auto &trans : state->transitions) {
      if (trans.first == kEpsilon) {
        std::cout << "  eps -> ";
      } else {
        std::cout << "  '" << trans.first << "' -> ";
      }

      for (int target_id : trans.second) {
        std::cout << target_id << " ";
      }
      std::cout << "\n";
    }
  }
}

NFA NFA::GetDFA() const {
  NFA result(*this);
  result.ToDFA();
  return result;
}

NFA NFA::GetMinimal() const {
  NFA result(*this);
  result.ToMinimal();
  return result;
}

NFA NFA::GetComplete() const {
  NFA result(*this);
  result.ToComplete();
  return result;
}

NFA NFA::GetComplement() const {
  NFA result(*this);
  result.ToComplement();
  return result;
}

void NFA::ToDFA() {
  std::set<char> alphabet = GetAlphabet();
  alphabet.erase(kEpsilon);

  std::map<std::set<int>, int> state_mapping;
  std::queue<std::set<int>> unprocessed_states;

  NFA dfa;

  std::set<int> start_closure = EpsilonClosure({start_id_});
  if (start_closure.empty()) {
    int start_state_id = dfa.CreateState(false)->id;
    dfa.start_id_ = start_state_id;
    *this = std::move(dfa);
    return;
  }

  state_mapping[start_closure] = 0;
  int start_state_id = dfa.CreateState()->id;
  dfa.GetState(start_state_id)->is_final = ContainsFinalState(start_closure);
  dfa.start_id_ = start_state_id;
  unprocessed_states.push(start_closure);

  while (!unprocessed_states.empty()) {
    std::set<int> current_set = unprocessed_states.front();
    unprocessed_states.pop();

    int current_dfa_state_id = state_mapping[current_set];

    for (char symbol : alphabet) {
      std::set<int> next_set =
          EpsilonClosure(FindReachableInOneStep(current_set, symbol));

      if (next_set.empty()) {
        continue;
      }

      if (state_mapping.find(next_set) == state_mapping.end()) {
        int new_state_id = dfa.CreateState()->id;
        dfa.GetState(new_state_id)->is_final = ContainsFinalState(next_set);
        state_mapping[next_set] = new_state_id;
        unprocessed_states.push(next_set);
      }

      int next_dfa_state_id = state_mapping[next_set];
      dfa.AddTransition(current_dfa_state_id, symbol, next_dfa_state_id);
    }
  }

  dfa.end_id_ = -1;
  for (const auto &state : dfa.states_) {
    if (state->is_final) {
      dfa.end_id_ = state->id;
      break;
    }
  }

  if (dfa.end_id_ == -1 && !dfa.states_.empty()) {
    dfa.end_id_ = dfa.start_id_;
  }

  *this = std::move(dfa);
}

void NFA::ToMinimal() {
  ToDFA();

  if (states_.size() <= 1) {
    return;
  }

  std::set<char> alphabet = GetAlphabet();
  alphabet.erase(kEpsilon);

  std::vector<std::set<int>> partitions;

  std::set<int> final_states;
  std::set<int> non_final_states;

  for (const auto &state : states_) {
    if (state->is_final) {
      final_states.insert(state->id);
    } else {
      non_final_states.insert(state->id);
    }
  }

  if (!final_states.empty()) {
    partitions.push_back(final_states);
  }
  if (!non_final_states.empty()) {
    partitions.push_back(non_final_states);
  }

  bool changed = true;
  while (changed) {
    changed = false;
    std::vector<std::set<int>> new_partitions;

    for (const auto &partition : partitions) {
      if (partition.size() <= 1) {
        new_partitions.push_back(partition);
        continue;
      }

      std::map<std::vector<int>, std::set<int>> behavior_groups;

      for (int state_id : partition) {
        const NFAState *current_state = GetState(state_id);
        if (!current_state) {
          continue;
        }

        std::vector<int> signature;
        for (char symbol : alphabet) {
          int target_partition = -1;
          auto trans = current_state->transitions.find(symbol);
          if (trans != current_state->transitions.end() &&
              !trans->second.empty()) {
            int target_id = trans->second[0];
            for (size_t i = 0; i < partitions.size(); ++i) {
              if (partitions[i].find(target_id) != partitions[i].end()) {
                target_partition = static_cast<int>(i);
                break;
              }
            }
          }
          signature.push_back(target_partition);
        }

        behavior_groups[signature].insert(state_id);
      }

      for (const auto &group : behavior_groups) {
        new_partitions.push_back(group.second);
      }

      if (behavior_groups.size() > 1) {
        changed = true;
      }
    }

    if (partitions.size() != new_partitions.size()) {
      changed = true;
    }
    partitions = new_partitions;
  }

  NFA minimized_dfa;
  std::map<int, int> partition_representative;
  std::map<int, int> new_state_ids;

  for (const auto &partition : partitions) {
    int representative_id = *partition.begin();

    bool is_final = false;
    for (int state_id : partition) {
      const NFAState *state = GetState(state_id);
      if (state && state->is_final) {
        is_final = true;
        break;
      }
    }

    int new_state_id = minimized_dfa.CreateState(is_final)->id;
    new_state_ids[representative_id] = new_state_id;

    for (int state_id : partition) {
      partition_representative[state_id] = representative_id;
    }

    if (partition.find(start_id_) != partition.end()) {
      minimized_dfa.start_id_ = new_state_id;
    }
  }

  for (const auto &partition : partitions) {
    int representative_id = *partition.begin();
    int new_state_id = new_state_ids[representative_id];

    const NFAState *rep_state = GetState(representative_id);
    if (rep_state) {
      for (char symbol : alphabet) {
        auto trans = rep_state->transitions.find(symbol);
        if (trans != rep_state->transitions.end() && !trans->second.empty()) {
          int target_old_id = trans->second[0];
          int target_rep_id = partition_representative[target_old_id];
          int target_new_id = new_state_ids[target_rep_id];
          minimized_dfa.AddTransition(new_state_id, symbol, target_new_id);
        }
      }
    }
  }

  minimized_dfa.end_id_ = -1;
  for (const auto &state : minimized_dfa.states_) {
    if (state->is_final) {
      minimized_dfa.end_id_ = state->id;
      break;
    }
  }

  *this = std::move(minimized_dfa);
}

void NFA::ToComplete() {
  ToDFA();

  std::set<char> alphabet = GetAlphabet();
  alphabet.erase(kEpsilon);

  bool needs_sink = false;
  for (const auto &state : states_) {
    for (char symbol : alphabet) {
      auto trans = state->transitions.find(symbol);
      if (trans == state->transitions.end() || trans->second.empty()) {
        needs_sink = true;
        break;
      }
    }
    if (needs_sink) {
      break;
    }
  }

  if (!needs_sink) {
    return;
  }

  int sink_id = CreateState(false)->id;
  for (char symbol : alphabet) {
    AddTransition(sink_id, symbol, sink_id);
  }

  for (const auto &state : states_) {
    if (state->id == sink_id) {
      continue;
    }

    for (char symbol : alphabet) {
      auto trans = state->transitions.find(symbol);
      if (trans == state->transitions.end() || trans->second.empty()) {
        AddTransition(state->id, symbol, sink_id);
      }
    }
  }
}

void NFA::ToComplement() {
  ToComplete();

  for (auto &state : states_) {
    state->is_final = !state->is_final;
  }

  end_id_ = -1;
  for (auto &state : states_) {
    if (state->is_final) {
      end_id_ = state->id;
      break;
    }
  }
}

bool RegexFactory::IsEmptyRegex(const std::string &r) { return r == "epsilon"; }

bool RegexFactory::IsEpsilonRegex(const std::string &r) { return r == "1"; }

std::string RegexFactory::WrapIfNeeded(const std::string &r) {
  if (r.empty())
    return "1";
  if (r == "1" || r == "epsilon")
    return r;
  if (r.size() == 1 && std::isalnum((unsigned char)r[0]))
    return r;
  if (r.front() == '(' && r.back() == ')') {
    int bal = 0;
    for (size_t i = 0; i < r.size(); ++i) {
      if (r[i] == '(')
        ++bal;
      else if (r[i] == ')')
        --bal;
      if (bal == 0 && i + 1 < r.size()) {
        return "(" + r + ")";
      }
    }
    return r;
  }
  return "(" + r + ")";
}

std::string RegexFactory::UnionRegex(const std::string &a,
                                     const std::string &b) {
  if (a == b)
    return a;
  if (IsEmptyRegex(a) && IsEmptyRegex(b))
    return std::string("epsilon");
  if (IsEmptyRegex(a))
    return b;
  if (IsEmptyRegex(b))
    return a;
  if (IsEpsilonRegex(a) && IsEpsilonRegex(b))
    return std::string("1");
  if (IsEpsilonRegex(a)) {
    if (b == "epsilon")
      return "1";
    return "(" + std::string("1") + "+" + b + ")";
  }
  if (IsEpsilonRegex(b)) {
    if (a == "epsilon")
      return "1";
    return "(" + a + "+" + std::string("1") + ")";
  }
  return "(" + a + "+" + b + ")";
}

std::string RegexFactory::ConcatRegex(const std::string &a,
                                      const std::string &b) {
  if (IsEmptyRegex(a) || IsEmptyRegex(b))
    return std::string("epsilon");
  if (IsEpsilonRegex(a))
    return b;
  if (IsEpsilonRegex(b))
    return a;
  return WrapIfNeeded(a) + WrapIfNeeded(b);
}

std::string RegexFactory::StarRegex(const std::string &r) {
  if (IsEmptyRegex(r))
    return std::string("1");
  if (IsEpsilonRegex(r))
    return std::string("1");
  if (!r.empty() && r.back() == '*')
    return r;
  if (r.size() == 1 && std::isalnum((unsigned char)r[0]))
    return r + "*";
  return WrapIfNeeded(r) + "*";
}

std::string RegexFactory::StripOuterParensOnce(const std::string &s) {
  if (s.size() >= 2 && s.front() == '(' && s.back() == ')') {
    int bal = 0;
    for (size_t i = 0; i < s.size(); ++i) {
      if (s[i] == '(')
        ++bal;
      else if (s[i] == ')')
        --bal;
      if (bal == 0 && i + 1 < s.size())
        return s;
    }
    return s.substr(1, s.size() - 2);
  }
  return s;
}

std::string RegexFactory::SimplifyRegex(const std::string &inp) {
  std::string cur = inp;
  for (int pass = 0; pass < 6; ++pass) {
    std::string prev = cur;

    if (cur == "(epsilon)")
      cur = "epsilon";
    if (cur == "(1)")
      cur = "1";
    cur = StripOuterParensOnce(cur);

    {
      std::string out;
      for (size_t i = 0; i < cur.size();) {
        if (cur.compare(i, 2, "1*") == 0) {
          out += "1";
          i += 2;
          continue;
        }
        if (cur.compare(i, 8, "epsilon*") == 0) {
          out += "1";
          i += 8;
          continue;
        }
        if (cur.compare(i, 4, "(1)*") == 0) {
          out += "1";
          i += 4;
          continue;
        }
        if (cur.compare(i, 10, "(epsilon)*") == 0) {
          out += "1";
          i += 10;
          continue;
        }
        out.push_back(cur[i++]);
      }
      cur.swap(out);
    }

    {
      std::string out;
      for (size_t i = 0; i < cur.size();) {
        if (cur.compare(i, 2, "1(") == 0) {
          i += 1;
          continue;
        }
        if (cur.compare(i, 1, "1") == 0) {
          if (i + 1 < cur.size() &&
              (std::isalnum((unsigned char)cur[i + 1]) || cur[i + 1] == '(')) {
            ++i;
            continue;
          }
        }
        if (i + 1 < cur.size() && cur[i + 1] == '1') {
          if (std::isalnum((unsigned char)cur[i]) || cur[i] == ')') {
            out.push_back(cur[i]);
            i += 2;
            continue;
          }
        }
        out.push_back(cur[i++]);
      }
      cur.swap(out);
    }

    if (cur == prev)
      break;
  }
  if (cur == "(1)")
    cur = "1";
  if (cur == "(epsilon)")
    cur = "epsilon";
  return cur;
}

std::string NFA::ToRegex() const {
  if (states_.empty())
    return "epsilon";

  std::vector<int> ids;
  ids.reserve(states_.size());
  std::unordered_map<int, int> id_to_index;
  for (const auto &s : states_) {
    id_to_index[s->id] = static_cast<int>(ids.size());
    ids.push_back(s->id);
  }
  int n = static_cast<int>(ids.size());
  if (n == 0)
    return "epsilon";

  std::vector<std::vector<std::string>> R(
      n, std::vector<std::string>(n, "epsilon"));

  for (const auto &s : states_) {
    int i = id_to_index.at(s->id);
    for (const auto &tr : s->transitions) {
      char sym = tr.first;
      std::string label =
          (sym == kEpsilon ? std::string("1") : std::string(1, sym));
      for (int to_id : tr.second) {
        auto it = id_to_index.find(to_id);
        if (it == id_to_index.end())
          continue;
        int j = it->second;
        R[i][j] = RegexFactory::UnionRegex(R[i][j], label);
      }
    }
  }

  int newStart = n;
  int newAccept = n + 1;
  int N = n + 2;
  std::vector<std::vector<std::string>> G(
      N, std::vector<std::string>(N, "epsilon"));

  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
      G[i][j] = R[i][j];

  auto itStart = id_to_index.find(start_id_);
  if (itStart == id_to_index.end())
    return "epsilon";
  G[newStart][itStart->second] =
      RegexFactory::UnionRegex(G[newStart][itStart->second], std::string("1"));

  for (const auto &s : states_) {
    if (s->is_final) {
      int i = id_to_index.at(s->id);
      G[i][newAccept] =
          RegexFactory::UnionRegex(G[i][newAccept], std::string("1"));
    }
  }

  for (int k = 0; k < N; ++k) {
    if (k == newStart || k == newAccept)
      continue;

    std::string Rkk = G[k][k];

    for (int i = 0; i < N; ++i) {
      if (i == k)
        continue;
      if (RegexFactory::IsEmptyRegex(G[i][k]))
        continue;
      for (int j = 0; j < N; ++j) {
        if (j == k)
          continue;
        if (RegexFactory::IsEmptyRegex(G[k][j]))
          continue;

        std::string via = RegexFactory::ConcatRegex(
            G[i][k],
            RegexFactory::ConcatRegex(RegexFactory::StarRegex(Rkk), G[k][j]));
        G[i][j] = RegexFactory::UnionRegex(G[i][j], via);
      }
    }

    for (int i = 0; i < N; ++i)
      G[i][k] = "epsilon";
    for (int j = 0; j < N; ++j)
      G[k][j] = "epsilon";
  }

  std::string result = G[newStart][newAccept];
  if (result.empty())
    result = "epsilon";

  result = RegexFactory::SimplifyRegex(result);

  return result;
}

int NFA::ContainsPrefix(const std::string &str) const {
  std::set<int> current = EpsilonClosure({start_id_});
  int longest_match = -1;

  if (ContainsFinalState(current)) {
    longest_match = 0;
  }

  for (size_t i = 0; i < str.length(); i++) {
    char c = str[i];
    std::set<int> next = EpsilonClosure(FindReachableInOneStep(current, c));

    if (next.empty()) {
      break;
    }

    current = next;

    if (ContainsFinalState(current)) {
      longest_match = i + 1;
    }
  }

  return longest_match;
}
