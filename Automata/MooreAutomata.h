#ifndef LAB1_MOOREAUTOMAT_H
#define LAB1_MOOREAUTOMAT_H

#include "IAutomata.h"
#include <iostream>
#include <vector>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <fstream>
#include <stdexcept>
using namespace std;

class MooreAutomata final : public IAutomata
{
public:
    MooreAutomata() = default;

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
            m_outputSymbols.push_back(cell);
        }

        getline(file, line);
        stringstream stateHeader(line);
        getline(stateHeader, cell, ';');
        while (getline(stateHeader, cell, ';'))
        {
            m_states.push_back(cell);
        }

        while (getline(file, line))
        {
            stringstream row(line);
            string inputSymbol;
            getline(row, inputSymbol, ';');
            m_inputSymbols.push_back(inputSymbol);

            vector<string> rowTransitions;
            while (getline(row, cell, ';'))
            {
                rowTransitions.push_back(cell);
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
            for (const auto &row : m_transitions)
            {
                string nextState = row[stateIndex];
                if (!reachable.count(nextState))
                {
                    toVisit.push(nextState);
                }
            }
        }

        vector<string> reducedStates;
        vector<string> reducedOutputSymbols;
        vector<vector<string>> reducedTransitions;

        for (size_t i = 0; i < m_states.size(); ++i)
        {
            if (reachable.count(m_states[i])) {
                reducedStates.push_back(m_states[i]);
                reducedOutputSymbols.push_back(m_outputSymbols[i]);
            }
        }

        for (const auto &row : m_transitions)
        {
            vector<string> newRow;
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
        m_outputSymbols = move(reducedOutputSymbols);
        m_transitions = move(reducedTransitions);

        vector<int> partition(m_states.size(), 0);
        unordered_map<string, int> outputMap;

        for (size_t i = 0; i < m_states.size(); ++i) {
            string outputs = m_outputSymbols[i];
            if (outputMap.find(outputs) == outputMap.end())
            {
                outputMap[outputs] = outputMap.size();
            }
            partition[i] = outputMap[outputs];
        }

        bool updated;
        do {
            updated = false;
            unordered_map<string, int> newPartitionMap;

            for (size_t i = 0; i < m_states.size(); ++i)
            {
                string key = to_string(partition[i]) + ",";
                for (const auto &row : m_transitions)
                {
                    int nextIndex = find(m_states.begin(), m_states.end(), row[i]) - m_states.begin();
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
        vector<string> minimizedOutputSymbols;
        vector<vector<string>> minimizedTransitions(m_inputSymbols.size());
        char sim = m_states[0][0];

        for (size_t i = 0; i < m_states.size(); ++i)
        {
            if (stateMap.find(partition[i]) == stateMap.end())
            {
                stateMap[partition[i]] = sim + to_string(stateMap.size());
                minimizedStates.push_back(stateMap[partition[i]]);
                minimizedOutputSymbols.push_back(m_outputSymbols[i]);
            }
        }

        for (size_t i = 0; i < m_inputSymbols.size(); ++i)
        {
            vector<string> newRow;
            for (size_t j = 0; j <minimizedStates.size(); ++j)
            {
                int nextIndex = find(minimizedStates.begin(), minimizedStates.end(), m_transitions[i][j]) - minimizedStates.begin();
                newRow.push_back(stateMap[partition[nextIndex]]);
            }
            minimizedTransitions[i] = newRow;
        }

        m_states = move(minimizedStates);
        m_outputSymbols = move(minimizedOutputSymbols);
        m_transitions = move(minimizedTransitions);
    }

    void PrintToFile(const std::string &filename) const override
    {
        ofstream file(filename);
        if (!file.is_open())
        {
            cerr << "Error: Unable to write to file " << filename << endl;
            exit(1);
        }

        for (const string &outputSymbol : m_outputSymbols)
        {
            file<< ";" << outputSymbol;
        }
        file << endl;

        for (const string &state : m_states)
        {
            file << ";" << state;
        }
        file << endl;

        for (size_t i = 0; i < m_inputSymbols.size(); ++i)
        {
            file << m_inputSymbols[i];
            for (const auto &transition : m_transitions[i]) {
                file << ";" << transition;
            }
            file << endl;
        }

        file.close();
    }

private:
    vector<string> m_states;
    vector<string> m_outputSymbols;
    vector<string> m_inputSymbols;
    vector<vector<string>> m_transitions;
};

#endif // LAB1_MOOREAUTOMAT_H
