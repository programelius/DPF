#ifndef RETROSPECTIVE_MIDI_BUFFER_H
#define RETROSPECTIVE_MIDI_BUFFER_H

#include "CustomMidiEvent.h"
#include <deque>
#include <vector>
#include <algorithm> 

class RetrospectiveMidiBuffer {
public:
    RetrospectiveMidiBuffer(double maxBufferTimeSeconds = 10.0);

    void addEvent(const LoopareliusUtils::CustomMidiEvent& event);
    std::vector<LoopareliusUtils::CustomMidiEvent> getNormalizedBufferedEvents() const;
    const std::deque<LoopareliusUtils::CustomMidiEvent>& getRawBuffer() const;
    void clear();
    size_t size() const;
    bool isEmpty() const;

private:
    void pruneBuffer();

    std::deque<LoopareliusUtils::CustomMidiEvent> buffer;
    double maxBufferTime; 
};

#endif // RETROSPECTIVE_MIDI_BUFFER_H