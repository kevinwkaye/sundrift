#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <array>
#include <filesystem>
#include <algorithm>

const std::array<const char*,SundriftAudioProcessor::numKnobs> SundriftAudioProcessor::knobIDs{{
    "input","compress","focus","drive","edge","body","tone","phaseRate","phase","chorus","wobble",
    "echoTime","feedback","echoMix","space","decay","width","output"
}};
const std::array<const char*,SundriftAudioProcessor::numKnobs> SundriftAudioProcessor::knobLabels{{
    "INPUT","SQUEEZE","FOCUS","DRIVE","EDGE","BODY","TONE","RATE","PHASE","CHORUS","WOBBLE",
    "TIME","REPEATS","LEVEL","SPACE","DECAY","WIDTH","OUTPUT"
}};
namespace {
using FloatParam=juce::AudioParameterFloat;
using Normal=juce::NormalisableRange<float>;
std::unique_ptr<FloatParam> knob(const char* id,const char* label,float min,float max,float init,float step=0.001f){
    return std::make_unique<FloatParam>(juce::ParameterID{id,1},label,Normal(min,max,step),init);
}

}
juce::AudioProcessorValueTreeState::ParameterLayout SundriftAudioProcessor::parameterLayout(){
    APVTS::ParameterLayout p;
    p.add(knob("input","Input",-18,18,0,.1f));
    p.add(knob("compress","Compression",0,1,.26f));
    p.add(knob("focus","Wah Focus",0,1,.48f));
    p.add(knob("drive","Drive",0,1,.18f));
    p.add(knob("edge","Pick Edge",0,1,.20f));
    p.add(knob("body","Body",0,1,.53f));
    p.add(knob("tone","Tone",0,1,.57f));
    p.add(knob("phaseRate","Phase Rate",.04f,3.f,.31f));
    p.add(knob("phase","Phase Depth",0,1,.19f));
    p.add(knob("chorus","Chorus",0,1,.2f));
    p.add(knob("wobble","Tape Wobble",0,1,.16f));
    p.add(knob("echoTime","Echo Time",80.f,1000.f,390.f,1.f));
    p.add(knob("feedback","Echo Feedback",0,.75f,.27f));
    p.add(knob("echoMix","Echo Mix",0,1,.19f));
    p.add(knob("space","Reverb Mix",0,1,.37f));
    p.add(knob("decay","Reverb Decay",0,1,.63f));
    p.add(knob("width","Stereo Width",0,1,.73f));
    p.add(knob("output","Output",-24,12,-1.f,.1f));
    p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"ampMode",1},"Amplifier",
          juce::StringArray{"Built-in","NAM A2"},0));
    return p;
}
SundriftAudioProcessor::SundriftAudioProcessor():
    juce::AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true)
                              .withOutput("Output",juce::AudioChannelSet::stereo(),true)),
    state(*this,nullptr,"PARAMETERS",parameterLayout()) {
    setPreset(0);
}
SundriftAudioProcessor::~SundriftAudioProcessor(){cancelPendingUpdate();}
bool SundriftAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
    const auto in=l.getMainInputChannelSet(),out=l.getMainOutputChannelSet();
    return (in==juce::AudioChannelSet::mono() || in==juce::AudioChannelSet::stereo())
        && (out==juce::AudioChannelSet::mono() || out==juce::AudioChannelSet::stereo());
}
void SundriftAudioProcessor::prepareToPlay(double sampleRate,int samplesPerBlock){
    hostSampleRate.store(sampleRate);hostBufferSize.store(samplesPerBlock);
    engine.prepare(sampleRate,samplesPerBlock);
    // Loading models happens on the message thread, never in the audio callback.
    if(capturePath.isNotEmpty())triggerAsyncUpdate();
}
void SundriftAudioProcessor::releaseResources(){engine.reset();}
void SundriftAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi){
    juce::ignoreUnused(midi);
    juce::ScopedNoDenormals noDenormals;
    if(buffer.getNumChannels()<1)return;
    bool useNam=false;
    const float requestedInputDb=state.getRawParameterValue("input")->load();
    const int inputChannels=getTotalNumInputChannels();
    // In mono->stereo layouts JUCE provides the second output channel, initially empty.
    if(inputChannels==1 && buffer.getNumChannels()>1)
        buffer.copyFrom(1,0,buffer,0,0,buffer.getNumSamples());
#ifdef SUNDRIFT_WITH_NAM
    if(state.getRawParameterValue("ampMode")->load()>0.5f){
        if(auto active=std::atomic_load(&activeNam)){
            // With a neural capture, INPUT adjusts the drive hitting the model.
            // The conventional preamp performs gain after the model otherwise.
            const float gain=sundrift::dbGain(requestedInputDb);
            for(int channel=0;channel<std::min(buffer.getNumChannels(),2);++channel)
                buffer.applyGain(channel,0,buffer.getNumSamples(),gain);
            if(buffer.getNumChannels()>1)active->process(buffer.getWritePointer(0),buffer.getWritePointer(1),buffer.getNumSamples());
            else active->process(buffer.getWritePointer(0),nullptr,buffer.getNumSamples());
            useNam=true;
        }
    }
#endif
    sundrift::Params p;
    p.inputDb=useNam?0.f:requestedInputDb;
    p.compression=state.getRawParameterValue("compress")->load();
    p.focus=state.getRawParameterValue("focus")->load();
    // For a NAM capture, keep the inbuilt preamp extremely light to avoid double distortion.
    p.externalAmp=useNam;
    p.drive=useNam?0.f:state.getRawParameterValue("drive")->load();
    p.edge=state.getRawParameterValue("edge")->load();
    p.body=state.getRawParameterValue("body")->load();
    p.tone=state.getRawParameterValue("tone")->load();
    p.phaseRateHz=state.getRawParameterValue("phaseRate")->load();
    p.phase=state.getRawParameterValue("phase")->load();
    p.chorus=state.getRawParameterValue("chorus")->load();
    p.wobble=state.getRawParameterValue("wobble")->load();
    p.echoMs=state.getRawParameterValue("echoTime")->load();
    p.echoFeedback=state.getRawParameterValue("feedback")->load();
    p.echoMix=state.getRawParameterValue("echoMix")->load();
    p.space=state.getRawParameterValue("space")->load();
    p.decay=state.getRawParameterValue("decay")->load();
    p.width=state.getRawParameterValue("width")->load();
    p.outputDb=state.getRawParameterValue("output")->load();
    engine.setParameters(p);
    float* l=buffer.getWritePointer(0);
    float* r=(buffer.getNumChannels()>1? buffer.getWritePointer(1):nullptr);
    engine.process(l,r,buffer.getNumSamples());
    for(int c=getTotalNumOutputChannels();c<buffer.getNumChannels();++c)buffer.clear(c,0,buffer.getNumSamples());
}
void SundriftAudioProcessor::setPreset(int index){
    if(index<0 || index>=static_cast<int>(sundrift::presets.size()))return;
    currentPreset.store(index);
    for(int i=0;i<numKnobs;++i){
        auto* parameter=state.getParameter(knobIDs[static_cast<size_t>(i)]);
        if(parameter)parameter->setValueNotifyingHost(parameter->convertTo0to1(sundrift::presets[static_cast<size_t>(index)].values[static_cast<size_t>(i)]));
    }
}
void SundriftAudioProcessor::setCurrentProgram(int index){setPreset(index);}
const juce::String SundriftAudioProcessor::getProgramName(int index){return index>=0&&index<static_cast<int>(sundrift::presets.size())?sundrift::presets[static_cast<size_t>(index)].name:"";}
void SundriftAudioProcessor::getStateInformation(juce::MemoryBlock& destination){
    auto copy=state.copyState();
    {const juce::ScopedLock lock(textLock);copy.setProperty("namPath",capturePath,nullptr);}
    copy.setProperty("preset",currentPreset.load(),nullptr);
    if(auto xml=copy.createXml())copyXmlToBinary(*xml,destination);
}
void SundriftAudioProcessor::setStateInformation(const void* data,int bytes){
    if(auto xml=getXmlFromBinary(data,bytes)){
        if(xml->hasTagName(state.state.getType())){
            auto newState=juce::ValueTree::fromXml(*xml);
            const juce::String path=newState.getProperty("namPath").toString();
            currentPreset.store(juce::jlimit(0,static_cast<int>(sundrift::presets.size())-1,int(newState.getProperty("preset",0))));
            state.replaceState(newState);
            {const juce::ScopedLock lock(textLock);capturePath=path;}
            if(path.isNotEmpty())triggerAsyncUpdate();
        }
    }
}
juce::AudioProcessorEditor* SundriftAudioProcessor::createEditor(){return new SundriftAudioProcessorEditor(*this);}
void SundriftAudioProcessor::selectNamFile(const juce::File& file){
    {const juce::ScopedLock lock(textLock);capturePath=file.getFullPathName();}
    stageNam(file);
}
void SundriftAudioProcessor::handleAsyncUpdate(){
    juce::String path;
    {const juce::ScopedLock lock(textLock);path=capturePath;}
    if(path.isNotEmpty())stageNam(juce::File(path));
}
void SundriftAudioProcessor::stageNam(const juce::File& file){
#ifdef SUNDRIFT_WITH_NAM
    try {
        auto pending=std::make_shared<NamPair>(file,hostSampleRate.load(),hostBufferSize.load());
        auto previous=std::atomic_exchange(&activeNam,std::move(pending));
        if(previous)retiredNam.push_back(std::move(previous));
        const juce::ScopedLock lock(textLock);
        captureStatus="NAM CAPTURE LOADED";
    }catch(const std::exception& e){
        // Preserve the previously working capture if this new file is invalid.
        const juce::ScopedLock lock(textLock);
        captureStatus="NAM ERROR: "+juce::String(e.what());
    }
#else
    juce::ignoreUnused(file);
    const juce::ScopedLock lock(textLock);
    captureStatus="A2 SUPPORT NOT ENABLED IN THIS BUILD";
#endif
}
juce::String SundriftAudioProcessor::namFilename() const {
    const juce::ScopedLock lock(textLock);
    return capturePath.isEmpty()?"NO CAPTURE":juce::File(capturePath).getFileName();
}
juce::String SundriftAudioProcessor::namStatusText() const {
#ifdef SUNDRIFT_WITH_NAM
    if(state.getRawParameterValue("ampMode")->load()>0.5f && !std::atomic_load(&activeNam))
        return "LOAD .NAM / USING BUILT-IN AMP";
#endif
    const juce::ScopedLock lock(textLock);
    return captureStatus;
}
bool SundriftAudioProcessor::namAvailable() const {
#ifdef SUNDRIFT_WITH_NAM
    return true;
#else
    return false;
#endif
}
#ifdef SUNDRIFT_WITH_NAM
SundriftAudioProcessor::NamPair::NamPair(const juce::File& file,double sr,int maxBlock) {
    blockLimit=std::max(1,maxBlock);
    scratchIn.resize(static_cast<size_t>(blockLimit));
    scratchOut.resize(static_cast<size_t>(blockLimit));
    if(!file.existsAsFile() || file.getFileExtension().toLowerCase()!=".nam")
        throw std::runtime_error("Select a valid .nam file");
    const auto path=std::filesystem::u8path(file.getFullPathName().toStdString());
    left=nam::get_dsp(path);
    right=nam::get_dsp(path);
    if(left->NumInputChannels()!=1 || left->NumOutputChannels()!=1)
        throw std::runtime_error("Only mono amp captures supported");
    const double expected=left->GetExpectedSampleRate();
    const double required=expected<=0.0?48000.0:expected;
    if(std::abs(required-sr)>1.0)
        throw std::runtime_error("Set your DAW to "+std::to_string(int(required))+" Hz for this capture");
    left->ResetAndPrewarm(sr,blockLimit);
    right->ResetAndPrewarm(sr,blockLimit);
}
void SundriftAudioProcessor::NamPair::process(float* l,float* r,int n){
    // Chunk safely if a host unexpectedly supplies buffers larger than prepareToPlay().
    for(int offset=0;offset<n;offset+=blockLimit){
        const int frames=std::min(blockLimit,n-offset);
        // Use non-aliased preallocated buffers: some models don't promise
        // safe in-place processing. No allocations occur in processBlock.
        std::copy_n(l+offset,frames,scratchIn.begin());
        NAM_SAMPLE* inputs[1]={scratchIn.data()}; NAM_SAMPLE* outputs[1]={scratchOut.data()};
        left->process(inputs,outputs,frames);
        std::copy_n(scratchOut.begin(),frames,l+offset);
        if(r){
            std::copy_n(r+offset,frames,scratchIn.begin());
            right->process(inputs,outputs,frames);
            std::copy_n(scratchOut.begin(),frames,r+offset);
        }
    }
}
#endif
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new SundriftAudioProcessor();}
