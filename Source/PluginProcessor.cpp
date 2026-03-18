#include "PluginProcessor.h"
#include "PluginEditor.h"

ReverbAudioProcessor::ReverbAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

ReverbAudioProcessor::~ReverbAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout ReverbAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "algoType", "Algorithm",
        juce::StringArray{"Room", "Hall", "Chamber", "Plate", "Spring"}, 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "roomSize", "Room Size", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "damping", "Damping", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "wetLevel", "Wet Level", 0.0f, 1.0f, 0.33f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "dryLevel", "Dry Level", 0.0f, 1.0f, 0.4f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "width", "Width", 0.0f, 1.0f, 1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "adsrAttack",  "Attack",  0.001f, 2.0f,  0.01f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "adsrDecay",   "Decay",   0.001f, 2.0f,  0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "adsrSustain", "Sustain", 0.0f,   1.0f,  1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "adsrRelease", "Release", 0.01f,  10.0f, 0.5f));

    return layout;
}

const juce::String ReverbAudioProcessor::getName() const { return JucePlugin_Name; }
bool ReverbAudioProcessor::acceptsMidi() const { return false; }
bool ReverbAudioProcessor::producesMidi() const { return false; }
bool ReverbAudioProcessor::isMidiEffect() const { return false; }
double ReverbAudioProcessor::getTailLengthSeconds() const { return 4.0; }
int ReverbAudioProcessor::getNumPrograms() { return 1; }
int ReverbAudioProcessor::getCurrentProgram() { return 0; }
void ReverbAudioProcessor::setCurrentProgram(int) {}
const juce::String ReverbAudioProcessor::getProgramName(int) { return {}; }
void ReverbAudioProcessor::changeProgramName(int, const juce::String&) {}

void ReverbAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    reverb.prepare(spec);

    adsr.setSampleRate(sampleRate);
    adsr.reset();
    inputLevelSmooth = 0.0f;
    adsrTriggered = false;

    dryBuffer.setSize(static_cast<int>(spec.numChannels), samplesPerBlock);

    preDelayBuf = {};
    preDelayWritePos = 0;

    springBuf = {};
    springWritePos = 0;
    springLFOPhase = 0.0f;
}

void ReverbAudioProcessor::releaseResources()
{
    reverb.reset();
}

bool ReverbAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
}

// Maps user-facing knob values through algorithm-specific characteristic curves.
// Each algo shapes roomSize/damping/width to give it a distinct sonic identity,
// and returns the pre-delay (ms) appropriate for that space type.
void ReverbAudioProcessor::applyAlgoParams(int algoType, float roomSize, float damping,
                                            float width, juce::dsp::Reverb::Parameters& p,
                                            float& preDelayMs)
{
    switch (algoType)
    {
        case 0: // Room — small/medium, moderate damping, tight reflections
            p.roomSize  = 0.2f + roomSize * 0.4f;
            p.damping   = 0.3f + damping  * 0.5f;
            p.width     = 0.4f + width    * 0.5f;
            preDelayMs  = 10.0f + roomSize * 15.0f;  // 10–25 ms
            break;

        case 1: // Hall — large, bright, wide, long pre-delay
            p.roomSize  = 0.6f + roomSize * 0.4f;
            p.damping   = 0.05f + damping * 0.3f;
            p.width     = 0.7f + width    * 0.3f;
            preDelayMs  = 30.0f + roomSize * 40.0f;  // 30–70 ms
            break;

        case 2: // Chamber — medium, distinct reflections, slightly damped
            p.roomSize  = 0.4f + roomSize * 0.35f;
            p.damping   = 0.2f + damping  * 0.45f;
            p.width     = 0.3f + width    * 0.6f;
            preDelayMs  = 15.0f + roomSize * 20.0f;  // 15–35 ms
            break;

        case 3: // Plate — dense, smooth, bright, near-zero pre-delay
            p.roomSize  = 0.5f + roomSize * 0.4f;
            p.damping   = 0.05f + damping * 0.25f;
            p.width     = 0.8f + width    * 0.2f;
            preDelayMs  = 3.0f;
            break;

        case 4: // Spring — short, dark, narrow; modulation added separately
            p.roomSize  = 0.15f + roomSize * 0.3f;
            p.damping   = 0.55f + damping  * 0.3f;
            p.width     = 0.1f + width    * 0.35f;
            preDelayMs  = 0.0f;
            break;

        default:
            p.roomSize = roomSize;
            p.damping  = damping;
            p.width    = width;
            preDelayMs = 0.0f;
            break;
    }
}

