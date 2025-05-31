#define DPF_MIDI_EFFECT 1 // Or DPF_AUDIO_EFFECT 0, DPF_SYNTH 0 if not a MIDI effect specifically
                          // For a minimal test, DPF_MIDI_EFFECT is fine.
#include "DistrhoPlugin.hpp"
#include "DistrhoPluginPython.hpp" // Required for Python plugins

// If your Python script needs to know the plugin name at init (optional)
#ifndef DISTRHO_PLUGIN_NAME
#define DISTRHO_PLUGIN_NAME "MinimalPythonTest"
#endif

START_NAMESPACE_DISTRHO

class MinimalPythonPlugin : public Plugin,
                            public PythonPlugin
{
public:
    MinimalPythonPlugin()
        : Plugin(0, 0, 0), // 0 Parameters, 0 Programs, 0 States for minimal
          // Python script filename, Python Class name inside that script, this plugin instance
          PythonPlugin("main_looper_script.py", "MyDPFPluginClass", this)
    {
        // You could call a Python init function here if needed, e.g.:
        // if (isPythonReady()) {
        //    pythonExec("self.instance.python_side_init(%f)", getSampleRate());
        // }
        DISTRHO_SAFE_ASSERT(isPythonReady(), "Python script failed to load or class not found!");
    }

protected:
    // --- Plugin Information ---
    const char* getLabel() const override { return DISTRHO_PLUGIN_NAME; }
    const char* getDescription() const override { return "A minimal DPF Python plugin test."; }
    const char* getMaker() const override { return "TestUser"; }
    const char* getLicense() const override { return "ISC"; }
    uint32_t getVersion() const override { return d_version(0, 0, 1); }
    int64_t getUniqueId() const override { return d_cconst('m', 'P', 'y', 'T'); } // Change for uniqueness

    // --- Init Functions ---
    // No parameters for this minimal test
    void initParameter(uint32_t index, Parameter& parameter) override {}

    // --- Internal data ---
    // No parameters to get/set for this minimal test
    float getParameterValue(uint32_t index) const override { return 0.0f; }
    void setParameterValue(uint32_t index, float value) override {
        // Example of how to call a Python method if you had parameters
        // if (isPythonReady()) {
        //     pythonExec("self.instance.dpf_parameter_changed(%u, %f)", index, value);
        // }
    }

    // --- Audio/MIDI Processing ---
    void run(const float** inputs, float** outputs, uint32_t nframes) override {
        if (!isPythonReady()) return;

        // Get MIDI events (if DPF_MIDI_EFFECT is true)
        const MidiEvent* midiEvents;
        uint32_t midiEventCount;
        midiEvents = getMidiEventsIn(0, midiEventCount); // Port 0

        for (uint32_t i = 0; i < midiEventCount; ++i) {
            const MidiEvent& ev(midiEvents[i]);
            pythonMidiInput(0, ev.data, ev.size); // Pass to Python's dpf_midi_input
        }

        // Call Python's run method (if it has one for processing)
        // For a purely MIDI effect that just uses dpf_midi_input, this might not call a Python run
        // If your Python dpf_run does audio or schedules MIDI out, call it:
        pythonRun(nframes); // This will call MyDPFPluginClass.dpf_run(nframes)
    }

    // --- Python Integration Callbacks (from Python to C++) ---
    // Called by self.plugin.send_midi_event_at_frame in Python
    void pythonMidiOutputAtFrame(const uint8_t* data, uint32_t size, uint32_t frame) override {
        sendMidiEventAtFrame(data, size, frame);
    }
    // Called by self.plugin.send_midi_event in Python
    void pythonMidiOutput(const uint8_t* data, uint32_t size) override {
        sendMidiEvent(data, size);
    }

    // Called if Python calls self.plugin.request_parameter_change
    // void pythonParameterChangeRequested(uint32_t index, float value) override {
    //    setParameterValue(index, value); // Example
    // }

    // Called from Python if it implements dpf_sample_rate_changed and DPF calls it
    void sampleRateChanged(double newSampleRate) override {
        Plugin::sampleRateChanged(newSampleRate); // Call base class method
        if (isPythonReady()) {
             // Example: Tell Python about it
             pythonExec("self.instance.host_sample_rate_changed(%f)", newSampleRate);
        }
    }


private:
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MinimalPythonPlugin)
};

Plugin* createPlugin() {
    return new MinimalPythonPlugin();
}

END_NAMESPACE_DISTRHO