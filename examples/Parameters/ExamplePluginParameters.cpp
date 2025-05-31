/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2024 Filipe Coelho <falktx@falktx.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "DistrhoPlugin.hpp"
#include <iostream> // For std::cout
#include <cstring>  // For std::memset, std::memcpy

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------------------------------------------

/**
  Simple plugin to demonstrate parameter usage (including UI).
  The plugin will be treated as an effect, but it will not change the host audio.
 */
class ExamplePluginParameters : public Plugin
{
public:
    ExamplePluginParameters()
        : Plugin(9, 2, 0) // 9 parameters, 2 programs, 0 states
    {
       /**
          Initialize all our parameters to their defaults.
          In this example all parameters have 0 as default, so we can simply zero them.
        */
        std::memset(fParamGrid, 0, sizeof(float)*9);
        std::cout << "ExamplePluginParameters CONSTRUCTOR CALLED" << std::endl;
    }

protected:
   /* --------------------------------------------------------------------------------------------------------
    * Information */

    const char* getLabel() const override
    {
        return "parameters";
    }

    const char* getDescription() const override
    {
        return "Simple plugin to demonstrate parameter usage (including UI).\n\
The plugin will be treated as an effect, but it will not change the host audio.";
    }

    const char* getMaker() const override
    {
        return "DISTRHO";
    }

    const char* getHomePage() const override
    {
        return "https://github.com/DISTRHO/DPF";
    }

    const char* getLicense() const override
    {
        return "ISC";
    }

    uint32_t getVersion() const override
    {
        return d_version(1, 0, 0);
    }

   /* --------------------------------------------------------------------------------------------------------
    * Init */

    enum PortGroupIds {
        portGroupTop = 0,
        portGroupMiddle,
        portGroupBottom
    };

    void initAudioPort(bool input, uint32_t index, AudioPort& port) override
    {
        port.groupId = kPortGroupStereo;
        Plugin::initAudioPort(input, index, port);
    }

    void initParameter(uint32_t index, Parameter& parameter) override
    {
        parameter.hints = kParameterIsAutomatable|kParameterIsBoolean;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        parameter.ranges.def = 0.0f;

        switch (index)
        {
        case 0:
            parameter.name = "top-left";
            parameter.groupId = portGroupTop;
            break;
        case 1:
            parameter.name = "top-center";
            parameter.groupId = portGroupTop;
            break;
        case 2:
            parameter.name = "top-right";
            parameter.groupId = portGroupTop;
            break;
        case 3:
            parameter.name = "middle-left";
            parameter.groupId = portGroupMiddle;
            break;
        case 4:
            parameter.name = "middle-center";
            parameter.groupId = portGroupMiddle;
            break;
        case 5:
            parameter.name = "middle-right";
            parameter.groupId = portGroupMiddle;
            break;
        case 6:
            parameter.name = "bottom-left";
            parameter.groupId = portGroupBottom;
            break;
        case 7:
            parameter.name = "bottom-center";
            parameter.groupId = portGroupBottom;
            break;
        case 8:
            parameter.name = "bottom-right";
            parameter.groupId = portGroupBottom;
            break;
        }

        parameter.symbol = parameter.name;
        parameter.symbol.replace('-', '_');
    }

    void initPortGroup(uint32_t groupId, PortGroup& portGroup) override
    {
        switch (groupId) {
        case portGroupTop:
            portGroup.name = "Top";
            portGroup.symbol = "top";
            break;
        case portGroupMiddle:
            portGroup.name = "Middle";
            portGroup.symbol = "middle";
            break;
        case portGroupBottom:
            portGroup.name = "Bottom";
            portGroup.symbol = "bottom";
            break;
        }
    }

    void initProgramName(uint32_t index, String& programName) override
    {
        switch (index)
        {
        case 0:
            programName = "Default";
            break;
        case 1:
            programName = "Custom";
            break;
        }
    }

   /* --------------------------------------------------------------------------------------------------------
    * Internal data */

    float getParameterValue(uint32_t index) const override
    {
        return fParamGrid[index];
    }

    void setParameterValue(uint32_t index, float value) override
    {
        std::cout << "ExamplePluginParameters::setParameterValue CALLED - Index: " << index << ", Value: " << value << std::endl;
        fParamGrid[index] = value;
    }

    void loadProgram(uint32_t index) override
    {
        switch (index)
        {
        case 0:
            for (int i=0; i<9; ++i) fParamGrid[i] = 0.0f;
            break;
        case 1:
            fParamGrid[0] = 1.0f;
            fParamGrid[1] = 1.0f;
            fParamGrid[2] = 0.0f;
            fParamGrid[3] = 0.0f;
            fParamGrid[4] = 1.0f;
            fParamGrid[5] = 1.0f;
            fParamGrid[6] = 1.0f;
            fParamGrid[7] = 0.0f;
            fParamGrid[8] = 1.0f;
            break;
        }
    }

   /* --------------------------------------------------------------------------------------------------------
    * Process */

   /**
      Run/process function for plugins without MIDI input.
    */
    void run(const float** inputs, float** outputs, uint32_t frames) override
    {
       /**
          This plugin does nothing, it just demonstrates parameter usage.
          So here we directly copy inputs over outputs, leaving the audio untouched.
          We need to be careful in case the host re-uses the same buffer for both inputs and outputs.
        */
        // Check if inputs and outputs are valid pointers before memcpy (basic safety)
        // The plugin is constructed with 2 audio inputs and 2 audio outputs by default
        // if initAudioPort is not changed to 0. This example calls Plugin::initAudioPort
        // which will setup default 2 in / 2 out if not already.
        // So, inputs[0], inputs[1], outputs[0], outputs[1] should be valid.

        if (outputs[0] != inputs[0]) // Ensure inputs[0] and outputs[0] are valid if used
            std::memcpy(outputs[0], inputs[0], sizeof(float)*frames);

        // This example plugin declares itself as stereo (2 in, 2 out by default, and initAudioPort confirms stereo grouping)
        // So, we expect inputs[1] and outputs[1] to be valid.
        if (outputs[1] != inputs[1]) // Ensure inputs[1] and outputs[1] are valid if used
            std::memcpy(outputs[1], inputs[1], sizeof(float)*frames);
    }

    // -------------------------------------------------------------------------------------------------------

private:
    float fParamGrid[9];
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExamplePluginParameters)
};

/* ------------------------------------------------------------------------------------------------------------
 * Plugin entry point, called by DPF to create a new plugin instance. */

Plugin* createPlugin()
{
    return new ExamplePluginParameters();
}

// -----------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO