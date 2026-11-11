// A synthetic plucked-string test signal, NOT an artist recording or guitar capture.
#include "../Source/ToneEngine.h"
#include "../Source/Presets.h"
#include <array>
#include <cstdint>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>
#include <vector>
#include <fstream>
static void u16(std::ofstream& o,uint16_t x){o.put(char(x));o.put(char(x>>8));}
static void u32(std::ofstream& o,uint32_t x){u16(o,uint16_t(x));u16(o,uint16_t(x>>16));}
static void wav(const std::string& path,const std::vector<float>& l,const std::vector<float>& r,int sr){
    std::ofstream f(path,std::ios::binary);
    const uint32_t datasize=uint32_t(l.size()*4);
    f.write("RIFF",4);u32(f,36+datasize);f.write("WAVEfmt ",8);u32(f,16);u16(f,1);u16(f,2);
    u32(f,uint32_t(sr));u32(f,uint32_t(sr*4));u16(f,4);u16(f,16);f.write("data",4);u32(f,datasize);
    for(size_t i=0;i<l.size();++i){
      for(float s:{l[i],r[i]})u16(f,uint16_t(int16_t(std::lround(sundrift::clamp(s,-.999f,.999f)*32767.f))));
    }
}
int main(int argc,char**argv){
    if(argc!=3){std::fprintf(stderr,"usage: render_demo <preset 0..11> <output.wav>\n");return 1;}
    const int preset=std::atoi(argv[1]);const int sr=48000;const int n=sr*8;
    std::vector<float> left(n,0),right(n,0);
    const int notes[8]={57,60,64,67,64,60,59,57};
    std::mt19937 rng(222);
    std::uniform_real_distribution<float> noise(-1,1);
    std::vector<float> loop(1024,0.f);int ix=0;float last=0.f;
    for(int i=0;i<n;++i){
      const int step=i/(sr/2);const int note=notes[step%8];
      const int period=std::max(2,int(float(sr)/(440.f*std::pow(2.f,(note-69)/12.f))));
      if(i%(sr/2)==0){for(int j=0;j<period;++j)loop[j]=noise(rng)*0.4f;ix=0;last=0;}
      float x=loop[ix];loop[ix]=.9965f*.5f*(x+last);last=x;ix=(ix+1)%period;
      const float envelope=std::exp(-3.8f*float(i%(sr/2))/(sr/2));
      left[i]=right[i]=x*envelope;
    }
    if(preset<0 || preset>=int(sundrift::presets.size())) return 2;
    const auto& v=sundrift::presets[static_cast<size_t>(preset)].values;
    sundrift::Params p;
    p.inputDb=v[0];p.compression=v[1];p.focus=v[2];p.drive=v[3];p.edge=v[4];
    p.body=v[5];p.tone=v[6];p.phaseRateHz=v[7];p.phase=v[8];p.chorus=v[9];
    p.wobble=v[10];p.echoMs=v[11];p.echoFeedback=v[12];p.echoMix=v[13];
    p.space=v[14];p.decay=v[15];p.width=v[16];p.outputDb=v[17];
    sundrift::Engine e;e.prepare(sr,512);e.setParameters(p);
    for(int i=0;i<n;i+=512)e.process(left.data()+i,right.data()+i,std::min(512,n-i));
    wav(argv[2],left,right,sr);
    return 0;
}
