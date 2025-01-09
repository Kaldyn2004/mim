#pragma once
#include <set>
#include <stdexcept>
#include <string>

class Group
{
public:
    void AddState(const std::string& state)
    {
        m_states.emplace(state);
    }

    void SetStates(const std::set<std::string>& states)
    {
        m_states = states;
    }

    void RemoveState(const std::string& state)
    {
        if (m_states.contains(state))
        {
            m_states.erase(state);
        }
    }

    [[nodiscard]] std::set<std::string> GetStates() const
    {
        return m_states;
    }

    [[nodiscard]] size_t GetStatesCount() const
    {
        return m_states.size();
    }

    std::string GetMainState()
    {
        for (auto& state : m_states)
        {
            return state;
        }

        throw std::runtime_error("No state found");
    }

    void ClearStates()
    {
        m_states.clear();
    }

private:
    std::set<std::string> m_states {};
};
