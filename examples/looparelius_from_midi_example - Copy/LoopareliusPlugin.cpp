// LoopareliusPlugin.cpp
#include "LoopareliusPlugin.hpp"
#include <iostream> 
#include <cstring> // For memcpy

// For DPF's MIDI event type and parameter hint constants
#include "DistrhoDetails.hpp" 


START_NAMESPACE_DISTRHO

LoopareliusPlugin::LoopareliusPlugin()
    : Plugin(paramCount, 0, 0), // Parameters from DistrhoPluginInfo.h
      retrospectiveBuffer(10.0), 
      playbackActive(false),
      variationPlayheadAbsoluteFrames(0),
      currentVariationStartAbsoluteFrame(0),
      currentVariationTotalFrames(0),
      currentAbsoluteFrameCounter(0),
      fCaptureLoopButtonState(0.0f)
{
    std::cout << "[LoopareliusCpp] Plugin Constructed. Sample Rate: " << getSampleRate() << std::endl;
}

LoopareliusPlugin::~LoopareliusPlugin() {
    std::cout << "[LoopareliusCpp] Plugin Destroyed." << std::endl;
}

// --- DPF Required Info Methods ---
const char* LoopareliusPlugin::getLabel() const {
    return DISTRHO_PLUGIN_NAME;
}
const char* LoopareliusPlugin::getMaker() const {
    return DISTRHO_PLUGIN_BRAND;
}
const char* LoopareliusPlugin::getLicense() const {
    return "ISC"; 
}
uint32_t LoopareliusPlugin::getVersion() const {
    return DISTRHO_PLUGIN_VERSION;
}
int64_t LoopareliusPlugin::getUniqueId() const {
    return DISTRHO_PLUGIN_UNIQUE_ID;
}

// --- Parameters ---
void LoopareliusPlugin::initParameter(uint32_t index, Parameter& parameter) {
    std::cout << "[LoopareliusCpp] initParameter called for Index: " << index << std::endl; // ADD THIS LINE

    if (index >= paramCount) {
        std::cout << "[LoopareliusCpp] initParameter: Index " << index << " is out of bounds (paramCount is " << paramCount << ")." << std::endl;
        return;
    }

    switch (index) {
        case paramCaptureLoop:
            parameter.name = "Capture";
            parameter.symbol = "capture";
            // Using the hints that we know are defined in your DistrhoDetails.hpp
            // kParameterIsTrigger implies boolean behavior (0 or 1, resets after run)
            // kParameterIsAutomatable is usually good to have.
            parameter.hints = kParameterIsTrigger | kParameterIsAutomatable; // TRY THIS COMBINATION
            parameter.ranges.def = 0.0f;
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 1.0f;
            std::cout << "[LoopareliusCpp] Initialized parameter 'Capture' (Index " << index << ") with hints: " << parameter.hints << std::endl;
            break;
        default:
            std::cout << "[LoopareliusCpp] initParameter: Unhandled index " << index << std::endl;
            break;
    }
}

float LoopareliusPlugin::getParameterValue(uint32_t index) const {
    switch (index) {
        case paramCaptureLoop:
            return fCaptureLoopButtonState;
    }
    return 0.0f;
}

void LoopareliusPlugin::setParameterValue(uint32_t index, float value) {
    std::cout << "[LoopareliusCpp] setParameterValue called. Index: " << index << ", Value: " << value << std::endl; // ALWAYS PRINT THIS

    switch (index) {
        case paramCaptureLoop:
            fCaptureLoopButtonState = value; // Store the value regardless for now
            std::cout << "[LoopareliusCpp] paramCaptureLoop (Index 0) received value: " << value << std::endl;
            
            // For now, trigger on any non-zero value to test if it's called
            if (value > 0.0f) { // Or even just `if (value != 0.0f)` or simply remove this if for testing
                std::cout << "[LoopareliusCpp] Capture condition met (value > 0.0f). Calling handleCaptureAction()." << std::endl;
                handleCaptureAction(); 
            }
            // else {
            //     std::cout << "[LoopareliusCpp] Capture condition NOT met (value <= 0.0f)." << std::endl;
            // }
            break;
        // Add other cases if you have more parameters
        default:
            std::cout << "[LoopareliusCpp] setParameterValue called for unhandled index: " << index << std::endl;
            break;
    }
}

