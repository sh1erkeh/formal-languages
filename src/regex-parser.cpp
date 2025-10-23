#include <iostream>
#include <string>

#include "nfa.hpp"

int main() {
  try {
    std::string regex;
    std::cout << "Enter regex: ";
    std::cin >> regex;

    NFA nfa(regex);

    std::cout << "Minimal:\n";
    nfa.ToMinimal();
    nfa.Print();
    
    std::cout << "Complement:\n";
    nfa.ToComplement();
    nfa.Print();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  return 0;
}
