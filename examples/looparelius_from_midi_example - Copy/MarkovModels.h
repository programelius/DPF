#ifndef MARKOV_MODELS_H
#define MARKOV_MODELS_H

#include "CustomMidiEvent.h" 
#include <vector>
#include <map>
#include <random> 
#include <cmath>  

class MarkovModels {
public:
    std::map<int, std::vector<int>> pitchModel;
    std::map<double, std::vector<double>> rhythmModel;
    std::map<double, std::vector<double>> durationModel;

    std::mt19937 rng; // Non-const for non-const generateVariation

    MarkovModels();

    void buildModels(const std::vector<LoopareliusUtils::CustomMidiEvent>& loop);
    void clearModels();
    void printModels() const; 
    
    std::vector<LoopareliusUtils::CustomMidiEvent> generateVariation(
        const std::vector<LoopareliusUtils::CustomMidiEvent>& seedLoop, 
        size_t numNotesToGenerate); 
};

#endif // MARKOV_MODELS_H