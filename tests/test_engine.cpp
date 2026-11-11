#include "../Source/ToneEngine.h"
#include "../Source/Presets.h"
#include <array>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include <vector>

#define CHECK(condition) do { if (!(condition)) throw std::runtime_error(     "Test failed: " #condition " at line " + std::to_string(__LINE__)); } while(false)

int main(){
    // Regression: fractional circular-buffer wrap previously rounded to size()
    // in floating point and accessed the element one past the allocation.
    // This directly checks taps on either side of the wrap at 44.1 kHz.
    {
        sundrift::Ring ring;
        ring.resize(5292);
        for (int i=0;i<25000;++i) {
            ring.push(std::sin(float(i)*0.087f));
            for (float delay: {1.f, 882.000061f, 881.999939f, 2000.5f, 5289.99f}) {
                float v=ring.tap(delay);
                CHECK(std::isfinite(v) && std::abs(v)<=1.001f);
            }
        }
    }

    // EDGE must introduce genuine harmonic colour, not just a louder or brighter EQ.
    // Use the external-amp path to isolate the new post-amp grit stage and to check
    // that the same control will still work after a loaded neural capture.
    auto thirdHarmonic = [](float edge) {
        constexpr int n=48000, skip=4096;
        constexpr float sr=48000.f, fundamental=400.f;
        sundrift::Params p;
        p.externalAmp=true; p.inputDb=0.f; p.compression=0.f; p.focus=0.f;
        p.drive=0.f; p.edge=edge; p.body=0.5f; p.tone=1.f;
        p.phase=0.f; p.chorus=0.f; p.wobble=0.f;
        p.echoMix=0.f; p.space=0.f; p.width=0.5f; p.outputDb=0.f;
        sundrift::Engine engine;
        engine.prepare(sr,512);
        engine.setParameters(p);
        std::vector<float> left(n),right(n);
        for(int i=0;i<n;++i) left[i]=right[i]=0.15f*std::sin(2.f*sundrift::pi*fundamental*float(i)/sr);
        for(int i=0;i<n;i+=512)
            engine.process(left.data()+i,right.data()+i,std::min(512,n-i));
        double sine=0.0,cosine=0.0;
        for(int i=skip;i<n;++i){
            CHECK(std::isfinite(left[i]) && std::abs(left[i])<4.f);
            const auto phase=double(2.f*sundrift::pi*3.f*fundamental)*i/sr;
            sine += left[i]*std::sin(phase);
            cosine += left[i]*std::cos(phase);
        }
        return std::hypot(sine,cosine)/double(n-skip);
    };
    const auto flatHarmonic=thirdHarmonic(0.f);
    const auto edgedHarmonic=thirdHarmonic(.8f);
    CHECK(edgedHarmonic > flatHarmonic*1.5 + 0.00001);

    static_assert(sundrift::bands.size()==4);
    static_assert(sundrift::presets.size()==12);
    for(size_t i=0;i<sundrift::presets.size();++i)
        CHECK(sundrift::bandForPreset(int(i))==int(i/3));
    for(double sr:{44100.,48000.,96000.}){
        sundrift::Engine engine;engine.prepare(sr,2048);
        std::array<double,4> soundFingerprints{};
        for(size_t preset=0;preset<sundrift::presets.size();++preset){
            sundrift::Params p;
            const auto& v=sundrift::presets[preset].values;
            p.inputDb=v[0];p.compression=v[1];p.focus=v[2];p.drive=v[3];p.edge=v[4];
            p.body=v[5];p.tone=v[6];p.phaseRateHz=v[7];p.phase=v[8];
            p.chorus=v[9];p.wobble=v[10];p.echoMs=v[11];p.echoFeedback=v[12];
            p.echoMix=v[13];p.space=v[14];p.decay=v[15];p.width=v[16];p.outputDb=v[17];
            engine.reset();engine.setParameters(p);
            const int n=100000;
            std::vector<float> l(n,0.f),r(n,0.f);l[0]=r[0]=0.8f;
            for(int pos=0;pos<n;pos+=457){
                const int frames=std::min(457,n-pos);
                engine.process(l.data()+pos,r.data()+pos,frames);
            }
            double energy=0, hash=0;
            for(int i=0;i<n;++i){
                CHECK(std::isfinite(l[i]) && std::isfinite(r[i]));
                CHECK(std::abs(l[i])<50.f && std::abs(r[i])<50.f);
                energy+=l[i]*l[i]+r[i]*r[i];
                if(i%100==0)hash+=i*(l[i]+r[i]);
            }
            CHECK(energy>0.00001);
            if(preset%3==0)soundFingerprints[preset/3]=hash;
        }
        // The four mode starting points must produce distinguishable output.
        for(size_t a=0;a<4;++a)for(size_t b=a+1;b<4;++b)
            CHECK(std::abs(soundFingerprints[a]-soundFingerprints[b])>0.001);
        // External neural amp mode bypasses the inbuilt waveshaper and remains finite.
        sundrift::Params external;external.externalAmp=true;external.drive=1.f;
        engine.reset();engine.setParameters(external);
        std::vector<float> l(4096,0.05f),r(4096,-0.05f);
        engine.process(l.data(),r.data(),int(l.size()));
        for(size_t i=0;i<l.size();++i)CHECK(std::isfinite(l[i]) && std::isfinite(r[i]));
        std::cout<<"PASS: 12 artist-grouped presets, 4 distinct modes, neural bypass, "<<sr<<" Hz\n";
    }
}