// Adds the characteristic spring "boing" — a modulated feedback delay that
// mimics the helical coil resonance of a physical spring tank.
void ReverbAudioProcessor::processSpringModulation(juce::AudioBuffer<float>& buffer)
{
    const int numSamples  = buffer.getNumSamples();
    const int numChannels = juce::jmin(buffer.getNumChannels(), 2);

    const float lfoRate    = 2.5f;  // Hz — coil resonance wobble
    const float lfoInc     = lfoRate / static_cast<float>(currentSampleRate);
    const float baseDelay  = 0.040f; // 40 ms centre delay
    const float modDepth   = 0.008f; // ±8 ms modulation
    const float feedback   = 0.28f;
    const float mix        = 0.35f;

    for (int i = 0; i < numSamples; ++i)
    {
        const float lfo = std::sin(springLFOPhase * juce::MathConstants<float>::twoPi);
        const float delayMs = (baseDelay + lfo * modDepth) * static_cast<float>(currentSampleRate);
        const int delaySamples = juce::jlimit(1, kSpringBufSize - 1, static_cast<int>(delayMs));

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* data = buffer.getWritePointer(ch);
            const int readPos = (springWritePos - delaySamples + kSpringBufSize) % kSpringBufSize;
            const float delayed = springBuf[ch][readPos];
            springBuf[ch][springWritePos] = data[i] + delayed * feedback;
            data[i] += delayed * mix;
        }

        springWritePos = (springWritePos + 1) % kSpringBufSize;
        springLFOPhase += lfoInc;
        if (springLFOPhase >= 1.0f) springLFOPhase -= 1.0f;
    }
}

void ReverbAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int   algoType = static_cast<int>(apvts.getRawParameterValue("algoType")->load());
    const float roomSize = apvts.getRawParameterValue("roomSize")->load();
    const float damping  = apvts.getRawParameterValue("damping")->load();
    const float wetLevel = apvts.getRawParameterValue("wetLevel")->load();
    const float dryLevel = apvts.getRawParameterValue("dryLevel")->load();
    const float width    = apvts.getRawParameterValue("width")->load();

    // Update ADSR parameters
    juce::ADSR::Parameters adsrParams;
    adsrParams.attack  = apvts.getRawParameterValue("adsrAttack")->load();
    adsrParams.decay   = apvts.getRawParameterValue("adsrDecay")->load();
    adsrParams.sustain = apvts.getRawParameterValue("adsrSustain")->load();
    adsrParams.release = apvts.getRawParameterValue("adsrRelease")->load();
    adsr.setParameters(adsrParams);

    float preDelayMs = 0.0f;
    applyAlgoParams(algoType, roomSize, damping, width, reverbParams, preDelayMs);
    // wet=1 / dry=0 here — we handle the mix manually so the ADSR only shapes the wet signal
    reverbParams.wetLevel = 1.0f;
    reverbParams.dryLevel = 0.0f;
    reverb.setParameters(reverbParams);

    const int numSamples  = buffer.getNumSamples();
    const int numChannels = juce::jmin(buffer.getNumChannels(), 2);

    // Save dry signal before any processing
    dryBuffer.makeCopyOf(buffer);

    // Apply pre-delay to the reverb feed
    const int preDelaySamples = juce::jlimit(0, kPreDelayBufSize - 1,
        static_cast<int>(preDelayMs * 0.001 * currentSampleRate));

    if (preDelaySamples > 0)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            const int readPos = (preDelayWritePos - preDelaySamples + kPreDelayBufSize) % kPreDelayBufSize;
            for (int ch = 0; ch < numChannels; ++ch)
            {
                auto* data = buffer.getWritePointer(ch);
                preDelayBuf[ch][preDelayWritePos] = data[i];
                data[i] = preDelayBuf[ch][readPos];
            }
            preDelayWritePos = (preDelayWritePos + 1) % kPreDelayBufSize;
        }
    }

    // Reverb core — buffer now holds pure wet signal
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);

    // Spring coil modulation on the wet signal
    if (algoType == 4)
        processSpringModulation(buffer);

    // Apply ADSR to the wet signal and mix with dry.
    // Level follower: instant attack, ~100ms release — triggers ADSR from input level.
    for (int i = 0; i < numSamples; ++i)
    {
        // Compute input peak across channels
        float inputPeak = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            inputPeak = std::max(inputPeak, std::abs(dryBuffer.getSample(ch, i)));

        // Fast-attack / slow-release level follower
        inputLevelSmooth = std::max(inputPeak, inputLevelSmooth * 0.9998f);

        const bool shouldTrigger = (inputLevelSmooth > 0.001f); // ~-60 dB threshold
        if (shouldTrigger && !adsrTriggered)  { adsr.noteOn();  adsrTriggered = true;  }
        if (!shouldTrigger && adsrTriggered)  { adsr.noteOff(); adsrTriggered = false; }

        const float env = adsr.getNextSample();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float wet = buffer.getSample(ch, i) * wetLevel * env;
            const float dry = dryBuffer.getSample(ch, i) * dryLevel;
            buffer.setSample(ch, i, dry + wet);
        }
    }
}

bool ReverbAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* ReverbAudioProcessor::createEditor()
{
    return new ReverbAudioProcessorEditor(*this);
}

void ReverbAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ReverbAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ReverbAudioProcessor();
}
