#ifndef LAB1_MEALYAUTOMAT_H
#define LAB1_MEALYAUTOMAT_H

#include "IAutomata.h"
#include <iostream>
#include <vector>
#include <queue>
#include <sstream>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <fstream>
#include <stdexcept>

class MealyAutomata final : public IAutomata
{
public:
    MealyAutomata() = default;

    MealyAutomata(std::vector<std::string> states, std::list<std::pair<std::string, std::vector<Transition>>> table)
            : m_states(std::move(states)),
              m_transition(std::move(table)) {}

    void ReadFromFile(const std::string &filename) override
    {
        m_states.clear();
        m_transition.clear();

        std::ifstream input(filename);
        if (!input.is_open())
        {
            throw std::runtime_error("Could not open the file.");
        }

        ReadStatesFromFile(input);
        ReadTransitionsFromFile(input);

        input.close();

        RemoveUnreachableStates();
    }

    void Minimize() override
    {

    }


    void PrintToFile(const std::string &filename) const override
    {
        std::ofstream output(filename);
        if (!output.is_open())
        {
            throw std::invalid_argument("Could not open file " + filename + " for writing.");
        }

        for (const auto& state: m_states)
        {
            output << ';' << state;
        }
        output << std::endl;

        for (const auto &[inputSymbol, transitions] : m_transition)
        {
            output << inputSymbol;
            for (const auto &transition : transitions)
            {
                output << ";" << transition.nextState << "/" << transition.outputSymbol;
            }
            output << std::endl;
        }
    }

    [[nodiscard]] std::list<std::pair<std::string, std::vector<Transition>>> GetTransition() const
    {
        return m_transition;
    }

    [[nodiscard]] std::vector<std::string> GetStates() const
    {
        return m_states;
    }

    [[nodiscard]] std::vector<std::string> GetInputSymbols() const
    {
        std::vector<std::string> inputSymbols;
        for (const auto &it : m_transition)
        {
            inputSymbols.emplace_back(it.first);
        }
        return inputSymbols;
    }

private:
    std::vector<std::string> m_states;
    std::list<std::pair<std::string, std::vector<Transition>>> m_transition;

    void ReadStatesFromFile(std::ifstream &inputFile)
    {
        std::string line;
        if (!std::getline(inputFile, line) || line.empty())
        {
            throw std::runtime_error("Invalid or empty input file.");
        }

        std::stringstream ss(line);
        std::string state;

        // Skip the initial empty cell
        if (!std::getline(ss, state, ';'))
        {
            throw std::runtime_error("Failed to parse the states line.");
        }

        while (std::getline(ss, state, ';'))
        {
            if (!state.empty())
            {
                m_states.push_back(state);
            }
        }

        if (m_states.empty())
        {
            throw std::runtime_error("No valid states found in the input file.");
        }
    }

    void ReadTransitionsFromFile(std::ifstream &inputFile)
    {
        std::string line;
        while (std::getline(inputFile, line))
        {
            std::stringstream ss(line);
            std::string inputSymbol;

            // Read the input symbol
            if (!std::getline(ss, inputSymbol, ';') || inputSymbol.empty())
            {
                throw std::runtime_error("Invalid transition line format.");
            }

            std::vector<Transition> transitions;

            for (const auto &state : m_states)
            {
                std::string transitionData;
                if (std::getline(ss, transitionData, ';'))
                {
                    size_t separatorPos = transitionData.find('/');
                    if (separatorPos != std::string::npos)
                    {
                        std::string nextState = transitionData.substr(0, separatorPos);
                        std::string output = transitionData.substr(separatorPos + 1);

                        transitions.emplace_back(nextState, output);
                    }
                }
            }

            m_transition.emplace_back(inputSymbol, transitions);
        }
    }

    void RemoveUnreachableStates()
    {
        std::unordered_set<std::string> reachableStates;
        std::queue<std::string> queue;

        if (!m_states.empty())
        {
            reachableStates.insert(m_states[0]);
            queue.push(m_states[0]);
        }

        while (!queue.empty())
        {
            std::string currentState = queue.front();
            queue.pop();

            for (const auto &[inputSymbol, transitions] : m_transition)
            {
                for (const auto &transition : transitions)
                {
                    if (reachableStates.find(transition.nextState) == reachableStates.end())
                    {
                        reachableStates.insert(transition.nextState);
                        queue.push(transition.nextState);
                    }
                }
            }
        }

        m_states.erase(std::remove_if(m_states.begin(), m_states.end(),
                                      [&reachableStates](const std::string &state)
                                      {
                                          return reachableStates.find(state) == reachableStates.end();
                                      }),
                       m_states.end());

        for (auto it = m_transition.begin(); it != m_transition.end();)
        {
            auto &transitions = it->second;
            transitions.erase(std::remove_if(transitions.begin(), transitions.end(),
                                             [&reachableStates](const Transition &transition)
                                             {
                                                 return reachableStates.find(transition.nextState) == reachableStates.end();
                                             }),
                              transitions.end());

            if (transitions.empty())
            {
                it = m_transition.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
};

#endif //LAB1_MEALYAUTOMAT_H
