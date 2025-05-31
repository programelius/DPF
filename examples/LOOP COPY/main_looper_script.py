# main_looper_script.py
from collections import deque, defaultdict
import time # For timestamps in retrospective buffer if not using DPF host time
import random
# REMOVE: import rtmidi
# REMOVE: import threading (for note-offs, DPF has other ways)
# REMOVE: import keyboard (use DPF parameters for UI controls)

class MyMIDILooperDPF: # Choose a class name
    def __init__(self, plugin_instance, sample_rate):
        self.plugin = plugin_instance  # This is the DPF C++ bridge object
        self.sample_rate = sample_rate
        print(f"[MyMIDILooperDPF] Initialized with sample rate: {sample_rate}")

        # --- Port your global variables to instance variables ---
        self.RETROSPECTIVE_BUFFER_MAX_TIME = 10.0
        self.retrospective_buffer = deque()
        self.captured_loops_history = deque(maxlen=10)
        self.current_source_loop = []
        self.loop_is_active = False
        self.pitch_markov_model = defaultdict(list)
        self.rhythm_markov_model = defaultdict(list)
        self.duration_markov_model = defaultdict(list)
        self.current_variation = []
        self.playback_active = False
        self.variation_playhead_index = 0
        self.next_event_playback_time_host_frames = 0 # For DPF timing
        self.current_block_frame_offset = 0 # To track frames within a block

        self.pending_note_ons = {} # (note, channel) -> (host_frame_on, velocity)
        
        # For DPF frame-based note-off scheduling
        # list of (frame_to_send_note_off, note, channel)
        self.pending_note_offs_frames = []


    # --- DPF CALLBACK METHODS ---
    def dpf_parameter_changed(self, index, value):
        print(f"[MyMIDILooperDPF] Parameter {index} changed to {value}")
        # This is where you'll handle button presses from the DAW UI
        # Parameter '0' could be "Capture", '1' could be "Undo"
        # For a button, value will be > 0.5 when pressed, then back to 0
        if index == 0 and value > 0.5: # Assuming param 0 is Capture
            self.capture_new_loop_action()
        elif index == 1 and value > 0.5: # Assuming param 1 is Undo
            self.undo_last_capture_action()
        # ...

    def dpf_midi_input(self, port_index, data_tuple):
        # data_tuple is typically (status_byte, data1, data2) or similar
        # print(f"[MyMIDILooperDPF] MIDI In (port {port_index}): {data_tuple}")
        
        # Get host frame for precise timing if available
        # current_host_frame = self.plugin.get_current_frame() # Or similar DPF API
        # For now, let's use time.time() for retrospective buffer, but aim for host frames
        current_time_for_buffer = time.time() 

        status = data_tuple[0]
        message_type = status & 0xF0
        channel = status & 0x0F

        if message_type == 0x90 and len(data_tuple) > 2: # Note On
            note, velocity = data_tuple[1], data_tuple[2]
            if velocity > 0:
                # Use a DPF provided frame counter if possible for precise timing
                # For now, we'll use time.time() for the retrospective buffer's internal logic
                self.pending_note_ons[(note, channel)] = (current_time_for_buffer, velocity)
            else: # Note On w/ vel 0 = Note Off
                if (note, channel) in self.pending_note_ons:
                    note_on_time, note_on_velocity = self.pending_note_ons.pop((note, channel))
                    duration = max(0.01, current_time_for_buffer - note_on_time)
                    # Create your MidiEvent (you might need to adapt its timestamp to be relative or use host frames)
                    event = self._create_midi_event(note_on_time, note, note_on_velocity, duration, channel)
                    self.process_incoming_midi(event)
        elif message_type == 0x80 and len(data_tuple) > 2: # Note Off
            note = data_tuple[1]
            if (note, channel) in self.pending_note_ons:
                note_on_time, note_on_velocity = self.pending_note_ons.pop((note, channel))
                duration = max(0.01, current_time_for_buffer - note_on_time)
                event = self._create_midi_event(note_on_time, note, note_on_velocity, duration, channel)
                self.process_incoming_midi(event)

    def dpf_run(self, nframes):
        # This is called every audio processing block.
        # `nframes` is the number of audio frames in this block.
        # NO BLOCKING OPERATIONS (like time.sleep()) ALLOWED HERE!

        # 1. Process scheduled note-offs for this block
        new_pending_note_offs = []
        for off_frame, note, channel in self.pending_note_offs_frames:
            # next_event_playback_time_host_frames is the start of the current *variation* in host time
            # off_frame is relative to the start of the *variation*
            absolute_off_frame_in_host_time = self.next_event_playback_time_host_frames + off_frame
            
            # current_block_start_host_frame can be obtained via DPF or tracked
            # For simplicity, assume self.current_block_frame_offset is the start of *this* block
            # relative to some DPF timeline, or just use relative logic.
            # This part is tricky and DPF specific. Let's simplify:
            # We need to know the host frame for the *start of this current run block*.
            # DPF usually provides this via self.plugin.get_current_frame() or similar at the start of run.
            # Let's assume self.host_frame_current_block_starts is set by C++ or DPF.

            # A simpler approach without absolute host frames initially for note-offs:
            # If the note-off's relative time (off_frame) has passed within the *variation's* timeline
            # AND it hasn't been sent yet. This is still not right.
            #
            # Correct frame-based logic for note-offs:
            # Note-offs are scheduled with their absolute host frame number.
            # current_host_frame_start_of_block = self.plugin.get_current_frame() # Hypothetical
            # current_host_frame_end_of_block = current_host_frame_start_of_block + nframes
            # for scheduled_off_host_frame, note, channel in self.pending_note_offs_frames:
            #    if current_host_frame_start_of_block <= scheduled_off_host_frame < current_host_frame_end_of_block:
            #        frame_offset_in_block = scheduled_off_host_frame - current_host_frame_start_of_block
            #        self.plugin.send_midi_event_at_frame([0x80 | channel, note, 0], frame_offset_in_block)
            #    elif scheduled_off_host_frame >= current_host_frame_end_of_block:
            #        new_pending_note_offs.append((scheduled_off_host_frame, note, channel)) # Keep for later
            # self.pending_note_offs_frames = new_pending_note_offs
            pass # Placeholder for robust DPF note-off scheduling

        # 2. Playback logic (ported from your playback_loop)
        if self.loop_is_active and self.playback_active and self.current_variation:
            host_frame_at_block_start = self.plugin.get_current_frame() # DPF specific API call!

            # Iterate through notes in current_variation
            while self.variation_playhead_index < len(self.current_variation):
                event_to_play = self.current_variation[self.variation_playhead_index]
                
                # event_to_play.timestamp is relative to start of variation (in seconds)
                # self.next_event_playback_time_host_frames is the host frame when the variation *should start*
                event_start_time_seconds = event_to_play.timestamp
                event_duration_seconds = event_to_play.duration

                event_start_offset_frames = int(event_start_time_seconds * self.sample_rate)
                event_note_on_host_frame = self.next_event_playback_time_host_frames + event_start_offset_frames
                
                event_note_off_host_frame = event_note_on_host_frame + int(event_duration_seconds * self.sample_rate)

                # Check if this note ON falls within the current processing block
                if host_frame_at_block_start <= event_note_on_host_frame < (host_frame_at_block_start + nframes):
                    frame_offset_in_block = event_note_on_host_frame - host_frame_at_block_start
                    
                    note_on_message = [0x90 | event_to_play.channel, event_to_play.note, event_to_play.velocity]
                    self.plugin.send_midi_event_at_frame(note_on_message, frame_offset_in_block)
                    # print(f"PY: NoteOn {event_to_play.note} at frame offset {frame_offset_in_block}")

                    # Schedule its note-off (add to self.pending_note_offs_frames with its *absolute host frame*)
                    # This needs to be processed in future dpf_run calls
                    self.pending_note_offs_frames.append((event_note_off_host_frame, event_to_play.note, event_to_play.channel))
                    
                    self.variation_playhead_index += 1
                elif event_note_on_host_frame >= (host_frame_at_block_start + nframes):
                    break # This note and subsequent notes are for future blocks
                else: # Note was in the past, missed it (should ideally not happen with correct logic)
                    self.variation_playhead_index += 1
            
            # If variation finished
            if self.variation_playhead_index >= len(self.current_variation):
                total_variation_duration_seconds = 0
                if self.current_variation:
                    # ... calculate total_variation_duration_seconds ...
                    last_event = self.current_variation[-1]
                    total_variation_duration_seconds = last_event.timestamp + last_event.duration

                self.generate_new_variation() # This resets variation_playhead_index
                if self.current_variation:
                    self.next_event_playback_time_host_frames += int(total_variation_duration_seconds * self.sample_rate)
                    # Resync if сильно drifted
                    if self.next_event_playback_time_host_frames < host_frame_at_block_start:
                         self.next_event_playback_time_host_frames = host_frame_at_block_start
                else:
                    self.playback_active = False
        
        # Process pending note offs that fall in *this* block (moved here for clarity)
        new_pending_note_offs = []
        host_frame_at_block_start = self.plugin.get_current_frame() # Get it again or pass it around
        for scheduled_off_host_frame, note, channel in self.pending_note_offs_frames:
            if host_frame_at_block_start <= scheduled_off_host_frame < (host_frame_at_block_start + nframes):
                frame_offset_in_block = scheduled_off_host_frame - host_frame_at_block_start
                self.plugin.send_midi_event_at_frame([0x80 | channel, note, 0], frame_offset_in_block)
                # print(f"PY: NoteOff {note} at frame offset {frame_offset_in_block}")
            elif scheduled_off_host_frame >= (host_frame_at_block_start + nframes):
                new_pending_note_offs.append((scheduled_off_host_frame, note, channel)) # Keep for later
        self.pending_note_offs_frames = new_pending_note_offs


    # --- Your existing methods, now part of this class, using `self.` ---
    # Make sure to use `self.` for all your instance variables (former globals)

    def _create_midi_event(self, timestamp, note, velocity, duration, channel):
        # This is your existing MidiEvent class, but you might instantiate it here
        # Or adapt your MidiEvent class to be defined within this script
        class LocalMidiEvent: # Define it locally or import if it's separate
             def __init__(self, timestamp, note, velocity, duration, channel=0):
                self.timestamp = timestamp; self.note = note; self.velocity = velocity;
                self.duration = duration; self.channel = channel
             def __repr__(self):
                return f"Event(t={self.timestamp:.2f}, n={self.note}, v={self.velocity}, d={self.duration:.2f}, ch={self.channel})"
        return LocalMidiEvent(timestamp, note, velocity, duration, channel)

    def process_incoming_midi(self, event): # event is now LocalMidiEvent
        self.retrospective_buffer.append(event)
        # ... (rest of your logic using self.retrospective_buffer, etc.) ...

    def set_active_loop_and_regenerate(self):
        # ... (use self.captured_loops_history, self.current_source_loop, etc.)
        # ... (this will call self.build_markov_models and self.generate_new_variation)
        if self.captured_loops_history:
            self.current_source_loop = self.captured_loops_history[-1]
            self.loop_is_active = True
            self.playback_active = True
            self.variation_playhead_index = 0
            # print(f"Active Loop set from history...")
            self.build_markov_models()
            self.generate_new_variation()
            if self.current_variation:
                self.next_event_playback_time_host_frames = self.plugin.get_current_frame() # Start now
        # ...

    def capture_new_loop_action(self):
        # ... (use self.retrospective_buffer, self.captured_loops_history)
        # ... (calls self.set_active_loop_and_regenerate)
        print("[MyMIDILooperDPF] Capture action triggered")
        if not self.retrospective_buffer: return

        newly_captured_segment = []
        # ... (normalize timestamps etc. from self.retrospective_buffer) ...
        temp_loop_from_buffer = list(self.retrospective_buffer)
        if not temp_loop_from_buffer: return
        first_event_timestamp = temp_loop_from_buffer[0].timestamp
        for event_data in temp_loop_from_buffer:
            normalized_event = self._create_midi_event( # Use helper
                timestamp=event_data.timestamp - first_event_timestamp,
                note=event_data.note, velocity=event_data.velocity,
                duration=event_data.duration, channel=event_data.channel
            )
            newly_captured_segment.append(normalized_event)

        if newly_captured_segment:
            self.captured_loops_history.append(newly_captured_segment)
            self.set_active_loop_and_regenerate()


    def undo_last_capture_action(self):
        # ... (use self.captured_loops_history)
        # ... (calls self.set_active_loop_and_regenerate)
        print("[MyMIDILooperDPF] Undo action triggered")
        if self.captured_loops_history:
            self.captured_loops_history.pop()
            self.set_active_loop_and_regenerate()


    def build_markov_models(self):
        # ... (use self.current_source_loop, self.pitch_markov_model, etc.) ...
        self.pitch_markov_model.clear(); self.rhythm_markov_model.clear(); self.duration_markov_model.clear()
        if len(self.current_source_loop) < 2: return
        # ... (rest of your existing build_markov_models logic using self. variables) ...
        for i in range(len(self.current_source_loop) - 1):
            # ...
            pass # Placeholder for your full logic
        print("[MyMIDILooperDPF] Markov models built.")


    def generate_new_variation(self):
        # ... (use self.current_source_loop, self.pitch_markov_model, etc. to populate self.current_variation)
        self.current_variation = []
        self.variation_playhead_index = 0
        if not self.current_source_loop: return
        # ... (rest of your existing generate_new_variation logic using self. variables) ...
        # Seed with first event
        first_original_event = self.current_source_loop[0]
        # ... create last_generated_event ...
        # self.current_variation.append(last_generated_event)
        # for i in range(1, num_notes_to_generate):
            # ...
        print("[MyMIDILooperDPF] New variation generated.")

    # You'll need your get_next_from_model as a method too
    def get_next_from_model(self, model, current_value, fallback_sequence, index, attribute_name):
        # ... (your existing logic) ...
        if current_value in model and model[current_value]:
            return random.choice(model[current_value])
        # ... rest of fallback ...
        return None

# IMPORTANT: DPF needs to know which class to instantiate.
# This is often done by convention (e.g., a class named 'DPFPluginClass')
# or specified in the C++ glue code.
# For now, ensure your C++ refers to "MyMIDILooperDPF".