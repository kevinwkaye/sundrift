#pragma once

// SUND RIFT: A dependency-free, allocation-free-in-process stereo guitar FX engine.
// Standard C++17. JUCE is used only at the plugin boundary.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

namespace sundrift {
constexpr float pi = 3.14159265358979323846f;
inline float clamp(float x, float lo, float hi) { return std::max(lo, std::min(hi, x)); }
inline float dbGain(float db) { return std::pow(10.f, db / 20.f); }

struct Params {
    float inputDb = 0.f;
    bool externalAmp = false;
    float compression = 0.26f;
    float focus = 0.48f;
    float drive = 0.18f;
    float edge = 0.20f; // upper-mid presence and parallel touch-sensitive grit
    float body = 0.53f;
    float tone = 0.57f;
    float phaseRateHz = 0.31f;
    float phase = 0.19f;
    float chorus = 0.20f;
    float wobble = 0.16f;
    float echoMs = 390.f;
    float echoFeedback = 0.27f;
    float echoMix = 0.19f;
    float space = 0.37f;
    float decay = 0.63f;
    float width = 0.73f;
    float outputDb = -1.0f;
};

class OnePole {
public:
    void setLowpass(float cutoff, float sr) { a = std::exp(-2.f * pi * clamp(cutoff, 1.f, 0.49f*sr) / sr); }
    float run(float x) { z = (1.f-a)*x + a*z; if (std::abs(z)<1.e-20f) z=0.f; return z; }
    void clear() { z=0.f; }
private:
    float z=0.f, a=0.5f;
};

class BiquadPeak {
public:
    void configure(float hz, float q, float gainDb, float sr) {
        const float a = std::pow(10.f, gainDb/40.f);
        const float w = 2.f*pi*clamp(hz, 20.f, .45f*sr)/sr;
        const float alpha=std::sin(w)/(2.f*q);
        const float inv = 1.f/(1.f+alpha/a);
        b0=(1.f+alpha*a)*inv;
        b1=(-2.f*std::cos(w))*inv;
        b2=(1.f-alpha*a)*inv;
        a1=(-2.f*std::cos(w))*inv;
        a2=(1.f-alpha/a)*inv;
    }
    float run(float x) {
        float y=b0*x+z1;
        z1=b1*x-a1*y+z2;
        z2=b2*x-a2*y;
        if (std::abs(z1)<1e-18f) z1=0.f;
        if (std::abs(z2)<1e-18f) z2=0.f;
        return y;
    }
    void clear(){z1=z2=0.f;}
private:
    float b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;
};

class AllpassStage {
public:
    float tick(float x, float c) { const float y = -c*x+z; z=x+c*y; return y; }
    void clear(){z=0.f;}
private:
    float z=0.f;
};

class Ring {
public:
    void resize(std::size_t n) { samples.assign(std::max<std::size_t>(n,2),0.f); pos=0; }
    void clear(){std::fill(samples.begin(),samples.end(),0.f);pos=0;}
    void push(float x) { samples[pos] = x; if (++pos >= samples.size()) pos=0; }
    float tap(float delaySamples) const {
        // pos is the NEXT write index, so delay=1 reads the most recent sample.
        // Keep the wrap in integer arithmetic: float(pos)-delay can round up to
        // exactly samples.size() near a zero crossing and cause an out-of-bounds
        // read after float wraparound. This is important for slow stereo modulation.
        const float d=clamp(delaySamples,1.f,static_cast<float>(samples.size()-2));
        const auto whole=static_cast<std::size_t>(d);
        const float frac=d-static_cast<float>(whole);
        const auto newest=(pos+samples.size()-whole)%samples.size();
        const auto older=(newest+samples.size()-1)%samples.size();
        return samples[newest]+frac*(samples[older]-samples[newest]);
    }
    std::size_t size() const {return samples.size();}
private:
    std::vector<float> samples;
    std::size_t pos=0;
};

class Comb {
public:
    void prepare(std::size_t n){ buf.resize(n); damp.clear(); damp.setLowpass(4400.f,44100.f); }
    void setDamping(float sr){damp.setLowpass(3600.f,sr);}
    float tick(float x,float feedback){ float y=buf.tap(static_cast<float>(buf.size()-2)); buf.push(x+damp.run(y)*feedback); return y; }
    void clear(){buf.clear();damp.clear();}
private:
    Ring buf; OnePole damp;
};

class Diffuser {
public:
    void prepare(std::size_t n){buf.resize(n);}
    float tick(float x){const float v=buf.tap(static_cast<float>(buf.size()-2));
       const float y=v-x*0.52f; buf.push(x+0.52f*y);return y;}
    void clear(){buf.clear();}
private: Ring buf;
};

class StereoVerb {
public:
    void prepare(float sr) {
        const int combLengths[8]={1116,1188,1277,1356,1422,1491,1557,1617};
        const int diffLengths[4]={556,441,341,225};
        for (int ch=0;ch<2;++ch){
            for(int i=0;i<8;++i){ combs[ch][i].prepare(static_cast<std::size_t>(std::round((combLengths[i]+ch*23)*sr/44100.f))+2);combs[ch][i].setDamping(sr); }
            for(int i=0;i<4;++i) diffs[ch][i].prepare(static_cast<std::size_t>(std::round((diffLengths[i]+ch*23)*sr/44100.f))+2);
        }
    }
    void clear(){for(auto& chan:combs)for(auto& c:chan)c.clear(); for(auto& chan:diffs)for(auto& d:chan)d.clear();}
    void tick(float l,float r,float feedback,float& outL,float& outR) {
        const float mono=(l+r)*0.5f*0.14f;
        float out[2]={0.f,0.f};
        for(int ch=0;ch<2;++ch){
            float v=0.f;
            for(auto& c:combs[ch])v+=c.tick(mono,feedback);
            for(auto& a:diffs[ch])v=a.tick(v);
            out[ch]=v;
        }
        outL=out[0];outR=out[1];
    }
private:
    std::array<std::array<Comb,8>,2> combs;
    std::array<std::array<Diffuser,4>,2> diffs;
};

class Engine {
public:
    void prepare(double sampleRate, int /*maximumBlockSize*/) {
        sr=static_cast<float>(std::max(8000.0,sampleRate));
        for(int ch=0;ch<2;++ch){
           echo[ch].resize(static_cast<std::size_t>(sr*2.25f));
           modulation[ch].resize(static_cast<std::size_t>(sr*0.12f));
           echoFilter[ch].setLowpass(2900.f,sr);
           air[ch].setLowpass(8500.f,sr);
           lowBody[ch].setLowpass(850.f,sr);
           dcLow[ch].setLowpass(65.f,sr);
        }
        verb.prepare(sr);
        setParameters(params);
        reset();
    }
    void reset() {
        compEnv=0.f; phaseLfo=0.f; chorusLfo=0.f;wowLfo=0.f;
        for(int ch=0;ch<2;++ch){
            focusFilter[ch].clear();edgePeak[ch].clear();echo[ch].clear();modulation[ch].clear();
            echoFilter[ch].clear();air[ch].clear();lowBody[ch].clear();dcLow[ch].clear();
            for(auto& a:phaseStages[ch])a.clear();
        }
        verb.clear();
    }
    void setParameters(const Params& next) {
        params=next;
        input= dbGain(clamp(params.inputDb,-24.f,18.f));
        output=dbGain(clamp(params.outputDb,-30.f,12.f));
        for(int ch=0;ch<2;++ch){
            // A parked-wah-like nasal mid peak, not a model of a specific pedal.
            focusFilter[ch].configure(420.f+params.focus*1450.f,1.25f+params.focus*2.35f,
                                      1.1f+params.focus*8.2f,sr);
            // More pick definition at 2.1 kHz, before the modulation and delays.
            edgePeak[ch].configure(2100.f,0.82f,3.2f*clamp(params.edge,0.f,1.f),sr);
            air[ch].setLowpass(2500.f+params.tone*10000.f,sr);
            lowBody[ch].setLowpass(620.f+params.body*550.f,sr);
        }
    }
    const Params& parameters() const {return params;}
    void process(float* left,float* right,int frames) {
        if(frames<=0 || !left)return;
        const float attack = std::exp(-1.f/(sr*0.007f));
        const float release= std::exp(-1.f/(sr*0.13f));
        const float comp=clamp(params.compression,0.f,1.f);
        const float drive = clamp(params.drive,0.f,1.f);
        const float edge = clamp(params.edge,0.f,1.f);
        const float edgeGain = 1.f + 3.0f*edge;
        const float edgeBias = 0.045f*edge;
        const float edgeZero = std::tanh(edgeBias);
        const float edgeBlend = 0.32f*edge;
        const float edgeNorm = std::max(0.01f,std::tanh(edgeGain));
        const float phmix=clamp(params.phase,0.f,1.f)*0.74f;
        // Even with Chorus at zero, Wobble gives a gentle moving pitch tint.
        const float chorusMix=clamp(params.chorus,0.f,1.f)*0.47f;
        const float wobbleMix=clamp(params.wobble,0.f,1.f)*0.16f;
        const float modMix=clamp(chorusMix+wobbleMix,0.f,0.66f);
        const float reverbMix=clamp(params.space,0.f,1.f)*0.56f;
        const float delayMix=clamp(params.echoMix,0.f,1.f)*0.62f;
        const float feedback=clamp(params.echoFeedback,0.f,.77f);
        const float room=0.69f+clamp(params.decay,0.f,1.f)*0.24f;
        const float widen=clamp(params.width,0.f,1.f);
        const float phInc=2.f*pi*clamp(params.phaseRateHz,0.04f,4.f)/sr;
        const float chorInc=2.f*pi*0.31f/sr;
        const float wowInc=2.f*pi*0.46f/sr;
        const float delaySamples=clamp(params.echoMs,70.f,1300.f)*sr/1000.f;
        for(int i=0;i<frames;++i){
            float raw[2]={left[i]*input,(right?right[i]:left[i])*input};
            const float peak=std::max(std::abs(raw[0]),std::abs(raw[1]));
            compEnv=peak>(compEnv)? attack*compEnv+(1.f-attack)*peak:release*compEnv+(1.f-release)*peak;
            // Simple peak compression, naturally brighter at low levels and without latency.
            const float amount=std::max(0.f,compEnv-0.15f);
            const float compGain=1.f/(1.f+comp*7.f*amount);
            float x[2]{};
            for(int ch=0;ch<2;++ch){
                float s=raw[ch]*(1.f+comp*0.15f)*compGain;
                s=focusFilter[ch].run(s);
                // Soft asymmetric amp-like waveshaping. Unity near zero at Drive 0.
                const float gain=1.f+drive*7.0f;
                if (!params.externalAmp) {
                    const float shaped = std::tanh(s*gain+drive*.065f)-std::tanh(drive*.065f);
                    s=(1.f-drive*.22f)*shaped/std::max(0.01f,std::tanh(gain));
                }
                // EDGE is a separate, lightly compressed upper-mid grit path.
                // It works with the built-in amplifier AND after an external NAM capture.
                // At zero the path is unity, so cleaner presets remain genuinely clean.
                const float present=edgePeak[ch].run(s);
                const float saturated=(std::tanh(present*edgeGain+edgeBias)-edgeZero)/edgeNorm;
                s=present*(1.f-edgeBlend)+saturated*edgeBlend;
                const float low=lowBody[ch].run(s);
                s=s+(low-s)*(params.body-0.5f)*0.72f;
                s=air[ch].run(s);
                s=s-dcLow[ch].run(s);
                x[ch]=s;
            }
            float effected[2]{};
            for(int ch=0;ch<2;++ch){
                const float sweep=std::sin(phaseLfo +ch*(widen*.25f));
                const float freq=400.f + 1250.f*(0.5f+0.5f*sweep);
                const float a=(1.f-std::tan(pi*freq/sr))/(1.f+std::tan(pi*freq/sr));
                float y=x[ch];
                for(auto& stage:phaseStages[ch]) y=stage.tick(y,a);
                const float phased=x[ch]*(1.f-phmix)+(.5f*x[ch]+.5f*y)*phmix;
                const float modPhase=chorusLfo+ch*(1.2f+0.7f*widen);
                const float wowPhase=wowLfo+ch*0.51f;
                const float modMs=20.f+4.f*std::sin(modPhase)+clamp(params.wobble,0.f,1.f)*3.0f*std::sin(wowPhase);
                const float chorusSample=modulation[ch].tap(sr*modMs/1000.f);
                modulation[ch].push(phased);
                effected[ch]=phased*(1.f-modMix)+chorusSample*modMix;
            }
            const float lEcho=echo[0].tap(delaySamples);
            const float rEcho=echo[1].tap(delaySamples*(1.f+0.025f*widen));
            echo[0].push(effected[0]+echoFilter[0].run(lEcho*(1.f-0.12f*widen)+rEcho*.12f*widen)*feedback);
            echo[1].push(effected[1]+echoFilter[1].run(rEcho*(1.f-0.12f*widen)+lEcho*.12f*widen)*feedback);
            const float wetL=effected[0]+lEcho*delayMix;
            const float wetR=effected[1]+rEcho*delayMix;
            float verbL=0.f,verbR=0.f;
            verb.tick(wetL,wetR,room,verbL,verbR);
            float outL=wetL*(1.f-reverbMix)+verbL*reverbMix;
            float outR=wetR*(1.f-reverbMix)+verbR*reverbMix;
            // Mid-side stereo width, from mono (0) to expanded (1).
            float mid=0.5f*(outL+outR);
            float side=0.5f*(outL-outR)*(0.7f+0.7f*widen);
            left[i]=(mid+side)*output;
            if(right)right[i]=(mid-side)*output;
            phaseLfo+=phInc; if(phaseLfo>2.f*pi) phaseLfo-=2.f*pi;
            chorusLfo+=chorInc; if(chorusLfo>2.f*pi) chorusLfo-=2.f*pi;
            wowLfo+=wowInc; if(wowLfo>2.f*pi)wowLfo-=2.f*pi;
        }
    }
private:
    Params params{};
    float sr=48000.f,input=1.f,output=1.f,compEnv=0.f;
    float phaseLfo=0.f,chorusLfo=0.f,wowLfo=0.f;
    BiquadPeak focusFilter[2],edgePeak[2];
    OnePole air[2],lowBody[2],dcLow[2],echoFilter[2];
    std::array<std::array<AllpassStage,4>,2> phaseStages{};
    Ring modulation[2],echo[2];
    StereoVerb verb;
};
}
