#define DPF_MIDI_EFFECT 1 // We are a MIDI effect
#include "DistrhoPlugin.hpp"
#include "DistrhoPluginChecks.h"
#include "DistrhoPluginPython.hpp" // For Python scripting

START_NAMESPACE_DISTRHO

class MyMIDILooperPlugin : public Plugin,
                           public PythonPlugin // Enable Python features
{
public:
    MyMIDILooperPlugin()
        : Plugin(NumParameters, 0, 0), // Parameters, Programs, States
          // Python script filename, Python Class name, pointer to this plugin instance
          PythonPlugin("main_looper_script.py", "MyMIDILooperDPF", this)
    {
        // Pass initial sample rate to Python if its __init__ needs it
        // This needs to be done carefully after Python environment is ready.
        // PythonPlugin might have a specific method for this, or you might call
        // a pythonExec in activate() or similar.
        // For now, assume Python's __init__ gets sampleRate via a DPF mechanism
        // or you call a python method like self.set_sample_rate() from activate().

        // Example: If your Python __init__ takes sample_rate.
        // DPF's PythonPlugin constructor might handle passing it, or you might need:
        // if (isPythonReady()) { // Check if Python interpreter is up
        //     pythonExec("self.instance.__init__(self.plugin_bridge_object, %f)", getSampleRate());
        // }
        // This part is DPF-specific for Python init. Often, sample_rate is passed
        // via a dpf_sample_rate_changed callback to Python.
    }

protected:
    // --- Parameters ---
    enum Parameters {
        paramCapture, // Index 0
        paramUndo,    // Index 1
        NumParameters // Total number of parameters
    };

    // --- Plugin Information ---
    const char* getLabel() const override { return "MyMIDILooper"; }
    const char* getDescription() const override { return "A live MIDI looper with Markov variations."; }
    const char* getMaker() const override { return "YourName"; }
    const char* getLicense() const override { return "ISC"; } // Or your chosen license
    uint32_t getVersion() const override { return d_version(0, 1, 0); } // Version 0.1.0
    int64_t getUniqueId() const override { return d_cconst('M', 'M', 'L', 'p'); } // Unique ID

    // --- Init Functions ---
    void initParameter(uint32_t index, Parameter& parameter) override {
        parameter.hints = kParameterIsAutomable;

        switch (index) {
        case paramCapture:
            parameter.name = "Capture Loop";
            parameter.symbol = "capture";
            parameter.hints |= kParameterIsButton | kParameterIsTrigger; // It's a button
            break;
        case paramUndo:
            parameter.name = "Undo Capture";
            parameter.symbol = "undo";
            parameter.hints |= kParameterIsButton | kParameterIsTrigger; // It's a button
            break;
        }
    }

    // --- Internal data ---
    // We don't need to store parameter values here if Python handles everything
    // float getParameterValue(uint32_t index) const override { /* Python might hold state */ return 0.0f; }
    // void setParameterValue(uint32_t index, float value) override {
    //     // This is where you notify Python
    //     if (isPythonReady()) {
    //          // Call the Python method directly via PythonPlugin's mechanisms
    //          // The PythonPlugin base class might automatically call a known Python function
    //          // like 'dpf_parameter_changed' if you implement pythonParameterChanged.
    //          // Or you might need:
    //          pythonExec("self.instance.dpf_parameter_changed(%u, %f)", index, value);
    //     }
    // }

    // --- Audio/MIDI Processing ---
    void run(const float**, float**, uint32_t nframes) override {
        if (!isPythonReady()) return;

        // 1. Pass current frame information to Python if needed for its internal timing
        //    This is crucial for frame-perfect MIDI scheduling from Python.
        //    DPF's PythonPlugin might have a helper, or you might call a Python method.
        const uint32_t host_frame = getCurrentFrame(); // Get current host frame
        pythonExec("self.instance.plugin_bridge_object_for_host_frame = %u", host_frame); // Example
        // A better way is to have a self.plugin.get_current_frame() method available in Python,
        // provided by the DPF Python bindings.

        // 2. Get MIDI input and send to Python
        const MidiEvent* midiEvents;
        uint32_t midiEventCount;
        midiEvents = getMidiEventsIn(0, midiEventCount); // Port 0

        for (uint32_t i = 0; i < midiEventCount; ++i) {
            const MidiEvent& ev(midiEvents[i]);
            // DPF's PythonPlugin has helpers to send MIDI to Python
            // This will call your Python MyMIDILooperDPF.dpf_midi_input method
            pythonMidiInput(0, ev.data, ev.size); // port_index, data_ptr, data_size
        }

        // 3. Call Python's main processing method
        // This will call your Python MyMIDILooperDPF.dpf_run method
        pythonRun(nframes);
    }

    // --- Python Integration Callbacks (from Python to C++) ---
    // These are called by self.plugin.send_midi_event_at_frame in Python
    void pythonMidiOutputAtFrame(const uint8_t* data, uint32_t size, uint32_t frame) override {
        sendMidiEventAtFrame(data, size, frame);
    }
    
    // If Python just uses send_midi_event (less precise for block processing)
    void pythonMidiOutput(const uint8_t* data, uint32_t size) override {
        sendMidiEvent(data, size);
    }

    // DPF PythonPlugin might have a pythonParameterChanged to directly call
    // your python class's dpf_parameter_changed
    void pythonParameterChanged(uint32_t index, float value) override {
         if (isPythonReady()) {
             pythonExec("self.instance.dpf_parameter_changed(%u, %f)", index, value);
         }
    }


    // Sample rate change notification to Python
    void sampleRateChanged(double newSampleRate) override {
        if (isPythonReady()) {
            pythonExec("self.instance.sample_rate = %f", newSampleRate);
            // Or call a specific Python method if more complex logic is needed
            // pythonExec("self.instance.dpf_sample_rate_changed(%f)", newSampleRate);
        }
    }


private:
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MyMIDILooperPlugin)
};

Plugin* createPlugin() {
    return new MyMIDILooperPlugin();
}

END_NAMESPACE_DISTRHO