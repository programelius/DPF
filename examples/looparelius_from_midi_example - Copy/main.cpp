// main.cpp (for testing - extended for Bite 3)
#include <iostream>
#include <vector>
#include <thread> 
#include <chrono> 
#include <cstdlib> 
#include <ctime>   
#include <random> // For std::mt19937, std::random_device

#include "MidiEvent.h"
#include "RetrospectiveMidiBuffer.h"
#include "MarkovModels.h" 

// getCurrentSimulatedTime function (as before)
double getCurrentSimulatedTime() {
    static double time = 0.0;
    time += (rand() % 401 + 100) / 1000.0; 
    return time;
}

int main() {
    srand(static_cast<unsigned int>(time(0))); 
    // For std::mt19937 in MarkovModels, it seeds itself with std::random_device
    // So, srand here is mostly for getCurrentSimulatedTime's rand()

    RetrospectiveMidiBuffer retroBuffer(5.0); 

    std::cout << "--- Simulating MIDI Input ---" << std::endl;
    std::vector<std::pair<int, double>> melody = { 
        {60, 1.0}, {62, 0.5}, {60, 1.0}, {64, 0.5}, 
        {65, 2.0}, {64, 0.5}, {62, 1.0}, {60, 0.5}, {58, 2.0} 
    };
    double base_duration = 0.3; 

    for (const auto& note_info : melody) {
        double currentTime = getCurrentSimulatedTime(); 
        int note = note_info.first;
        double duration = base_duration * note_info.second;
        int velocity = 90 + (rand() % 20);
        
        MidiEvent currentEvent(currentTime, note, velocity, duration, 0);
        retroBuffer.addEvent(currentEvent);
        std::cout << "Added: " << currentEvent.toString() << std::endl;
    }
    // Add a few more random notes
    for(int i=0; i<3; ++i) {
        double currentTime = getCurrentSimulatedTime();
        retroBuffer.addEvent(MidiEvent(currentTime, 55 + (rand()%17), 100, 0.1 + (rand()%2)/10.0, 0));
    }

    std::cout << "\n--- Captured & Normalized Loop ---" << std::endl;
    std::vector<MidiEvent> capturedLoop = retroBuffer.getNormalizedBufferedEvents();

    if (capturedLoop.empty()) {
        std::cout << "Captured loop is empty. Cannot proceed." << std::endl;
        return 1;
    } else {
        std::cout << "Captured " << capturedLoop.size() << " events for model building:" << std::endl;
        // for (const auto& event : capturedLoop) { // Optional: print captured loop
        //     std::cout << event.toString() << std::endl;
        // }
    }

    MarkovModels models;
    models.buildModels(capturedLoop);
    // models.printModels(); // Optional: print built models

    // --- Generate a few variations ---
    std::cout << "\n--- Generating Variations ---" << std::endl;
    for (int i = 0; i < 3; ++i) { // Generate 3 variations
        std::cout << "\n--- Variation " << i + 1 << " ---" << std::endl;
        // Generate a variation of the same length as the captured loop
        std::vector<MidiEvent> newVariation = models.generateVariation(capturedLoop, capturedLoop.size()); 
        
        if (newVariation.empty()) {
            std::cout << "Generated variation is empty." << std::endl;
        } else {
            for (const auto& event : newVariation) {
                std::cout << event.toString() << std::endl;
            }
            if (newVariation.size() > 1) {
                std::cout << "Approx. total time of variation: " 
                          << (newVariation.back().timestamp + newVariation.back().duration) 
                          << "s" << std::endl;
            }
        }
    }

    return 0;
}