// --- Plugin State ---
void LoopareliusPlugin::activate() {
    currentAbsoluteFrameCounter = 0; 
    retrospectiveBuffer.clear();
    capturedLoop.clear();
    currentVariation.clear();
    markovModels.clearModels();
    playbackActive = false;
    pendingHostNoteOns.clear();
    fCaptureLoopButtonState = 0.0f;
    std::cout << "[LoopareliusCpp] Plugin Activated. Sample Rate: " << getSampleRate() << std::endl;
}

void LoopareliusPlugin::deactivate() {
    std::cout << "[LoopareliusCpp] Plugin Deactivated." << std::endl;
}

// --- Core Logic Triggers ---
void LoopareliusPlugin::handleCaptureAction() {
    std::cout << "[LoopareliusCpp] Handling capture action..." << std::endl;
    capturedLoop = retrospectiveBuffer.getNormalizedBufferedEvents(); 
    
    if (capturedLoop.empty()) {
        std::cout << "[LoopareliusCpp] Captured loop is empty." << std::endl;
        playbackActive = false;
        return;
    }
    std::cout << "[LoopareliusCpp] Captured " << capturedLoop.size() << " events for model." << std::endl;

    markovModels.buildModels(capturedLoop);
    generateNewVariationAndPreparePlayback();
}

void LoopareliusPlugin::generateNewVariationAndPreparePlayback() {
    if (capturedLoop.empty()) {
        std::cout << "[LoopareliusCpp] No captured loop to generate variation from." << std::endl;
        playbackActive = false;
        currentVariation.clear();
        return;
    }

    currentVariation = markovModels.generateVariation(capturedLoop, capturedLoop.size());
    
    if (currentVariation.empty()) {
        std::cout << "[LoopareliusCpp] Generated variation is empty." << std::endl;
        playbackActive = false;
        return;
    }

    std::cout << "[LoopareliusCpp] Generated new variation with " << currentVariation.size() << " events." << std::endl;

    currentVariationTotalFrames = 0;
    if (!currentVariation.empty()) {
        const CustomMidiEvent& lastEvent = currentVariation.back(); 
        currentVariationTotalFrames = static_cast<uint32_t>((lastEvent.timestamp + lastEvent.duration) * getSampleRate());
    }
     if(currentVariationTotalFrames == 0 && !currentVariation.empty()) { 
        currentVariationTotalFrames = static_cast<uint32_t>(currentVariation.front().duration * getSampleRate());
    }

    currentVariationStartAbsoluteFrame = currentAbsoluteFrameCounter; 
    // variationPlayheadAbsoluteFrames = currentVariationStartAbsoluteFrame; // This line was redundant here
    playbackActive = true;
    std::cout << "[LoopareliusCpp] Playback prepared. Total var frames: " << currentVariationTotalFrames 
              << ". Starting at host frame: " << currentVariationStartAbsoluteFrame << std::endl;
}

