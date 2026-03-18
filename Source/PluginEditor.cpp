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
      algoAttachment    (p.apvts, "algoType", algoBox),
      roomSizeAttachment(p.apvts, "roomSize",  roomSizeSlider),
      dampingAttachment  (p.apvts, "damping",   dampingSlider),
      wetLevelAttachment (p.apvts, "wetLevel",  wetLevelSlider),
      dryLevelAttachment (p.apvts, "dryLevel",  dryLevelSlider),
      widthAttachment    (p.apvts, "width",     widthSlider),
      attackAttachment  (p.apvts, "adsrAttack",  attackSlider),
      decayAttachment   (p.apvts, "adsrDecay",   decaySlider),
      sustainAttachment (p.apvts, "adsrSustain", sustainSlider),
      releaseAttachment (p.apvts, "adsrRelease", releaseSlider)
{
    algoBox.addItemList({"Room", "Hall", "Chamber", "Plate", "Spring"}, 1);
    algoLabel.setText("Algorithm", juce::dontSendNotification);
    algoLabel.setJustificationType(juce::Justification::centredRight);
    algoLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(algoBox);
    addAndMakeVisible(algoLabel);

    setupSlider(roomSizeSlider,  roomSizeLabel,  "Room Size", this);
    setupSlider(dampingSlider,   dampingLabel,   "Damping",   this);
    setupSlider(wetLevelSlider,  wetLevelLabel,  "Wet",       this);
    setupSlider(dryLevelSlider,  dryLevelLabel,  "Dry",       this);
    setupSlider(widthSlider,     widthLabel,     "Width",     this);

    setupSlider(attackSlider,  attackLabel,  "Attack",  this);
    setupSlider(decaySlider,   decayLabel,   "Decay",   this);
    setupSlider(sustainSlider, sustainLabel, "Sustain", this);
    setupSlider(releaseSlider, releaseLabel, "Release", this);

    adsrSectionLabel.setText("ADSR", juce::dontSendNotification);
    adsrSectionLabel.setJustificationType(juce::Justification::centredLeft);
    adsrSectionLabel.setColour(juce::Label::textColourId, juce::Colour(0xffaaaacc));
    adsrSectionLabel.setFont(juce::Font(13.0f, juce::Font::bold));
    addAndMakeVisible(adsrSectionLabel);

    setSize(500, 400);
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
    const int titleH   = 30;
    const int comboH   = 26;
    const int padding  = 8;
    const int labelW   = 75;
    const int sectionH = 18;
    const int labelH   = 20;
    const int knobH    = 150;

    // Algorithm selector row
    algoLabel.setBounds(padding, titleH + padding, labelW, comboH);
    algoBox.setBounds(padding + labelW + 4, titleH + padding,
                      getWidth() - padding * 2 - labelW - 4, comboH);

    // Reverb knob row
    const int reverbTop = titleH + padding + comboH + padding;
    const int knobW5    = getWidth() / 5;

    auto layout5 = [&](juce::Slider& s, juce::Label& l, int col) {
        l.setBounds(col * knobW5, reverbTop, knobW5, labelH);
        s.setBounds(col * knobW5, reverbTop + labelH, knobW5, knobH);
    };

    layout5(roomSizeSlider, roomSizeLabel, 0);
    layout5(dampingSlider,  dampingLabel,  1);
    layout5(wetLevelSlider, wetLevelLabel, 2);
    layout5(dryLevelSlider, dryLevelLabel, 3);
    layout5(widthSlider,    widthLabel,    4);

    // ADSR section
    const int adsrSectionTop = reverbTop + labelH + knobH + padding;
    adsrSectionLabel.setBounds(padding, adsrSectionTop, 60, sectionH);

    const int adsrTop = adsrSectionTop + sectionH;
    const int knobW4  = getWidth() / 4;

    auto layout4 = [&](juce::Slider& s, juce::Label& l, int col) {
        l.setBounds(col * knobW4, adsrTop, knobW4, labelH);
        s.setBounds(col * knobW4, adsrTop + labelH, knobW4, knobH);
    };

    layout4(attackSlider,  attackLabel,  0);
    layout4(decaySlider,   decayLabel,   1);
    layout4(sustainSlider, sustainLabel, 2);
    layout4(releaseSlider, releaseLabel, 3);
}
