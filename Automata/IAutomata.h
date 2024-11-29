#ifndef LAB1_IAUTOMATA_H
#define LAB1_IAUTOMATA_H

#include <string>
#include <utility>

struct Transition
{
    Transition(std::string  nextState, std::string  outputSymbol)
            : nextState(std::move(nextState)), outputSymbol(std::move(outputSymbol))
    {}

    std::string nextState;
    std::string outputSymbol;

    bool operator<(const Transition& other) const
    {
        if (nextState != other.nextState)
        {
            return nextState < other.nextState;
        }
        return outputSymbol < other.outputSymbol;
    }
};

class IAutomata
{
public:
    virtual void PrintToFile(const std::string& filename) const = 0;
    virtual void ReadFromFile(const std::string& filename) = 0;
    virtual void Minimize() = 0;
    virtual ~IAutomata() = default;
};

#endif //LAB1_IAUTOMATA_H
