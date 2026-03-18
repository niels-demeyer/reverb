#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>

class ReverbAudioProcessor : public juce::AudioProcessor
{
public:
    ReverbAudioProcessor();
    ~ReverbAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;

    // Pre-delay: up to ~104ms at 96kHz
    static constexpr int kPreDelayBufSize = 10000;
    std::array<std::array<float, kPreDelayBufSize>, 2> preDelayBuf {};
    int preDelayWritePos = 0;

    // Spring reverb: modulated feedback delay
    static constexpr int kSpringBufSize = 16384;
    std::array<std::array<float, kSpringBufSize>, 2> springBuf {};
    int springWritePos = 0;
    float springLFOPhase = 0.0f;

    // ADSR envelope on the wet signal
    juce::ADSR adsr;
    float inputLevelSmooth = 0.0f;  // input level follower for auto-trigger
    bool adsrTriggered = false;
    juce::AudioBuffer<float> dryBuffer;  // pre-reverb signal for manual wet/dry mix

    double currentSampleRate = 44100.0;

    void applyAlgoParams(int algoType, float roomSize, float damping, float width,
                         juce::dsp::Reverb::Parameters& p, float& preDelayMs);
    void processSpringModulation(juce::AudioBuffer<float>& buffer);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbAudioProcessor)
};
