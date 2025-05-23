#include "PluginEditor.h"

PluginEditor::PluginEditor (PressedProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    auto& params = processor.apvts;

    auto setupSlider = [this](juce::Slider& s) {
        s.setSliderStyle(juce::Slider::Rotary);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        addAndMakeVisible(s);
    };

    setupSlider(thresholdSlider);
    setupSlider(ratioSlider);
    setupSlider(attackSlider);
    setupSlider(releaseSlider);
    setupSlider(makeupGainSlider);

    thresholdAttachment = std::make_unique<SliderAttachment>(processor.apvts, "threshold", thresholdSlider);
    ratioAttachment = std::make_unique<SliderAttachment>(processor.apvts, "ratio", ratioSlider);
    attackAttachment = std::make_unique<SliderAttachment>(processor.apvts, "attack", attackSlider);
    releaseAttachment = std::make_unique<SliderAttachment>(processor.apvts, "release", releaseSlider);
    makeupAttachment = std::make_unique<SliderAttachment>(processor.apvts, "makeupGain", makeupGainSlider);

    addAndMakeVisible (thresholdLabel);
    thresholdLabel.setText ("Threshold", juce::dontSendNotification);

    addAndMakeVisible (attackLabel);
    attackLabel.setText ("Attack", juce::dontSendNotification);

    addAndMakeVisible (releaseLabel);
    releaseLabel.setText ("Release", juce::dontSendNotification);

    addAndMakeVisible (ratioLabel);
    ratioLabel.setText ("Ratio", juce::dontSendNotification);

    addAndMakeVisible (makeupLabel);
    makeupLabel.setText ("Makeup Gain", juce::dontSendNotification);

    setSize (500, 300);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void PluginEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    auto row = area.removeFromTop(220);

    auto sliderWidth = row.getWidth() / 5;

    thresholdSlider.setBounds(row.removeFromLeft(sliderWidth));
    ratioSlider.setBounds(row.removeFromLeft(sliderWidth));
    attackSlider.setBounds(row.removeFromLeft(sliderWidth));
    releaseSlider.setBounds(row.removeFromLeft(sliderWidth));
    makeupGainSlider.setBounds(row.removeFromLeft(sliderWidth));

    auto labelRow = getLocalBounds().reduced(20).removeFromTop(180 + 0).removeFromTop(30);
    auto labelWidth = labelRow.getWidth() / 5;

    thresholdLabel.setBounds(labelRow.removeFromLeft(labelWidth));
    ratioLabel.setBounds(labelRow.removeFromLeft(labelWidth));
    attackLabel.setBounds(labelRow.removeFromLeft(labelWidth));
    releaseLabel.setBounds(labelRow.removeFromLeft(labelWidth));
    makeupLabel.setBounds(labelRow.removeFromLeft(labelWidth));


}