// --- Real-time Audio/MIDI Processing ---
void LoopareliusPlugin::run(const float** /*inputs*/, float** /*outputs*/, uint32_t nframes,
                             const DISTRHO::MidiEvent* hostMidiEvents, uint32_t hostMidiEventCount)
{
    const double sampleRate = getSampleRate();

    // 1. Process Incoming MIDI
    for (uint32_t i = 0; i < hostMidiEventCount; ++i) {
        const DISTRHO::MidiEvent& hostEvent = hostMidiEvents[i];
        uint64_t eventIncomingAbsoluteFrame = currentAbsoluteFrameCounter + hostEvent.frame;

        const uint8_t statusByte = hostEvent.data[0];
        const uint8_t statusNoChannel = statusByte & 0xF0;
        const uint8_t channel = statusByte & 0x0F;
        const uint8_t note = hostEvent.data[1];
        const uint8_t velocityByte = hostEvent.data[2];

        auto key = std::make_pair(static_cast<int>(note), static_cast<int>(channel));

        if (statusNoChannel == 0x90 && velocityByte > 0) { 
            pendingHostNoteOns[key] = std::make_pair(eventIncomingAbsoluteFrame, static_cast<int>(velocityByte));
        } else if (statusNoChannel == 0x80 || (statusNoChannel == 0x90 && velocityByte == 0)) { 
            auto it = pendingHostNoteOns.find(key);
            if (it != pendingHostNoteOns.end()) {
                uint64_t noteOnFrame = it->second.first;
                int actualVelocity = it->second.second;
                double durationSeconds = static_cast<double>(eventIncomingAbsoluteFrame - noteOnFrame) / sampleRate;
                
                if (durationSeconds < 0.001) durationSeconds = 0.001;

                retrospectiveBuffer.addEvent(
                    CustomMidiEvent(static_cast<double>(noteOnFrame) / sampleRate, 
                                   key.first, actualVelocity, durationSeconds, key.second)
                );
                pendingHostNoteOns.erase(it);
            }
        }
    }

    // 2. Handle Playback of Current Variation
    if (playbackActive && !currentVariation.empty()) {
        uint64_t currentBlockEndAbsoluteFrame = currentAbsoluteFrameCounter + nframes;

        for (const CustomMidiEvent& eventToPlay : currentVariation) { 
            uint64_t noteOnAbsoluteFrameForVariation = currentVariationStartAbsoluteFrame + 
                                                       static_cast<uint64_t>(eventToPlay.timestamp * sampleRate);
            uint64_t noteOffAbsoluteFrameForVariation = noteOnAbsoluteFrameForVariation + 
                                                        static_cast<uint64_t>(eventToPlay.duration * sampleRate);
            
            DISTRHO::MidiEvent dpfEventToSend; 
            dpfEventToSend.size = 3; 
            if (dpfEventToSend.size < 4) dpfEventToSend.data[3] = 0;

            if (noteOnAbsoluteFrameForVariation >= currentAbsoluteFrameCounter && 
                noteOnAbsoluteFrameForVariation < currentBlockEndAbsoluteFrame) 
            {
                dpfEventToSend.frame = static_cast<uint32_t>(noteOnAbsoluteFrameForVariation - currentAbsoluteFrameCounter);
                dpfEventToSend.data[0] = static_cast<uint8_t>(0x90 | eventToPlay.channel);
                dpfEventToSend.data[1] = static_cast<uint8_t>(eventToPlay.noteNumber);
                dpfEventToSend.data[2] = static_cast<uint8_t>(eventToPlay.velocity);
                writeMidiEvent(dpfEventToSend); 
            }

            if (noteOffAbsoluteFrameForVariation >= currentAbsoluteFrameCounter && 
                noteOffAbsoluteFrameForVariation < currentBlockEndAbsoluteFrame)
            {
                dpfEventToSend.frame = static_cast<uint32_t>(noteOffAbsoluteFrameForVariation - currentAbsoluteFrameCounter);
                dpfEventToSend.data[0] = static_cast<uint8_t>(0x80 | eventToPlay.channel);
                dpfEventToSend.data[1] = static_cast<uint8_t>(eventToPlay.noteNumber);
                dpfEventToSend.data[2] = static_cast<uint8_t>(0); 
                writeMidiEvent(dpfEventToSend);
            }
        }
        
        if ( (currentAbsoluteFrameCounter + nframes) >= (currentVariationStartAbsoluteFrame + currentVariationTotalFrames) &&
             currentVariationTotalFrames > 0 ) 
        {
             std::cout << "[LoopareliusCpp] Variation ended. Generating new one." << std::endl;
             generateNewVariationAndPreparePlayback(); 
        }
    }
    currentAbsoluteFrameCounter += nframes;
}

Plugin* createPlugin() {
    return new LoopareliusPlugin();
}

END_NAMESPACE_DISTRHO