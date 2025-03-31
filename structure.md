firmware/
├── CMakeLists.txt
├── main/
│   ├── main.cpp
│   ├── audio_hal.cpp        # Audio hardware abstraction
│   └── system_manager.cpp   # Basic system management
├── components/
│   ├── audio_processing/
│   │   ├── i2s_config.cpp
│   │   ├── audio_input.cpp
│   │   └── audio_output.cpp
│   ├── tts/
│   │   ├── piper_tts.cpp
│   │   └── text_processor.cpp
│   └── stt/  (Whisper.cpp)
│       ├── vad.cpp
│       └── speech_recognizer.cpp
└── sdkconfig
