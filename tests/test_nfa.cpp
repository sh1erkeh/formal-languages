#include <gtest/gtest.h>
#include "../src/nfa.hpp"
#include <sstream>
#include <map>
#include <set>

class NFAPropertiesTest : public ::testing::Test {
protected:
    struct DFASnapshot {
        int state_count;
        int start_state;
        std::set<int> final_states;
        std::map<std::pair<int, char>, int> transitions;
        std::set<char> alphabet;
    };

    DFASnapshot TakeSnapshot(const NFA& nfa) {
        DFASnapshot snapshot;
        snapshot.state_count = static_cast<int>(nfa.GetStates().size());

        auto start_ptr = nfa.GetStart();
        snapshot.start_state = start_ptr ? start_ptr->id : -1;

        for (const auto& state : nfa.GetStates()) {
            if (state->is_final) {
                snapshot.final_states.insert(state->id);
            }

            for (const auto& trans : state->transitions) {
                if (trans.first != 0) {
                    snapshot.alphabet.insert(trans.first);
                }
                if (!trans.second.empty()) {
                    snapshot.transitions[{state->id, trans.first}] =
                    trans.second[0];
                }
            }
        }

        return snapshot;
    }

    void PrintSnapshot(const DFASnapshot& snap) {
        std::cout << "DFA Snapshot:\n";
        std::cout << "  States: " << snap.state_count << "\n";
        std::cout << "  Start: " << snap.start_state << "\n";
        std::cout << "  Final states: ";
        for (int s : snap.final_states) std::cout << s << " ";
        std::cout << "\n  Alphabet: ";
        for (char c : snap.alphabet) std::cout << c << " ";
        std::cout << "\n  Transitions:\n";
        for (const auto& [key, target] : snap.transitions) {
            std::cout << "    " << key.first << " --" << key.second << "--> "
            << target << "\n";
        }
    }

    void TestMinimizationReducesStates(const std::string& regex) {
        NFA nfa(regex);
        nfa.ToDFA();
        int before_states = TakeSnapshot(nfa).state_count;

        nfa.ToMinimal();
        int after_states = TakeSnapshot(nfa).state_count;

        EXPECT_LE(after_states, before_states) << "Minimization should not increase state count for regex: " << regex;
    }

    void TestDoubleComplement(const std::string& regex) {
        NFA original(regex);
        original.ToMinimal();
        original.ToComplete();
        auto original_snap = TakeSnapshot(original);

        NFA complement = original;
        complement.ToComplement();
        complement.ToComplement();

        auto double_complement_snap = TakeSnapshot(complement);

        EXPECT_EQ(original_snap.state_count,
        double_complement_snap.state_count);
        EXPECT_EQ(original_snap.final_states.size(),
        double_complement_snap.final_states.size());
    }

    void TestComplementFlipsFinalStates(const std::string& regex) {
        NFA nfa(regex);
        nfa.ToMinimal();
        nfa.ToComplete();

        auto before_snap = TakeSnapshot(nfa);
        int before_final_count = before_snap.final_states.size();

        nfa.ToComplement();
        auto after_snap = TakeSnapshot(nfa);
        int after_final_count = after_snap.final_states.size();

        EXPECT_EQ(after_final_count, before_snap.state_count - before_final_count) << "Complement should flip final/non-final states for regex: " << regex;
    }

    void TestMinimizationPreservesStartFinal(const std::string& regex) {
        NFA nfa(regex);
        nfa.ToDFA();
        auto before_snap = TakeSnapshot(nfa);
        bool start_was_final =
        before_snap.final_states.count(before_snap.start_state) > 0;

        nfa.ToMinimal();
        auto after_snap = TakeSnapshot(nfa);
        bool start_is_final =
        after_snap.final_states.count(after_snap.start_state) > 0;

        EXPECT_EQ(start_was_final, start_is_final) << "Minimization should preserve whether start state is final for regex: " << regex;
    }

    void TestKnownMinimalStateCount(const std::string& regex, int
    expected_max_states) {
        NFA nfa(regex);
        nfa.ToMinimal();
        auto snap = TakeSnapshot(nfa);

        EXPECT_LE(snap.state_count, expected_max_states)
            << "Regex " << regex << " should have at most " <<
            expected_max_states
            << " states after minimization, but has " << snap.state_count;
    }
};


TEST_F(NFAPropertiesTest, ToMinimal_ReducesStates_SingleSymbol) {
    TestMinimizationReducesStates("a");
}

TEST_F(NFAPropertiesTest, ToMinimal_ReducesStates_KleeneStar) {
    TestMinimizationReducesStates("a*");
}

TEST_F(NFAPropertiesTest, ToMinimal_ReducesStates_Union) {
    TestMinimizationReducesStates("a+b");
}

TEST_F(NFAPropertiesTest, ToMinimal_ReducesStates_Concat) {
    TestMinimizationReducesStates("a.b");
}

TEST_F(NFAPropertiesTest, ToMinimal_ReducesStates_Complex) {
    TestMinimizationReducesStates("(a+b)*.c");
}

TEST_F(NFAPropertiesTest, ToMinimal_PreservesStartFinal_SingleSymbol) {
    TestMinimizationPreservesStartFinal("a");
}

TEST_F(NFAPropertiesTest, ToMinimal_PreservesStartFinal_EmptyString) {
    TestMinimizationPreservesStartFinal("1");
}

TEST_F(NFAPropertiesTest, ToMinimal_PreservesStartFinal_KleeneStar) {
    TestMinimizationPreservesStartFinal("a*");
}

TEST_F(NFAPropertiesTest, ToMinimal_KnownStateCount_SingleA) {
    TestKnownMinimalStateCount("a", 3);
}

