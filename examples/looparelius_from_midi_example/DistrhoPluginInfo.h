#ifndef DISTRHO_PLUGIN_INFO_H_INCLUDED
#define DISTRHO_PLUGIN_INFO_H_INCLUDED

#define DISTRHO_PLUGIN_BRAND "LoopareliusBrand"
#define DISTRHO_PLUGIN_NAME  "LoopareliusCpp"
#define DISTRHO_PLUGIN_URI   "urn:loopareliusbrand:loopareliuscpp" // Make this unique

// Plugin version (optional, but good practice)
#define DISTRHO_VERSION_MAJOR 0
#define DISTRHO_VERSION_MINOR 1
#define DISTRHO_VERSION_PATCH 0
#define DISTRHO_PLUGIN_VERSION d_version(DISTRHO_VERSION_MAJOR, DISTRHO_VERSION_MINOR, DISTRHO_VERSION_PATCH)


#define DISTRHO_PLUGIN_HAS_UI        0 // 0 if no DPF-based UI, 1 if it has one
#define DISTRHO_PLUGIN_IS_RT_SAFE    1 // 1 if plugin is real-time safe
#define DISTRHO_PLUGIN_NUM_INPUTS    0 // For a MIDI effect
#define DISTRHO_PLUGIN_NUM_OUTPUTS   0 // For a MIDI effect
#define DISTRHO_PLUGIN_WANT_MIDI_INPUT  1
#define DISTRHO_PLUGIN_WANT_MIDI_OUTPUT 1

// Set ONE of these to true, others to false or undefined
#define DISTRHO_PLUGIN_IS_AUDIO_EFFECT false
#define DISTRHO_PLUGIN_IS_MIDI_EFFECT  true
#define DISTRHO_PLUGIN_IS_SYNTH        false

// Revert to d_cconst as this is what DPF's static_assert is designed for,
// even if the assert itself has issues with how STRINGIFY works on it.
// We will temporarily comment out the static_assert in DistrhoPluginChecks.h
#define DISTRHO_PLUGIN_UNIQUE_ID d_cconst('L','A','C','P') // "L"oop"A"relius"C"pp"P"lugin

// Define parameters
enum ParameterIndices {
    paramCaptureLoop,
    // paramClearLoop, // Future: A button to clear
    // ... other parameters later
    paramCount
};

#endif // DISTRHO_PLUGIN_INFO_H_INCLUDED