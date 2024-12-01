#include "Automata/MealyAutomata.h"
#include "Automata/MooreAutomata.h"
#include <memory>
#include <iostream>
#include <string>

void Minimize(std::unique_ptr<IAutomata> automat, const std::string& inputFile, const std::string& outputFile)
{
    try
    {
        automat->ReadFromFile(inputFile);
        automat->Minimize();
        automat->PrintToFile(outputFile);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error during processing: " << e.what() << std::endl;
    }
}


int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        std::cerr << "Wrong input data" << std::endl;
        std::cerr << "Usage: " << argv[0] << " mealy mealy.csv mealy_min.csv" << std::endl;
        std::cerr << "   or: " << argv[0] << " moore mealy.csv mealy_min.csv" << std::endl;
        return 1;
    }

    std::string command = argv[1];
    std::string inputFile = argv[2];
    std::string outputFile = argv[3];

    try {
        if (command == "mealy")
        {
            auto automaton = std::make_unique<MealyAutomata>();
            Minimize(std::move(automaton), inputFile, outputFile);
        } else if (command == "moore")
        {
            auto automaton = std::make_unique<MooreAutomata>();
            Minimize(std::move(automaton), inputFile, outputFile);
        } else
        {
            throw std::invalid_argument("Invalid automaton command: " + command);
        }
    } catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
