#include "vad.h"
#include <cmath>

VoiceActivityDetector::VoiceActivityDetector(float energy_threshold, int frame_size)
    : energy_threshold_(energy_threshold), frame_size_(frame_size) {}

bool VoiceActivityDetector::isSpeech(const std::vector<int16_t>& audio_frame) {
    if (audio_frame.size() != frame_size_) {
        return false; // Invalid frame size
    }

    float energy = calculateEnergy(audio_frame);
    return energy > energy_threshold_;
}

float VoiceActivityDetector::calculateEnergy(const std::vector<int16_t>& audio_frame) {
    float energy = 0.0f;
    for (int16_t sample : audio_frame) {
        energy += sample * sample;
    }
    return energy / frame_size_;
} 