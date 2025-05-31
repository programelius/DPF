#ifndef MIDI_EVENT_H
#define MIDI_EVENT_H

#include <deque>   // For the retrospective buffer
#include <vector>  // For captured loops, variations
#include <string>  // For debugging/printing
#include <iomanip> // For std::fixed, std.setprecision
#include <sstream> // For string conversion in toString

struct MidiEvent {
    double timestamp;  // Absolute time of note_on (e.g., seconds from start, or host frames)
    int noteNumber;    // MIDI note number (0-127)
    int velocity;      // MIDI velocity (0-127)
    double duration;   // Duration of the note (in same units as timestamp)
    int channel;       // MIDI channel (0-15)

    MidiEvent(double ts, int nn, int vel, double dur, int ch = 0)
        : timestamp(ts), noteNumber(nn), velocity(vel), duration(dur), channel(ch) {}

    // For easy printing/debugging
    std::string toString() const {
        std::ostringstream oss;
        oss << "Event(t=" << std::fixed << std::setprecision(2) << timestamp
            << ", n=" << noteNumber
            << ", v=" << velocity
            << ", d=" << std::fixed << std::setprecision(2) << duration
            << ", ch=" << channel << ")";
        return oss.str();
    }
};

#endif // MIDI_EVENT_H