#ifndef LAB1_IAUTOMATA_H
#define LAB1_IAUTOMATA_H

#include "../stdafx.h"

class IAutomata
{
public:
    virtual void PrintToFile(const std::string& filename) const = 0;
    virtual void ReadFromFile(const std::string& filename) = 0;
    virtual void Minimize() = 0;
    virtual ~IAutomata() = default;
};

#endif //LAB1_IAUTOMATA_H
