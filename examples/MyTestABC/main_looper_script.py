# Minimal DPF Python Plugin Script: main_looper_script.py

class MyDPFPluginClass: # Must match the class name specified in C++ PythonPlugin()
    def __init__(self, plugin_instance, sample_rate):
        """
        Called by DPF when the plugin is loaded.
        `plugin_instance` is a bridge object to call C++/DPF functions.
        `sample_rate` is the initial sample rate.
        """
        self.plugin = plugin_instance
        self.sample_rate = sample_rate
        self.host_frame = 0 # Example: track current host frame

        print(f"[Python MyDPFPluginClass] Initialized! Sample Rate: {self.sample_rate}")
        print(f"[Python MyDPFPluginClass] Plugin instance type: {type(self.plugin)}")
        
        # Example: Check if some plugin methods are available
        # if hasattr(self.plugin, 'get_current_frame'):
        #     print("[Python MyDPFPluginClass] self.plugin.get_current_frame() is available.")
        # else:
        #     print("[Python MyDPFPluginClass] WARNING: self.plugin.get_current_frame() not found!")
        # if hasattr(self.plugin, 'send_midi_event_at_frame'):
        #     print("[Python MyDPFPluginClass] self.plugin.send_midi_event_at_frame() is available.")
        # else:
        #     print("[Python MyDPFPluginClass] WARNING: self.plugin.send_midi_event_at_frame() not found!")


    def dpf_parameter_changed(self, index, value):
        """Called by DPF (via C++) when a plugin parameter changes."""
        print(f"[Python MyDPFPluginClass] Parameter {index} changed to: {value}")

    def dpf_midi_input(self, port_index, data_tuple):
        """
        Called by DPF (via C++) with incoming MIDI data.
        `data_tuple` is typically (status, data1, data2) or similar.
        """
        # print(f"[Python MyDPFPluginClass] MIDI In (port {port_index}): {data_tuple}")
        
        # Example: simple MIDI thru
        # self.plugin.send_midi_event(list(data_tuple)) # Send immediately (less precise)

        # For frame-accurate MIDI thru, use current frame + small offset
        # current_block_host_frame = self.plugin.get_current_frame() # Assuming this DPF API exists
        # self.plugin.send_midi_event_at_frame(list(data_tuple), 0) # Send at frame 0 of current block
        pass


    def dpf_run(self, nframes):
        """
        Called by DPF (via C++) for each audio processing block.
        `nframes` is the number of frames in this block.
        This is where real-time MIDI event generation/scheduling would happen.
        """
        # self.host_frame = self.plugin.get_current_frame() # Update current host frame
        # print(f"[Python MyDPFPluginClass] dpf_run called, nframes: {nframes}, host_frame: {self.host_frame}")
        
        # For this minimal test, do nothing here.
        pass

    def host_sample_rate_changed(self, new_sample_rate):
        """Called from C++ when sample rate changes."""
        self.sample_rate = new_sample_rate
        print(f"[Python MyDPFPluginClass] Sample rate changed to: {self.sample_rate}")

# Optional: A function DPF might call if defined (check DPF Python examples)
# def dpf_script_init(plugin_instance, sample_rate):
#     print("[Python dpf_script_init] called.")
#     return MyDPFPluginClass(plugin_instance, sample_rate)