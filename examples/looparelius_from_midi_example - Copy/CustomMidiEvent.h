#ifndef CUSTOM_MIDI_EVENT_H
#define CUSTOM_MIDI_EVENT_H

#include <string>
#include <sstream>
#include <iomanip>

namespace LoopareliusUtils {

struct CustomMidiEvent {
    double timestamp;
    int noteNumber;
    int velocity;
    double duration;
    int channel;

    CustomMidiEvent(double ts = 0.0, int nn = 0, int vel = 0, double dur = 0.0, int ch = 0)
        : timestamp(ts), noteNumber(nn), velocity(vel), duration(dur), channel(ch) {}

    std::string toString() const {
        std::ostringstream oss;
        oss << "Event(t=" << std::fixed << std::setprecision(3) << timestamp
            << ", n=" << noteNumber
            << ", v=" << velocity
            << ", d=" << std::fixed << std::setprecision(3) << duration
            << ", ch=" << channel << ")";
        return oss.str();
    }
};

} 
#endif // CUSTOM_MIDI_EVENT_H