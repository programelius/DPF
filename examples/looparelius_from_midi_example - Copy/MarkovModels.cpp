#include "MarkovModels.h"
#include <iostream> 
#include <iomanip>  
#include <algorithm> // For std::max

namespace { 
    double roundToPrecision(double value, int precision = 3) {
        double factor = std::pow(10.0, precision);
        return std::round(value * factor) / factor;
    }
}

MarkovModels::MarkovModels() : rng(std::random_device{}()) {}

void MarkovModels::clearModels() {
    pitchModel.clear();
    rhythmModel.clear();
    durationModel.clear();
}

void MarkovModels::buildModels(const std::vector<LoopareliusUtils::CustomMidiEvent>& loop) {
    clearModels(); 

    if (loop.size() < 2) {
        return;
    }

    for (size_t i = 0; i < loop.size() - 1; ++i) {
        const LoopareliusUtils::CustomMidiEvent& currentEvent = loop[i];
        const LoopareliusUtils::CustomMidiEvent& nextEvent = loop[i+1];

        pitchModel[currentEvent.noteNumber].push_back(nextEvent.noteNumber);

        double currentIOI = nextEvent.timestamp - currentEvent.timestamp;
        currentIOI = std::max(0.001, currentIOI); 
        double roundedCurrentIOI = roundToPrecision(currentIOI);

        if (i + 2 < loop.size()) {
            const LoopareliusUtils::CustomMidiEvent& nextNextEvent = loop[i+2];
            double nextIOI = nextNextEvent.timestamp - nextEvent.timestamp;
            nextIOI = std::max(0.001, nextIOI);
            rhythmModel[roundedCurrentIOI].push_back(roundToPrecision(nextIOI));
        } else if (loop.size() > 1) { 
            double firstIOIinLoop = loop[1].timestamp - loop[0].timestamp;
            firstIOIinLoop = std::max(0.001, firstIOIinLoop);
            rhythmModel[roundedCurrentIOI].push_back(roundToPrecision(firstIOIinLoop));
        }

        double roundedCurrentDuration = roundToPrecision(currentEvent.duration);
        double roundedNextDuration = roundToPrecision(nextEvent.duration);
        durationModel[roundedCurrentDuration].push_back(roundedNextDuration);
    }

    if (!loop.empty()) {
        double roundedLastEventDuration = roundToPrecision(loop.back().duration);
        // Check if key exists or if its vector is empty before adding fallback
        auto it = durationModel.find(roundedLastEventDuration);
        if (it == durationModel.end() || it->second.empty()) {
             if (it == durationModel.end()) { // Key doesn't exist, create it
                durationModel[roundedLastEventDuration].push_back(roundToPrecision(loop.front().duration));
             } else { // Key exists but vector is empty
                it->second.push_back(roundToPrecision(loop.front().duration));
             }
        }
    }
}

std::vector<LoopareliusUtils::CustomMidiEvent> MarkovModels::generateVariation(
    const std::vector<LoopareliusUtils::CustomMidiEvent>& seedLoop, 
    size_t numNotesToGenerate) 
{
    std::vector<LoopareliusUtils::CustomMidiEvent> variation;
    if (seedLoop.empty() || numNotesToGenerate == 0) {
        return variation;
    }

    LoopareliusUtils::CustomMidiEvent lastGeneratedEvent = seedLoop.front();
    lastGeneratedEvent.timestamp = 0.0;
    variation.push_back(lastGeneratedEvent);

    double currentPlaybackTime = 0.0;

    for (size_t i = 1; i < numNotesToGenerate; ++i) {
        int prevNote = lastGeneratedEvent.noteNumber;
        double prevDurationRounded = roundToPrecision(lastGeneratedEvent.duration);
        
        double prevIOIRounded = 0.5; 
        if (variation.size() > 1) {
            double actualPrevIOI = variation.back().timestamp - variation[variation.size()-2].timestamp;
            prevIOIRounded = roundToPrecision(std::max(0.001, actualPrevIOI));
        } else if (seedLoop.size() > 1) {
            double seedFirstIOI = seedLoop[1].timestamp - seedLoop[0].timestamp;
            prevIOIRounded = roundToPrecision(std::max(0.001, seedFirstIOI));
        }

        int nextNote = prevNote;
        auto pitchIt = pitchModel.find(prevNote);
        if (pitchIt != pitchModel.end() && !pitchIt->second.empty()) {
            std::uniform_int_distribution<> distrib(0, static_cast<int>(pitchIt->second.size() - 1));
            nextNote = pitchIt->second[distrib(rng)];
        } else {
            nextNote = seedLoop[i % seedLoop.size()].noteNumber;
        }

        double nextIOI = 0.5;
        auto rhythmIt = rhythmModel.find(prevIOIRounded);
        if (rhythmIt != rhythmModel.end() && !rhythmIt->second.empty()) {
            std::uniform_int_distribution<> distrib(0, static_cast<int>(rhythmIt->second.size() - 1));
            nextIOI = rhythmIt->second[distrib(rng)];
        } else {
            if (seedLoop.size() > i && i > 0) {
                double originalIOI = seedLoop[i].timestamp - seedLoop[i-1].timestamp;
                if (originalIOI > 0.0001) nextIOI = originalIOI;
            }
        }
        nextIOI = std::max(0.001, nextIOI);

        double nextDuration = 0.25;
        auto durationIt = durationModel.find(prevDurationRounded);
        if (durationIt != durationModel.end() && !durationIt->second.empty()) {
            std::uniform_int_distribution<> distrib(0, static_cast<int>(durationIt->second.size() - 1));
            nextDuration = durationIt->second[distrib(rng)];
        } else {
            nextDuration = seedLoop[i % seedLoop.size()].duration;
        }
        nextDuration = std::max(0.001, nextDuration);
            
        currentPlaybackTime += nextIOI;
        
        LoopareliusUtils::CustomMidiEvent newEvent(
            currentPlaybackTime,
            nextNote,
            seedLoop[i % seedLoop.size()].velocity,
            nextDuration,
            seedLoop[i % seedLoop.size()].channel
        );
        variation.push_back(newEvent);
        lastGeneratedEvent = newEvent;
    }
    return variation;
}

void MarkovModels::printModels() const {
    std::cout << "\n--- Pitch Model ---" << std::endl;
    for (const auto& pair : pitchModel) {
        std::cout << "  Note " << pair.first << " -> { ";
        for (int nextNote : pair.second) { std::cout << nextNote << " "; }
        std::cout << "}" << std::endl;
    }
    std::cout << "\n--- Rhythm (IOI) Model (seconds) ---" << std::endl;
    for (const auto& pair : rhythmModel) {
        std::cout << "  IOI " << std::fixed << std::setprecision(3) << pair.first << "s -> { ";
        for (double nextIOI : pair.second) { std::cout << std::fixed << std::setprecision(3) << nextIOI << "s "; }
        std::cout << "}" << std::endl;
    }
    std::cout << "\n--- Duration Model (seconds) ---" << std::endl;
    for (const auto& pair : durationModel) {
        std::cout << "  Dur " << std::fixed << std::setprecision(3) << pair.first << "s -> { ";
        for (double nextDur : pair.second) { std::cout << std::fixed << std::setprecision(3) << nextDur << "s "; }
        std::cout << "}" << std::endl;
    }
}