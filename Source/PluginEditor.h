#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <array>
#include <memory>

// Sundrift shares Murmur's Flip Fella visual language: black metal, tactile clock
// dials and mechanical split-flap cards. No fonts or images from Murmur are needed.
class SundriftLookAndFeel : public juce::LookAndFeel_V4 {
public:
    SundriftLookAndFeel();
    void setAccent(juce::Colour next) { accent=next; }
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
private:
    juce::Colour accent {0xffe9ad54};
};

class BandButton final : public juce::Button {
public:
    explicit BandButton(int band);
    void setSelected(bool value) { if (active!=value) {active=value;repaint();} }
    void paintButton(juce::Graphics&, bool highlighted, bool down) override;
private:
    int index=0;
    bool active=false;
};

class SplitFlapDisplay final : public juce::Component {
public:
    void setContent(const juce::String& text, const juce::String& desc, juce::Colour colour);
    void advance(float seconds);
    void paint(juce::Graphics&) override;
private:
    juce::String title="SUNDAY FORECAST", previous="SUNDAY FORECAST", description;
    juce::Colour accent{0xffe9ad54};
    float flip=1.f;
};

class SundriftAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit SundriftAudioProcessorEditor(SundriftAudioProcessor&);
    ~SundriftAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    struct Knob {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
    };
    void timerCallback() override;
    void syncPresetControls();
    SundriftAudioProcessor& processor;
    SundriftLookAndFeel theme;
    std::array<Knob, SundriftAudioProcessor::numKnobs> knobs;
    std::array<std::unique_ptr<BandButton>, 4> bandButtons;
    std::array<int, 4> lastBandPresets {{0, 3, 6, 9}};
    SplitFlapDisplay nameDisplay;
    juce::ComboBox presetChoice, ampMode;
    juce::Label presetLabel, ampLabel, captureState, versionMark;
    juce::TextButton loadModel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> ampAttachment;
    std::unique_ptr<juce::FileChooser> chooser;
    int displayedPreset=-1;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SundriftAudioProcessorEditor)
};
