#pragma once

#include "PluginProcessor.h"

class ReverbAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit ReverbAudioProcessorEditor(ReverbAudioProcessor&);
    ~ReverbAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    ReverbAudioProcessor& processor;

    juce::ComboBox algoBox;
    juce::Label    algoLabel;

    juce::Slider roomSizeSlider, dampingSlider, wetLevelSlider, dryLevelSlider, widthSlider;
    juce::Label  roomSizeLabel, dampingLabel, wetLevelLabel, dryLevelLabel, widthLabel;

    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Label  attackLabel, decayLabel, sustainLabel, releaseLabel;
    juce::Label  adsrSectionLabel;

    using Attachment      = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    ComboAttachment algoAttachment;
    Attachment roomSizeAttachment, dampingAttachment, wetLevelAttachment,
               dryLevelAttachment, widthAttachment;
    Attachment attackAttachment, decayAttachment, sustainAttachment, releaseAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbAudioProcessorEditor)
};
