#ifndef LAB1_MEALYAUTOMAT_H
#define LAB1_MEALYAUTOMAT_H

#include "IAutomata.h"
#include <iostream>
#include <vector>
#include <queue>
#include <sstream>
#include <list>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <fstream>
#include <stdexcept>

class MealyAutomata final : public IAutomata
{
public:
    MealyAutomata() = default;

    void ReadFromFile(const std::string& filename) override
    {
        std::ifstream input(filename);
        if (!input.is_open())
        {
            throw std::invalid_argument("Could not open file " + filename + " for reading.");
        }

        std::string line;

        if (!std::getline(input, line))
        {
            throw std::invalid_argument("File is empty or has invalid format: " + filename);
        }

        std::stringstream headerStream(line);
        std::string cell;

        std::getline(headerStream, cell, ';');

        m_states.clear();
        while (std::getline(headerStream, cell, ';'))
        {
            m_states.push_back(cell);
        }

        m_inputSymbols.clear();
        m_transitionTable.clear();

        while (std::getline(input, line))
        {
            std::stringstream rowStream(line);

            std::string inputSymbol;
            if (!std::getline(rowStream, inputSymbol, ';'))
            {
                throw std::invalid_argument("Invalid row format in file: " + filename);
            }
            m_inputSymbols.push_back(inputSymbol);

            std::vector<Transition> transitions;
            size_t stateIndex = 0;

            while (std::getline(rowStream, cell, ';'))
            {
                size_t slashPos = cell.find('/');
                if (slashPos == std::string::npos)
                {
                    throw std::invalid_argument("Invalid transition format in file: " + filename);
                }

                std::string nextState = cell.substr(0, slashPos);
                std::string outputSymbol = cell.substr(slashPos + 1);

                if (stateIndex >= m_states.size())
                {
                    throw std::out_of_range("More transitions than states in file: " + filename);
                }

                transitions.emplace_back(nextState, outputSymbol);
                ++stateIndex;
            }

            if (transitions.size() != m_states.size())
            {
                throw std::invalid_argument("Inconsistent number of transitions for input symbol: " + inputSymbol);
            }

            m_transitionTable[inputSymbol] = transitions;
        }

        if (m_states.empty())
        {
            throw std::invalid_argument("No states found in file: " + filename);
        }
        m_initialState = m_states.front();
    }

    void Minimize() override
    {
        RemoveUnreachableStates();
        MergeEquivalentStates();
    }


    void PrintToFile(const std::string &filename) const override
    {
        std::ofstream output(filename);
        if (!output.is_open())
        {
            throw std::invalid_argument("Could not open file " + filename + " for writing.");
        }

        for (const auto& state : m_states)
        {
            output << ';' << state;
        }
        output << std::endl;

        for (const auto& [inputSymbol, transitions] : m_transitionTable)
        {
            output << inputSymbol;
            for (const auto& transition : transitions)
            {
                output << ";" << transition.nextState << "/" << transition.outputSymbol;
            }
            output << std::endl;
        }
    }
private:
    std::vector<std::string> m_states;
    std::string m_initialState;
    std::vector<std::string> m_inputSymbols;
    std::map<std::string, std::vector<Transition>> m_transitionTable;

    void RemoveUnreachableStates()
    {
        std::unordered_set<std::string> reachableStates;
        std::queue<std::string> toVisit;

        reachableStates.insert(m_initialState);
        toVisit.push(m_initialState);

        while (!toVisit.empty())
        {
            std::string currentState = toVisit.front();
            toVisit.pop();

            for (const auto& [inputSymbol, transitions] : m_transitionTable)
            {
                for (const auto& transition : transitions)
                {
                    if (reachableStates.find(transition.nextState) == reachableStates.end())
                    {
                        reachableStates.insert(transition.nextState);
                        toVisit.push(transition.nextState);
                    }
                }
            }
        }

        for (auto it = m_transitionTable.begin(); it != m_transitionTable.end();)
        {
            it->second.erase(
                    std::remove_if(it->second.begin(), it->second.end(),
                                   [&reachableStates](const Transition& transition)
                                   {
                                       return reachableStates.find(transition.nextState) == reachableStates.end();
                                   }),
                    it->second.end());

            if (it->second.empty())
            {
                it = m_transitionTable.erase(it);
            } else
            {
                ++it;
            }
        }

        m_states.erase(
                std::remove_if(m_states.begin(), m_states.end(),
                               [&reachableStates](const std::string& state)
                               {
                                   return reachableStates.find(state) == reachableStates.end();
                               }),
                m_states.end());
    }

    void MergeEquivalentStates()
    {
        std::map<std::string, int> stateToClass;
        int classId = 0;
        for (const auto& state : m_states)
        {
            stateToClass[state] = classId++;
        }

        bool changed;
        do {
            changed = false;
            std::map<int, std::vector<std::string>> classToStates;

            for (const auto& [state, classIdx] : stateToClass)
            {
                classToStates[classIdx].push_back(state);
            }

            std::map<std::string, int> newStateToClass;
            int newClassId = 0;

            for (const auto& [_, statesInClass] : classToStates)
            {
                std::map<std::vector<std::pair<std::string, std::string>>, int> transitionsToNewClass;
                for (const auto& state : statesInClass)
                {
                    std::vector<std::pair<std::string, std::string>> transitions;

                    // Собираем пары (следующее состояние, выходной символ) для каждого входного символа
                    for (const auto& [inputSymbol, transitionsVec] : m_transitionTable)
                    {
                        auto it = std::find_if(transitionsVec.begin(), transitionsVec.end(),
                                               [&state](const Transition& transition)
                                               {
                                                   return transition.nextState == state;
                                               });

                        if (it != transitionsVec.end())
                        {
                            auto pair = std::make_pair(std::to_string(stateToClass[it->nextState]), it->outputSymbol);
                            transitions.emplace_back(pair);
                        }
                        else
                        {
                            std::pair<std::string, std::string> pair = std::make_pair("-1", "");
                            transitions.emplace_back(pair);
                        }
                    }

                    if (transitionsToNewClass.find(transitions) == transitionsToNewClass.end())
                    {
                        transitionsToNewClass[transitions] = newClassId++;
                    }

                    newStateToClass[state] = transitionsToNewClass[transitions];
                }
            }

            if (newStateToClass != stateToClass)
            {
                changed = true;
                stateToClass = std::move(newStateToClass);
            }
        } while (changed);

        for (const auto& [state, classIdx] : stateToClass)
        {
            if (state != m_states[classIdx])
            {
                ReplaceState(state, m_states[classIdx]);
            }
        }

        // Убираем дубликаты из списка состояний
        std::unordered_set<std::string> uniqueStates(m_states.begin(), m_states.end());
        m_states.assign(uniqueStates.begin(), uniqueStates.end());
    }

    void ReplaceState(const std::string& oldState, const std::string& newState)
    {
        for (auto& [inputSymbol, transitions] : m_transitionTable)
        {
            for (auto& transition : transitions)
            {
                if (transition.nextState == oldState)
                {
                    transition.nextState = newState;
                }
            }
        }
    }
};

#endif //LAB1_MEALYAUTOMAT_H
