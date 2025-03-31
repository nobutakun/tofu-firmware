idf_component_register(
    SRCS 
        "audio_input.test.cpp"
    INCLUDE_DIRS 
        "."
        "../components/audio_processing"
        "../components/audio_processing/library"
    REQUIRES 
        unity
        audio_processing
        driver
        arduino-esp32
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE
    "UNITY_INCLUDE_CONFIG_H"
) 