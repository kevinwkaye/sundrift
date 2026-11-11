#include "../Source/PluginProcessor.h"
#include <JuceHeader.h>
#include <cmath>
#include <memory>
#include <iostream>

// Regression test: JUCE Component::setSize invokes resized() synchronously.
// This used to dereference bandButtons before they were constructed.
int main()
{
    juce::ScopedJuceInitialiser_GUI gui;
    SundriftAudioProcessor processor;
    processor.prepareToPlay(48000.0, 512);
    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
    if (!editor || editor->getWidth() != 1140 || editor->getHeight() != 742)
    {
        std::cerr << "Sundrift editor failed to initialise" << std::endl;
        return 1;
    }

    juce::AudioBuffer<float> audio(2, 512);
    juce::MidiBuffer midi;
    audio.clear();
    for (int i = 0; i < audio.getNumSamples(); ++i)
        audio.setSample(0, i, 0.08f * std::sin(juce::MathConstants<float>::twoPi * i * 330.0f / 48000.0f));
    audio.copyFrom(1, 0, audio, 0, 0, 512);

    processor.processBlock(audio, midi);
    for (int channel = 0; channel < 2; ++channel)
        for (int i = 0; i < audio.getNumSamples(); ++i)
            if (!std::isfinite(audio.getSample(channel, i)))
                return 2;

    for (int preset = 0; preset < processor.getNumPrograms(); ++preset)
    {
        processor.setCurrentProgram(preset);
        processor.processBlock(audio, midi);
    }

    editor.reset();
    processor.releaseResources();
    std::cout << "Sundrift editor construction, resize, presets and audio passed." << std::endl;
    return 0;
}
