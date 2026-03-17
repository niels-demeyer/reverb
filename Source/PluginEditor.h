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

    juce::Slider roomSizeSlider, dampingSlider, wetLevelSlider, dryLevelSlider, widthSlider;
    juce::Label  roomSizeLabel, dampingLabel, wetLevelLabel, dryLevelLabel, widthLabel;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    Attachment roomSizeAttachment, dampingAttachment, wetLevelAttachment,
               dryLevelAttachment, widthAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbAudioProcessorEditor)
};
