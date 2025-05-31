#ifndef LOOPARELIUS_PLUGIN_HPP_INCLUDED
#define LOOPARELIUS_PLUGIN_HPP_INCLUDED

#include "DistrhoPlugin.hpp"        
#include "CustomMidiEvent.h"        
#include "RetrospectiveMidiBuffer.h"
#include "MarkovModels.h"           

#include <vector>
#include <map> 

using LoopareliusUtils::CustomMidiEvent; // Use our namespaced struct

START_NAMESPACE_DISTRHO

class LoopareliusPlugin : public Plugin {
public:
    LoopareliusPlugin();
    ~LoopareliusPlugin() override;

protected:
    const char* getLabel() const override;
    const char* getMaker() const override;
    const char* getLicense() const override;
    uint32_t getVersion() const override;
    int64_t getUniqueId() const override;

    void initParameter(uint32_t index, Parameter& parameter) override;
    float getParameterValue(uint32_t index) const override;
    void setParameterValue(uint32_t index, float value) override;

    void activate() override;
    void deactivate() override;
    void run(const float** inputs, float** outputs, uint32_t nframes,
             const MidiEvent* midiEvents, uint32_t midiEventCount) override; 

private:
    RetrospectiveMidiBuffer retrospectiveBuffer;
    MarkovModels markovModels;
    std::vector<CustomMidiEvent> capturedLoop;     
    std::vector<CustomMidiEvent> currentVariation; 
    
    bool playbackActive;
    uint64_t variationPlayheadAbsoluteFrames; 
    uint64_t currentVariationStartAbsoluteFrame; 
    uint32_t currentVariationTotalFrames;     

    uint64_t currentAbsoluteFrameCounter; 
    
    std::map<std::pair<int, int>, std::pair<uint64_t, int>> pendingHostNoteOns;

    float fCaptureLoopButtonState;

    void handleCaptureAction();
    void generateNewVariationAndPreparePlayback();
    
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoopareliusPlugin)
};

END_NAMESPACE_DISTRHO

#endif // LOOPARELIUS_PLUGIN_HPP_INCLUDED