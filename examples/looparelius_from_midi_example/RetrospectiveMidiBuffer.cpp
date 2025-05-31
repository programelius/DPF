#include "RetrospectiveMidiBuffer.h"
#include <iostream> 

RetrospectiveMidiBuffer::RetrospectiveMidiBuffer(double maxBufferTimeSeconds)
    : maxBufferTime(maxBufferTimeSeconds) {}

void RetrospectiveMidiBuffer::addEvent(const LoopareliusUtils::CustomMidiEvent& event) {
    buffer.push_back(event);
    pruneBuffer();
}

std::vector<LoopareliusUtils::CustomMidiEvent> RetrospectiveMidiBuffer::getNormalizedBufferedEvents() const {
    std::vector<LoopareliusUtils::CustomMidiEvent> normalizedEvents;
    if (buffer.empty()) {
        return normalizedEvents;
    }

    double firstEventTimestamp = buffer.front().timestamp;
    for (const auto& event : buffer) {
        normalizedEvents.emplace_back(
            event.timestamp - firstEventTimestamp,
            event.noteNumber,
            event.velocity,
            event.duration,
            event.channel
        );
    }
    return normalizedEvents;
}

const std::deque<LoopareliusUtils::CustomMidiEvent>& RetrospectiveMidiBuffer::getRawBuffer() const {
    return buffer;
}

void RetrospectiveMidiBuffer::clear() {
    buffer.clear();
}

size_t RetrospectiveMidiBuffer::size() const {
    return buffer.size();
}

bool RetrospectiveMidiBuffer::isEmpty() const {
    return buffer.empty();
}

void RetrospectiveMidiBuffer::pruneBuffer() {
    if (buffer.size() < 2) {
        return;
    }

    double currentTimeSpan = buffer.back().timestamp - buffer.front().timestamp;
    while (currentTimeSpan > maxBufferTime && !buffer.empty()) {
        buffer.pop_front();
        if (buffer.size() < 2) {
            break;
        }
        currentTimeSpan = buffer.back().timestamp - buffer.front().timestamp;
    }
}