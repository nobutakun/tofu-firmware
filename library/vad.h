#ifndef VAD_H
#define VAD_H

#include <vector>
#include <cstdint>

class VoiceActivityDetector {
public:
    VoiceActivityDetector(float energy_threshold, int frame_size);

    bool isSpeech(const std::vector<std::int16_t>& audio_frame);

private:
    float calculateEnergy(const std::vector<std::int16_t>& audio_frame);

    float energy_threshold_;
    int frame_size_;
};

#endif // VAD_H 