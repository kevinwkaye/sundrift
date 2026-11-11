#pragma once
#include <JuceHeader.h>
#include "ToneEngine.h"
#include "Presets.h"
#include <atomic>
#include <memory>
#include <string>
#include <vector>

#ifdef SUNDRIFT_WITH_NAM
#include "NAM/get_dsp.h"
#endif

class SundriftAudioProcessor final : public juce::AudioProcessor, private juce::AsyncUpdater {
public:
    SundriftAudioProcessor();
    ~SundriftAudioProcessor() override;

    void prepareToPlay(double sampleRate,int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override {return true;}
    const juce::String getName() const override {return "Sundrift";}
    bool acceptsMidi() const override {return false;}
    bool producesMidi() const override {return false;}
    bool isMidiEffect() const override {return false;}
    double getTailLengthSeconds() const override {return 8.0;}
    int getNumPrograms() override {return static_cast<int>(sundrift::presets.size());}
    int getCurrentProgram() override {return currentPreset;}
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*,int) override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;

    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS state;
    void setPreset(int presetIndex);
    int currentPresetIndex() const {return currentPreset.load();}
    int currentBandIndex() const {return sundrift::bandForPreset(currentPreset.load());}
    void selectNamFile(const juce::File& file);
    juce::String namStatusText() const;
    juce::String namFilename() const;
    bool namAvailable() const;
    static juce::AudioProcessorValueTreeState::ParameterLayout parameterLayout();
    static constexpr int numKnobs = 18;
    static const std::array<const char*,numKnobs> knobIDs;
    static const std::array<const char*,numKnobs> knobLabels;
private:
    void handleAsyncUpdate() override;
    void stageNam(const juce::File& file);
    sundrift::Engine engine;
    std::atomic<double> hostSampleRate{48000.0};
    std::atomic<int> hostBufferSize{512};
    std::atomic<int> currentPreset{0};
    juce::String capturePath;
    juce::String captureStatus{"BUILT-IN AMP READY"};
    mutable juce::CriticalSection textLock;
#ifdef SUNDRIFT_WITH_NAM
    struct NamPair {
        std::unique_ptr<nam::DSP> left, right;
        int blockLimit=512;
        std::vector<NAM_SAMPLE> scratchIn, scratchOut;
        explicit NamPair(const juce::File& file,double sampleRate,int maxBlock);
        void process(float* l,float* r,int n);
    };
    std::shared_ptr<NamPair> activeNam;
    // Keep superseded captures alive until teardown, so they cannot be destroyed
    // on the audio thread when a new model replaces one mid-process.
    std::vector<std::shared_ptr<NamPair>> retiredNam;
#endif
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SundriftAudioProcessor)
};
