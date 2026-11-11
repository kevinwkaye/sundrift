// Optional offline bridge for rendering a user's DI through the exact Sundrift DSP.
// Build: c++ -O3 -std=c++17 -fPIC -shared tools/render_di_bridge.cpp -o libsundrift_render.so
// No JUCE / NAM / network required.
#include "../Source/ToneEngine.h"
#include "../Source/Presets.h"
#include <new>

extern "C" {
struct Renderer { sundrift::Engine engine; };
void* sundrift_renderer_create(int preset, double sampleRate) {
    if(preset < 0 || preset >= static_cast<int>(sundrift::presets.size()) || sampleRate < 8000 || sampleRate > 192000) return nullptr;
    auto* r=new(std::nothrow) Renderer;
    if(!r) return nullptr;
    r->engine.prepare(sampleRate, 1024);
    const auto& v=sundrift::presets[static_cast<std::size_t>(preset)].values;
    sundrift::Params p;
    p.inputDb=v[0];p.compression=v[1];p.focus=v[2];p.drive=v[3];p.edge=v[4];p.body=v[5];p.tone=v[6];
    p.phaseRateHz=v[7];p.phase=v[8];p.chorus=v[9];p.wobble=v[10];p.echoMs=v[11];
    p.echoFeedback=v[12];p.echoMix=v[13];p.space=v[14];p.decay=v[15];p.width=v[16];p.outputDb=v[17];
    r->engine.setParameters(p);
    return r;
}
void sundrift_renderer_process(void* renderer,float* left,float* right,int frames) {
    if(renderer && frames>0) static_cast<Renderer*>(renderer)->engine.process(left,right,frames);
}
void sundrift_renderer_destroy(void* renderer) { delete static_cast<Renderer*>(renderer); }
}