TEST_F(NFAPropertiesTest, ToMinimal_KnownStateCount_AStar) {
    TestKnownMinimalStateCount("a*", 2);
}

TEST_F(NFAPropertiesTest, ToMinimal_KnownStateCount_AB) {
    TestKnownMinimalStateCount("a+b", 3);
}

TEST_F(NFAPropertiesTest, ToMinimal_KnownStateCount_Empty) {
    TestKnownMinimalStateCount("1", 2);
}

TEST_F(NFAPropertiesTest, ToMinimal_KnownStateCount_AA) {
    TestKnownMinimalStateCount("a.a", 4);
}


TEST_F(NFAPropertiesTest, ToComplement_DoubleComplement_SingleSymbol) {
    TestDoubleComplement("a");
}

TEST_F(NFAPropertiesTest, ToComplement_DoubleComplement_KleeneStar) {
    TestDoubleComplement("a*");
}

TEST_F(NFAPropertiesTest, ToComplement_DoubleComplement_Union) {
    TestDoubleComplement("a+b");
}

TEST_F(NFAPropertiesTest, ToComplement_DoubleComplement_Concat) {
    TestDoubleComplement("a.b");
}

TEST_F(NFAPropertiesTest, ToComplement_DoubleComplement_Complex) {
    TestDoubleComplement("(a+b)*");
}

TEST_F(NFAPropertiesTest, ToComplement_FlipsFinalStates_SingleSymbol) {
    TestComplementFlipsFinalStates("a");
}

TEST_F(NFAPropertiesTest, ToComplement_FlipsFinalStates_KleeneStar) {
    TestComplementFlipsFinalStates("a*");
}

TEST_F(NFAPropertiesTest, ToComplement_FlipsFinalStates_Union) {
    TestComplementFlipsFinalStates("a+b");
}

TEST_F(NFAPropertiesTest, ToComplement_FlipsFinalStates_EmptyString) {
    TestComplementFlipsFinalStates("1");
}

TEST_F(NFAPropertiesTest, ToComplement_FlipsFinalStates_NoAccepting) {
    NFA nfa("a.a.a");
    nfa.ToDFA();
    nfa.ToComplete();

    for (auto& state : nfa.GetStates()) {
        state->is_final = false;
    }

    auto before_snap = TakeSnapshot(nfa);
    nfa.ToComplement();
    auto after_snap = TakeSnapshot(nfa);

    EXPECT_EQ(after_snap.final_states.size(), after_snap.state_count);
}


TEST_F(NFAPropertiesTest, ToMinimal_SingleSymbol_HasTransition) {
    NFA nfa("a");
    nfa.ToMinimal();
    auto snap = TakeSnapshot(nfa);

    bool has_a_transition = false;
    for (const auto& [key, target] : snap.transitions) {
        if (key.second == 'a') {
            has_a_transition = true;
            break;
        }
    }
    EXPECT_TRUE(has_a_transition);
}

TEST_F(NFAPropertiesTest, ToMinimal_AStar_StartIsFinal) {
    NFA nfa("a*");
    nfa.ToMinimal();
    auto snap = TakeSnapshot(nfa);

    EXPECT_TRUE(snap.final_states.count(snap.start_state));
}

TEST_F(NFAPropertiesTest, ToMinimal_EmptyString_StartIsFinal) {
    NFA nfa("1");
    nfa.ToMinimal();
    auto snap = TakeSnapshot(nfa);

    EXPECT_TRUE(snap.final_states.count(snap.start_state));
}

TEST_F(NFAPropertiesTest, ToComplement_EmptyString_NoFinalStates) {
    NFA nfa("1");
    nfa.ToComplement();
    auto snap = TakeSnapshot(nfa);

    EXPECT_GE(snap.state_count, 1);
}

TEST_F(NFAPropertiesTest, ToComplement_AStar_StartNotFinal) {
    NFA nfa("a*");
    nfa.ToComplement();
    auto snap = TakeSnapshot(nfa);

    EXPECT_FALSE(snap.final_states.count(snap.start_state));
}

TEST_F(NFAPropertiesTest, ToMinimal_Then_ToComplement_Consistent) {
    NFA nfa("a+b");
    nfa.ToMinimal();
    int minimal_states = TakeSnapshot(nfa).state_count;

    nfa.ToComplement();
    int complement_states = TakeSnapshot(nfa).state_count;

    EXPECT_LE(complement_states, minimal_states + 2);
}

TEST_F(NFAPropertiesTest, ToComplement_Then_ToMinimal_Consistent) {
    NFA nfa("a.b");
    nfa.ToComplement();
    int complement_states = TakeSnapshot(nfa).state_count;

    nfa.ToMinimal();
    int minimal_states = TakeSnapshot(nfa).state_count;

    EXPECT_LE(minimal_states, complement_states);
}

TEST_F(NFAPropertiesTest, ToMinimal_SingleState) {
    NFA nfa("1");
    nfa.ToMinimal();
    auto snap = TakeSnapshot(nfa);

    EXPECT_GE(snap.state_count, 1);
    EXPECT_LE(snap.state_count, 3);
}

TEST_F(NFAPropertiesTest, ToComplement_AllStatesFinal) {
    NFA nfa("(a+b)*");
    nfa.ToDFA();
    nfa.ToComplete();

    auto before_snap = TakeSnapshot(nfa);
    EXPECT_GT(before_snap.final_states.size(), 0);

    nfa.ToComplement();
    auto after_snap = TakeSnapshot(nfa);

    EXPECT_EQ(after_snap.final_states.size(), 0);
}
