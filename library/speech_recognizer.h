#ifndef SPEECH_RECOGNIZER_H
#define SPEECH_RECOGNIZER_H

#include <string>
#include <vector>

class SpeechRecognizer {
public:
    SpeechRecognizer();
    bool initialize();
    std::string recognizeSpeech(const std::vector<int16_t>& audio_data);

private:
    std::string whisper_transcribe(const std::vector<int16_t>& audio_data);
};

#endif // SPEECH_RECOGNIZER_H