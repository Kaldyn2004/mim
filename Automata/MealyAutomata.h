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
using namespace std;

class MealyAutomata final : public IAutomata
{
public:
    MealyAutomata() = default;

    void ReadFromFile(const std::string &filename) override
    {
        ifstream file(filename);
        if (!file.is_open())
        {
            cerr << "Error: Unable to open file " << filename << endl;
            exit(1);
        }

        string line;

        getline(file, line);
        stringstream header(line);
        string cell;
        getline(header, cell, ';');
        while (getline(header, cell, ';'))
        {
            m_states.push_back(cell);
        }

        while (getline(file, line))
        {
            stringstream row(line);
            string inputSymbol;
            getline(row, inputSymbol, ';');
            m_inputSymbols.push_back(inputSymbol);

            vector<pair<string, string>> rowTransitions;
            while (getline(row, cell, ';'))
            {
                auto pos = cell.find('/');
                string nextState = cell.substr(0, pos);
                string outputSymbol = cell.substr(pos + 1);
                rowTransitions.emplace_back(nextState, outputSymbol);
            }
            m_transitions.push_back(rowTransitions);
        }

        file.close();
    }

    void Minimize() override
    {
        unordered_set<string> reachable;
        queue<string> toVisit;
        toVisit.push(m_states[0]);

        while (!toVisit.empty())
        {
            string current = toVisit.front();
            toVisit.pop();

            if (reachable.count(current)) continue;
            reachable.insert(current);

            int stateIndex = find(m_states.begin(), m_states.end(), current) - m_states.begin();
            for (const auto &transition : m_transitions)
            {
                string nextState = transition[stateIndex].first;
                if (!reachable.count(nextState))
                {
                    toVisit.push(nextState);
                }
            }
        }

        vector<string> reducedStates;
        vector<vector<pair<string, string>>> reducedTransitions;

        for (const string &state : m_states)
        {
            if (reachable.count(state)) reducedStates.push_back(state);
        }

        for (const auto &row : m_transitions)
        {
            vector<pair<string, string>> newRow;
            for (size_t i = 0; i < m_states.size(); ++i)
            {
                if (reachable.count(m_states[i]))
                {
                    newRow.push_back(row[i]);
                }
            }
            reducedTransitions.push_back(newRow);
        }

        m_states = move(reducedStates);
        m_transitions = move(reducedTransitions);

        vector<int> partition(m_states.size(), 0);
        unordered_map<string, int> outputMap;

        for (size_t i = 0; i < m_states.size(); ++i)
        {
            string outputs;
            for (const auto &row : m_transitions)
            {
                outputs += row[i].second + ",";
            }
            if (outputMap.find(outputs) == outputMap.end())
            {
                outputMap[outputs] = outputMap.size();
            }
            partition[i] = outputMap[outputs];
        }

        bool updated;
        do
        {
            updated = false;
            unordered_map<string, int> newPartitionMap;

            for (size_t i = 0; i < m_states.size(); ++i)
            {
                string key = to_string(partition[i]) + ",";
                for (const auto &row : m_transitions)
                {
                    int nextIndex = find(m_states.begin(), m_states.end(), row[i].first) - m_states.begin();
                    key += to_string(partition[nextIndex]) + ",";
                }
                if (newPartitionMap.find(key) == newPartitionMap.end())
                {
                    newPartitionMap[key] = newPartitionMap.size();
                }
                if (partition[i] != newPartitionMap[key])
                {
                    updated = true;
                }
                partition[i] = newPartitionMap[key];
            }
        } while (updated);

        unordered_map<int, string> stateMap;
        vector<string> minimizedStates;
        vector<vector<pair<string, string>>> minimizedTransitions(m_inputSymbols.size());
        char sim = m_states[0][0];

        for (size_t i = 0; i < m_states.size(); ++i)
        {
            if (stateMap.find(partition[i]) == stateMap.end())
            {
                stateMap[partition[i]] = sim + to_string(stateMap.size());
                minimizedStates.push_back(stateMap[partition[i]]);
            }
        }

        for (size_t i = 0; i < m_inputSymbols.size(); ++i)
        {
            vector<pair<string, string>> newRow;
            for (size_t j = 0; j < m_states.size(); ++j)
            {
                int nextIndex = find(m_states.begin(), m_states.end(), m_transitions[i][j].first) - m_states.begin();
                string newState = stateMap[partition[nextIndex]];
                string outputSymbol = m_transitions[i][j].second;
                newRow.emplace_back(newState, outputSymbol);
            }
            minimizedTransitions[i] = newRow;
        }

        m_states = move(minimizedStates);
        m_transitions = move(minimizedTransitions);
    }

    void PrintToFile(const std::string &filename) const override
    {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Unable to write to file " << filename << endl;
            exit(1);
        }

        for (const string &state : m_states)
        {
            file << ";" << state;
        }
        file << endl;

        for (size_t i = 0; i < m_inputSymbols.size(); ++i)
        {
            file << m_inputSymbols[i];
            for (const auto &transition : m_transitions[i])
            {
                file << ";" << transition.first << "/" << transition.second;
            }
            file << endl;
        }

        file.close();
    }

private:
    vector<string> m_states;
    vector<string> m_inputSymbols;
    vector<vector<pair<string, string>>> m_transitions;
};

#endif
