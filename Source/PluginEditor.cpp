#include "PluginEditor.h"

static void setupSlider(juce::Slider& s, juce::Label& l, const juce::String& name,
                        juce::Component* parent)
{
    s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    parent->addAndMakeVisible(s);

    l.setText(name, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    parent->addAndMakeVisible(l);
}

ReverbAudioProcessorEditor::ReverbAudioProcessorEditor(ReverbAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processor(p),
      roomSizeAttachment(p.apvts, "roomSize",  roomSizeSlider),
      dampingAttachment  (p.apvts, "damping",   dampingSlider),
      wetLevelAttachment (p.apvts, "wetLevel",  wetLevelSlider),
      dryLevelAttachment (p.apvts, "dryLevel",  dryLevelSlider),
      widthAttachment    (p.apvts, "width",     widthSlider)
{
    setupSlider(roomSizeSlider,  roomSizeLabel,  "Room Size", this);
    setupSlider(dampingSlider,   dampingLabel,   "Damping",   this);
    setupSlider(wetLevelSlider,  wetLevelLabel,  "Wet",       this);
    setupSlider(dryLevelSlider,  dryLevelLabel,  "Dry",       this);
    setupSlider(widthSlider,     widthLabel,     "Width",     this);

    setSize(500, 200);
}

ReverbAudioProcessorEditor::~ReverbAudioProcessorEditor() {}

void ReverbAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1e1e2e));
    g.setColour(juce::Colours::white);
    g.setFont(18.0f);
    g.drawFittedText("Reverb", getLocalBounds().removeFromTop(30), juce::Justification::centred, 1);
}

void ReverbAudioProcessorEditor::resized()
{
    const int knobW = getWidth() / 5;
    const int labelH = 20;
    const int knobH = getHeight() - labelH - 30;
    const int top = 30;

    auto layout = [&](juce::Slider& s, juce::Label& l, int col) {
        l.setBounds(col * knobW, top, knobW, labelH);
        s.setBounds(col * knobW, top + labelH, knobW, knobH);
    };

    layout(roomSizeSlider, roomSizeLabel, 0);
    layout(dampingSlider,  dampingLabel,  1);
    layout(wetLevelSlider, wetLevelLabel, 2);
    layout(dryLevelSlider, dryLevelLabel, 3);
    layout(widthSlider,    widthLabel,    4);
}
