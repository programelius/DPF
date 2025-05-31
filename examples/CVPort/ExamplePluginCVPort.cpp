// LoopareliusPlugin.cpp
#include "LoopareliusPlugin.hpp"
#include <iostream> 
#include <cstring> 
#include "DistrhoDetails.hpp" 

START_NAMESPACE_DISTRHO

LoopareliusPlugin::LoopareliusPlugin()
    : Plugin(paramCount, 0, 0),
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

const char* LoopareliusPlugin::getLabel() const { return DISTRHO_PLUGIN_NAME; }
const char* LoopareliusPlugin::getMaker() const { return DISTRHO_PLUGIN_BRAND; }
const char* LoopareliusPlugin::getLicense() const { return "ISC"; }
uint32_t LoopareliusPlugin::getVersion() const { return DISTRHO_PLUGIN_VERSION; }
int64_t LoopareliusPlugin::getUniqueId() const { return DISTRHO_PLUGIN_UNIQUE_ID; }

void LoopareliusPlugin::initParameter(uint32_t index, Parameter& parameter) {
    if (index >= paramCount) {
        return;
    }

    switch (index) {
        case paramCaptureLoop:
            parameter.name = "Capture";
            parameter.symbol = "capture";
            // Assuming these are correct based on other DPF usage patterns.
            // If these are still not found, we absolutely need to see the enum Hints
            // definition from YOUR DistrhoPlugin.hpp
            parameter.hints = kParameterIsButton | kParameterIsTrigger; 
            parameter.ranges.def = 0.0f;
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 1.0f;
            break;
    }
}

float LoopareliusPlugin::getParameterValue(uint32_t index) const {
    // ... (as before)
    switch (index) {
        case paramCaptureLoop:
            return fCaptureLoopButtonState;
    }
    return 0.0f;
}

void LoopareliusPlugin::setParameterValue(uint32_t index, float value) {
    // ... (as before)
    switch (index) {
        case paramCaptureLoop:
            if (value > 0.5f && fCaptureLoopButtonState < 0.5f) { 
                std::cout << "[LoopareliusCpp] Capture Button Parameter Changed!" << std::endl;
                handleCaptureAction();
            }
            fCaptureLoopButtonState = value;
            break;
    }
}

void LoopareliusPlugin::activate() {
    // ... (as before)
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
    // ... (as before)
    std::cout << "[LoopareliusCpp] Plugin Deactivated." << std::endl;
}

void LoopareliusPlugin::handleCaptureAction() {
    // ... (as before)
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
    // ... (as before)
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
    variationPlayheadAbsoluteFrames = currentVariationStartAbsoluteFrame; 
    playbackActive = true;
    std::cout << "[LoopareliusCpp] Playback prepared. Total var frames: " << currentVariationTotalFrames << std::endl;
}

void LoopareliusPlugin::run(const float** /*inputs*/, float** /*outputs*/, uint32_t nframes,
                             const DISTRHO::MidiEvent* hostMidiEvents, uint32_t hostMidiEventCount)
{
    // ... (as before, with direct byte checks for MIDI status) ...
    const double sampleRate = getSampleRate();
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