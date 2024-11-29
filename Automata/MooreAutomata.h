#ifndef LAB1_MOOREAUTOMATA_H
#define LAB1_MOOREAUTOMATA_H

#include "IAutomata.h"
#include <iostream>
#include <utility>
#include <vector>
#include <regex>
#include <list>
#include <unordered_map>
#include <fstream>

class MooreAutomata final : public IAutomata {
public:
    MooreAutomata() = default;

    MooreAutomata(
            std::vector<std::string>&& inputSymbols,
            std::vector<std::pair<std::string, std::string>>&& statesInfo,
            std::list<std::pair<std::string, std::vector<std::string>>>&& transitionTable
    )
            : m_inputSymbols(std::move(inputSymbols)),
              m_statesInfo(std::move(statesInfo)),
              m_transitionTable(std::move(transitionTable))
    {}

    void ReadFromFile(const std::string &filename) override {
        std::list<std::pair<std::string, std::vector<std::string>>> transitionTable;

        std::ifstream file(filename);
        if (!file.is_open())
        {
            throw std::runtime_error("Could not open the file.");
        }

        ReadOutputSymbolsFromFile(file);
        ReadStatesFromFile(file);
        ReadTransitionsFromFile(file);

        file.close();
    }

    void PrintToFile(const std::string &filename) const override {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open the file: " + filename);
        }

        for (const auto &info: m_statesInfo) {
            file << ';' + info.second;
        }
        file << std::endl;

        for (const auto &info: m_statesInfo) {
            file << ';' + info.first;
        }
        file << std::endl;

        for (const auto &inputSymbol: m_inputSymbols) {
            file << inputSymbol;

            for (const auto &transitions: m_transitionTable) {
                if (transitions.first == inputSymbol) {
                    for (const auto &transition: transitions.second) {
                        file << ";" << transition;
                    }
                    file << std::endl;
                }
            }
        }

        file.close();
    }

    void Minimize() override
    {

    }

    [[nodiscard]] std::vector<std::string> GetInputSymbols() const
    {
        return m_inputSymbols;
    }

    [[nodiscard]] std::vector<std::string> GetOutputSymbols() const
    {
        return m_outputSymbols;
    }

    [[nodiscard]] std::vector<std::pair<std::string, std::string>> GetStatesInfo() const
    {
        return m_statesInfo;
    }

    [[nodiscard]] std::list<std::pair<std::string, std::vector<std::string>>> GetTransitionTable() const
    {
        return m_transitionTable;
    }

private:
    std::vector<std::string> m_inputSymbols = {};
    std::vector<std::string> m_outputSymbols = {};
    std::vector<std::pair<std::string, std::string>> m_statesInfo = {};
    std::list<std::pair<std::string, std::vector<std::string>>> m_transitionTable = {};

    inline void ReadOutputSymbolsFromFile(std::istream& input)
    {
        if (std::string line; std::getline(input, line))
        {
            std::stringstream ss(line);
            std::string outputSymbol;
            while (std::getline(ss, outputSymbol, ';'))
            {
                if (!outputSymbol.empty())
                {
                    m_outputSymbols.push_back(outputSymbol);
                }
            }
        }
    }

    inline void ReadStatesFromFile(std::istream& input)
    {
        std::string line;
        if (std::getline(input, line))
        {
            size_t index = 0;
            std::stringstream ss(line);
            std::string state;
            while (std::getline(ss, state, ';'))
            {
                if (!state.empty())
                {
                    m_statesInfo.emplace_back(state, m_outputSymbols.at(index++));
                }
            }
        }
    }

    inline void ReadTransitionsFromFile(std::istream& input)
    {
        std::string line;

        while (std::getline(input, line))
        {
            std::stringstream ss(line);
            std::string inputSymbol;
            if (std::getline(ss, inputSymbol, ';'))
            {
                std::vector<std::string> stateTransitions;
                std::string transition;
                while (std::getline(ss, transition, ';'))
                {
                    if (!transition.empty())
                    {
                        stateTransitions.push_back(transition);
                    }
                }
                m_inputSymbols.push_back(inputSymbol);
                m_transitionTable.emplace_back(inputSymbol, stateTransitions);
            }
        }
    }

    void RemoveUnreachableStates()
    {
        std::unordered_set<std::string> reachableStates;
        std::queue<std::string> queue;

        if (!m_statesInfo.empty()) {
            reachableStates.insert(m_statesInfo[0].first);
            queue.push(m_statesInfo[0].first);
        }

        while (!queue.empty()) {
            std::string currentState = queue.front();
            queue.pop();

            for (const auto& [inputSymbol, transitions] : m_transitionTable) {
                for (const std::string& nextState : transitions) {
                    if (reachableStates.find(nextState) == reachableStates.end()) {
                        reachableStates.insert(nextState);
                        queue.push(nextState);
                    }
                }
            }
        }

        m_statesInfo.erase(std::remove_if(m_statesInfo.begin(), m_statesInfo.end(),
                                          [&reachableStates](const std::pair<std::string, std::string>& stateInfo) {
                                              return reachableStates.find(stateInfo.first) == reachableStates.end();
                                          }),
                           m_statesInfo.end());

        for (auto it = m_transitionTable.begin(); it != m_transitionTable.end();) {
            auto& transitions = it->second;
            transitions.erase(std::remove_if(transitions.begin(), transitions.end(),
                                             [&reachableStates](const std::string& nextState) {
                                                 return reachableStates.find(nextState) == reachableStates.end();
                                             }),
                              transitions.end());

            if (transitions.empty()) {
                it = m_transitionTable.erase(it);
            } else {
                ++it;
            }
        }
    }
};


#endif //LAB1_MOOREAUTOMATA_H
