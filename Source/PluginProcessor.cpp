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

    return layout;
}

const juce::String ReverbAudioProcessor::getName() const { return JucePlugin_Name; }
bool ReverbAudioProcessor::acceptsMidi() const { return false; }
bool ReverbAudioProcessor::producesMidi() const { return false; }
bool ReverbAudioProcessor::isMidiEffect() const { return false; }
double ReverbAudioProcessor::getTailLengthSeconds() const { return 2.0; }
int ReverbAudioProcessor::getNumPrograms() { return 1; }
int ReverbAudioProcessor::getCurrentProgram() { return 0; }
void ReverbAudioProcessor::setCurrentProgram(int) {}
const juce::String ReverbAudioProcessor::getProgramName(int) { return {}; }
void ReverbAudioProcessor::changeProgramName(int, const juce::String&) {}

void ReverbAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    reverb.prepare(spec);
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

void ReverbAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    reverbParams.roomSize  = apvts.getRawParameterValue("roomSize")->load();
    reverbParams.damping   = apvts.getRawParameterValue("damping")->load();
    reverbParams.wetLevel  = apvts.getRawParameterValue("wetLevel")->load();
    reverbParams.dryLevel  = apvts.getRawParameterValue("dryLevel")->load();
    reverbParams.width     = apvts.getRawParameterValue("width")->load();
    reverb.setParameters(reverbParams);

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);
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
