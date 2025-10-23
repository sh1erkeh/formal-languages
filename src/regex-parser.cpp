#include <iostream>
#include <string>

#include "nfa.hpp"

int main() {
  try {
    std::string regex;
    std::cout << "Enter regex: ";
    std::cin >> regex;

    NFA nfa(regex, true);

    std::cout << nfa.ContainsPrefix("abacb") << '\n';
    std::cout << nfa.ContainsPrefix("cb") << '\n';
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  return 0;
}
