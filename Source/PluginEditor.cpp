#include "PluginEditor.h"
#include <algorithm>
#include <cmath>

namespace {
const juce::Colour bg{0xff070809};
const juce::Colour face{0xff101213};
const juce::Colour card{0xff1b1d1e};
const juce::Colour brighter{0xff272a2b};
const juce::Colour border{0xff3a3a3a};
const juce::Colour cream{0xfff3f0e8};
const juce::Colour dim{0xff9a9a96};
const juce::Colour faint{0xff646764};

juce::Font uiFont(float height, bool bold=false) {
    return juce::Font(juce::FontOptions(height, bold ? juce::Font::bold : juce::Font::plain));
}
void panel(juce::Graphics& g, juce::Rectangle<float> box, const char* title,
           const char* subtitle, juce::Colour accent) {
    g.setColour(face); g.fillRoundedRectangle(box, 11.f);
    g.setColour(border); g.drawRoundedRectangle(box, 11.f, 1.f);
    g.setColour(accent); g.fillRoundedRectangle(box.getX()+14,box.getY()+19,4.f,18.f,2.f);
    g.setColour(cream); g.setFont(uiFont(15.f,true));
    g.drawText(title,juce::Rectangle<int>(int(box.getX()+28),int(box.getY()+13),240,24),juce::Justification::centredLeft);
    g.setColour(dim); g.setFont(uiFont(11.f));
    g.drawText(subtitle,juce::Rectangle<int>(int(box.getX()+290),int(box.getY()+16),int(box.getWidth())-302,20),juce::Justification::centredRight);
    g.setColour(border); g.drawHorizontalLine(int(box.getY()+44),box.getX()+14,box.getRight()-14);
}
}

SundriftLookAndFeel::SundriftLookAndFeel() {
    setColour(juce::Slider::textBoxTextColourId,cream);
    setColour(juce::Slider::textBoxBackgroundColourId,juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
    setColour(juce::ComboBox::backgroundColourId,card);
    setColour(juce::ComboBox::outlineColourId,border);
    setColour(juce::ComboBox::textColourId,cream);
    setColour(juce::ComboBox::arrowColourId,dim);
    setColour(juce::PopupMenu::backgroundColourId,card);
    setColour(juce::PopupMenu::textColourId,cream);
    setColour(juce::PopupMenu::highlightedBackgroundColourId,juce::Colour(0xff484b4b));
    setColour(juce::TextButton::buttonColourId,brighter);
    setColour(juce::TextButton::textColourOffId,cream);
    setColour(juce::TextButton::buttonOnColourId,brighter);
}
void SundriftLookAndFeel::drawRotarySlider(juce::Graphics& g,int x,int y,int width,int height,float pos,
    float start,float end,juce::Slider&) {
    const float cx=x+width*.5f,cy=y+height*.47f;
    const float r=std::min(float(width),float(height))*.33f;
    for(int i=0;i<11;++i){
        const float t=float(i)/10.f;
        const float a=start+(end-start)*t;
        const float x1=cx+std::sin(a)*(r+6.f),y1=cy-std::cos(a)*(r+6.f);
        const float x2=cx+std::sin(a)*(r+9.f),y2=cy-std::cos(a)*(r+9.f);
        g.setColour(faint.withAlpha(.68f));g.drawLine(x1,y1,x2,y2,1.0f);
    }
    juce::Path track,progress;
    track.addCentredArc(cx,cy,r,r,0.f,start,end,true);
    progress.addCentredArc(cx,cy,r,r,0.f,start,start+(end-start)*pos,true);
    g.setColour(border);g.strokePath(track,juce::PathStrokeType(3.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    g.setColour(accent);g.strokePath(progress,juce::PathStrokeType(3.3f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    g.setColour(juce::Colour(0xff161819));g.fillEllipse(cx-r*.84f,cy-r*.84f,1.68f*r,1.68f*r);
    g.setColour(brighter);g.drawEllipse(cx-r*.84f,cy-r*.84f,1.68f*r,1.68f*r,1.f);
    const float angle=start+(end-start)*pos;
    g.setColour(cream);g.drawLine(cx,cy,cx+std::sin(angle)*r*.67f,cy-std::cos(angle)*r*.67f,2.5f);
    g.setColour(accent);g.fillEllipse(cx-2.4f,cy-2.4f,4.8f,4.8f);
}
void SundriftLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down) {
    const auto bounds=b.getLocalBounds().toFloat().reduced(.75f);
    g.setColour(down ? brighter.brighter(.16f) : over ? brighter.brighter(.08f) : card);
    g.fillRoundedRectangle(bounds,6.f);
    g.setColour(border);g.drawRoundedRectangle(bounds,6.f,1.f);
}

BandButton::BandButton(int band):juce::Button("band-"+juce::String(band)),index(band){
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}
void BandButton::paintButton(juce::Graphics& g,bool over,bool down){
    const auto r=getLocalBounds().toFloat().reduced(1.f);
    const auto tint=juce::Colour(sundrift::bands[static_cast<size_t>(index)].colour);
    g.setColour(active?juce::Colour(0xff1c1e20):(over?juce::Colour(0xff17191a):face));
    g.fillRoundedRectangle(r,10.f);
    g.setColour(active?tint.withAlpha(.65f):border.withAlpha(.8f));
    g.drawRoundedRectangle(r,10.f,active?1.6f:1.f);
    if(active) {g.setColour(tint);g.fillRoundedRectangle(r.getX()+10,r.getBottom()-6,r.getWidth()-20,3.f,1.5f);}
    g.setColour(active?tint:dim);g.fillEllipse(r.getX()+17,r.getY()+18,8.f,8.f);
    g.setColour(cream.withAlpha(active?1.f:.76f));g.setFont(uiFont(18.f,true));
    g.drawText(sundrift::bands[static_cast<size_t>(index)].name,int(r.getX()+33),int(r.getY()+9),int(r.getWidth()-42),26,juce::Justification::centredLeft);
    g.setColour(active?tint.withAlpha(.83f):faint);g.setFont(uiFont(10.f));
    g.drawText(sundrift::bands[static_cast<size_t>(index)].descriptor,int(r.getX()+16),int(r.getY()+38),int(r.getWidth()-30),16,juce::Justification::centredLeft);
    if(down){g.setColour(cream.withAlpha(.05f));g.fillRoundedRectangle(r,10.f);}
}
void SplitFlapDisplay::setContent(const juce::String& text,const juce::String& desc,juce::Colour colour) {
    if(title!=text){previous=title;title=text;flip=0.f;}
    description=desc;accent=colour;repaint();
}
void SplitFlapDisplay::advance(float seconds) {
    if(flip<1.f){flip=std::min(1.f,flip+seconds/.24f);repaint();}
}
void SplitFlapDisplay::paint(juce::Graphics& g) {
    const auto r=getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff030404));g.fillRoundedRectangle(r,9.f);
    const auto inset=r.reduced(6.f);
    const auto mainCard=inset.withTrimmedBottom(18.f);
    const float midpoint=mainCard.getCentreY();
    g.setGradientFill(juce::ColourGradient(brighter,mainCard.getX(),mainCard.getY(),card,mainCard.getX(),mainCard.getBottom(),false));
    g.fillRoundedRectangle(mainCard,7.f);
    g.setColour(border);g.drawRoundedRectangle(mainCard,7.f,1.f);
    const juce::String upper=(flip<.5f?previous:title);
    const juce::String lower=(flip>.5f?title:previous);
    auto drawTextHalf=[&](const juce::String& text,bool top){
        juce::Graphics::ScopedSaveState save(g);
        if(top)g.reduceClipRegion(juce::Rectangle<int>(int(mainCard.getX()),int(mainCard.getY()),int(mainCard.getWidth()),int(mainCard.getHeight()*.5f)));
        else g.reduceClipRegion(juce::Rectangle<int>(int(mainCard.getX()),int(midpoint),int(mainCard.getWidth()),int(mainCard.getHeight()*.5f)+1));
        g.setColour(cream);g.setFont(uiFont(30.f,true));
        g.drawText(text,juce::Rectangle<int>(int(mainCard.getX()+18),int(mainCard.getY()+2),int(mainCard.getWidth()-32),int(mainCard.getHeight()-6)),juce::Justification::centredLeft);
    };
    drawTextHalf(upper,true);drawTextHalf(lower,false);
    g.setColour(juce::Colours::black.withAlpha(.78f));g.fillRect(mainCard.getX()+1,midpoint-1.f,mainCard.getWidth()-2,2.f);
    g.setColour(accent);g.fillRoundedRectangle(mainCard.getRight()-9.f,mainCard.getY()+8.f,3.f,mainCard.getHeight()-16.f,1.5f);
    g.setColour(juce::Colour(0xff656766));
    for(auto x:{mainCard.getX()+8.f,mainCard.getRight()-13.f}){
        g.fillEllipse(x,midpoint-2.7f,5.f,5.f);
    }
    g.setColour(accent.withAlpha(.78f));g.setFont(uiFont(10.f,true));
    g.drawText(description,juce::Rectangle<int>(int(inset.getX()+14),int(mainCard.getBottom()+1),int(inset.getWidth()-25),17),juce::Justification::centredLeft);
}

SundriftAudioProcessorEditor::SundriftAudioProcessorEditor(SundriftAudioProcessor& p)
    :juce::AudioProcessorEditor(&p),processor(p){
    setLookAndFeel(&theme);
    setSize(1140,742);
    for(int i=0;i<SundriftAudioProcessor::numKnobs;++i){
        auto& k=knobs[static_cast<size_t>(i)];
        k.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        k.slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,76,17);
        k.slider.setRotaryParameters(juce::degreesToRadians(218.f),juce::degreesToRadians(502.f),true);
        if(auto* param=processor.state.getParameter(SundriftAudioProcessor::knobIDs[static_cast<size_t>(i)]))
            k.slider.setDoubleClickReturnValue(true,param->convertFrom0to1(param->getDefaultValue()));
        addAndMakeVisible(k.slider);
        k.label.setText(SundriftAudioProcessor::knobLabels[static_cast<size_t>(i)],juce::dontSendNotification);
        k.label.setJustificationType(juce::Justification::centred);
        k.label.setColour(juce::Label::textColourId,dim);
        k.label.setFont(uiFont(11.f,true));
        addAndMakeVisible(k.label);
        k.attach=std::make_unique<SundriftAudioProcessor::APVTS::SliderAttachment>(processor.state,
              SundriftAudioProcessor::knobIDs[static_cast<size_t>(i)],k.slider);
    }
    for(int i=0;i<4;++i){
        bandButtons[static_cast<size_t>(i)]=std::make_unique<BandButton>(i);
        bandButtons[static_cast<size_t>(i)]->onClick=[this,i]{
            processor.setPreset(lastBandPresets[static_cast<size_t>(i)]);
            syncPresetControls();
        };
        addAndMakeVisible(*bandButtons[static_cast<size_t>(i)]);
    }
    addAndMakeVisible(nameDisplay);
    presetChoice.onChange=[this]{
        const int pick=presetChoice.getSelectedId();
        if(pick>0){
            const int index=processor.currentBandIndex()*sundrift::presetsPerBand+(pick-1);
            processor.setPreset(index);syncPresetControls();
        }
    };
    addAndMakeVisible(presetChoice);
    ampMode.addItem("Built-in clean",1);
    ampMode.addItem("NAM A2 model",2);
    addAndMakeVisible(ampMode);
    ampAttachment=std::make_unique<SundriftAudioProcessor::APVTS::ComboBoxAttachment>(processor.state,"ampMode",ampMode);
    loadModel.setButtonText("LOAD .NAM");
    loadModel.onClick=[this]{
        chooser=std::make_unique<juce::FileChooser>("Choose a NAM amplifier capture",juce::File{},"*.nam");
        juce::Component::SafePointer<SundriftAudioProcessorEditor> safe(this);
        chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,
        [safe](const juce::FileChooser& file){
            if(safe && file.getResult().existsAsFile())safe->processor.selectNamFile(file.getResult());
        });
    };
    loadModel.setEnabled(processor.namAvailable());
    addAndMakeVisible(loadModel);
    captureState.setColour(juce::Label::textColourId,dim);
    captureState.setFont(uiFont(10.f));
    addAndMakeVisible(captureState);
    versionMark.setText("FLIP FELLA-STYLE / SND.01 / V0.1.2",juce::dontSendNotification);
    versionMark.setColour(juce::Label::textColourId,faint);
    versionMark.setFont(uiFont(10.f));
    versionMark.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(versionMark);
    presetLabel.setText("SCENE",juce::dontSendNotification);
    ampLabel.setText("AMP ENGINE",juce::dontSendNotification);
    for(auto* label:{&presetLabel,&ampLabel}){
        label->setColour(juce::Label::textColourId,dim);
        label->setFont(uiFont(11.f,true));
        addAndMakeVisible(*label);
    }
    syncPresetControls();
    startTimerHz(30);
}
SundriftAudioProcessorEditor::~SundriftAudioProcessorEditor(){
    stopTimer();chooser.reset();ampAttachment.reset();
    for(auto& k:knobs)k.attach.reset();
    setLookAndFeel(nullptr);
}
void SundriftAudioProcessorEditor::syncPresetControls(){
    const int index=processor.currentPresetIndex();
    if(index==displayedPreset)return;
    const int band=sundrift::bandForPreset(index);
    const auto& info=sundrift::bands[static_cast<size_t>(band)];
    const auto& recipe=sundrift::presets[static_cast<size_t>(index)];
    const auto tint=juce::Colour(info.colour);
    lastBandPresets[static_cast<size_t>(band)]=index;
    if(displayedPreset<0 || sundrift::bandForPreset(displayedPreset)!=band){
        presetChoice.clear(juce::dontSendNotification);
        for(int i=0;i<sundrift::presetsPerBand;++i)
            presetChoice.addItem(sundrift::presets[static_cast<size_t>(band*sundrift::presetsPerBand+i)].name,i+1);
    }
    presetChoice.setSelectedId(index%sundrift::presetsPerBand+1,juce::dontSendNotification);
    for(int i=0;i<4;++i)bandButtons[static_cast<size_t>(i)]->setSelected(i==band);
    nameDisplay.setContent(recipe.name,recipe.character,tint);
    theme.setAccent(tint);
    displayedPreset=index;
    for(auto& k:knobs)k.slider.repaint();
    repaint();
}
void SundriftAudioProcessorEditor::paint(juce::Graphics& g){
    g.fillAll(bg);
    const int b=processor.currentBandIndex();
    const auto accent=juce::Colour(sundrift::bands[static_cast<size_t>(b)].colour);
    g.setColour(face);g.fillRoundedRectangle({15.f,12.f,1110.f,717.f},15.f);
    g.setColour(border);g.drawRoundedRectangle({15.f,12.f,1110.f,717.f},15.f,1.f);
    g.setColour(cream);g.setFont(uiFont(47.f,true));
    g.drawText("SUNDRIFT",36,24,450,65,juce::Justification::centredLeft);
    g.setColour(accent);g.fillRoundedRectangle(39.f,98.f,214.f,3.f,1.5f);
    g.setColour(dim);g.setFont(uiFont(12.f));
    g.drawText("WARM SIGNALS  /  SLOW HORIZONS",41,102,480,23,juce::Justification::centredLeft);
    g.setColour(cream.withAlpha(.55f));g.setFont(uiFont(10.f,true));
    g.drawText("FOUR FREQUENCIES / ONE GUITAR",799,57,299,23,juce::Justification::centredRight);
    g.setColour(accent);g.fillEllipse(1093.f,40.f,8.f,8.f);
    g.setColour(faint);g.drawHorizontalLine(129,33.f,1107.f);
    // MODE STRIP: four tactile tabs, each with its own colour and description.
    g.setColour(dim);g.setFont(uiFont(11.f,true));
    g.drawText("SELECT BAND",34,132,140,19,juce::Justification::centredLeft);
    g.setColour(face);g.fillRoundedRectangle({28.f,218.f,720.f,94.f},10.f);
    g.setColour(border);g.drawRoundedRectangle({28.f,218.f,720.f,94.f},10.f,1.f);
    g.setColour(face);g.fillRoundedRectangle({760.f,218.f,352.f,94.f},10.f);
    g.setColour(border);g.drawRoundedRectangle({760.f,218.f,352.f,94.f},10.f,1.f);
    g.setColour(accent);g.fillRoundedRectangle(763.f,221.f,3.f,88.f,1.5f);
    panel(g,{28.f,327.f,620.f,182.f},"01  /  SIGNAL","TOUCH / SATURATION",accent);
    panel(g,{660.f,327.f,452.f,182.f},"02  /  MOTION","SLOW SWELLS",accent);
    panel(g,{28.f,522.f,476.f,180.f},"03  /  ECHO","DARK REPEATS",accent);
    panel(g,{516.f,522.f,596.f,180.f},"04  /  SPACE","SOFT EDGES",accent);
    g.setColour(dim);g.setFont(uiFont(10.f));
    g.drawText("ARTIST-INSPIRED TONE RECIPES. NOT AFFILIATED WITH THE ARTISTS.",32,707,680,16,juce::Justification::centredLeft);
}
void SundriftAudioProcessorEditor::resized(){
    for(int i=0;i<4;++i)bandButtons[static_cast<size_t>(i)]->setBounds(29+i*273,155,263,54);
    nameDisplay.setBounds(37,226,465,78);
    presetLabel.setBounds(517,231,160,17);
    presetChoice.setBounds(517,250,216,36);
    ampLabel.setBounds(780,229,111,19);
    ampMode.setBounds(778,249,175,32);
    loadModel.setBounds(966,249,132,32);
    captureState.setBounds(778,282,322,23);
    versionMark.setBounds(816,705,286,19);
    auto place=[this](int id,int x,int y,int width){
        auto& k=knobs[static_cast<size_t>(id)];
        k.slider.setBounds(x,y,width,96);
        k.label.setBounds(x-4,y+93,width+8,18);
    };
    for(int i=0;i<7;++i)place(i,36+i*87,378,84);
    for(int i=0;i<4;++i)place(7+i,675+i*107,378,98);
    for(int i=0;i<3;++i)place(11+i,52+i*147,570,101);
    for(int i=0;i<4;++i)place(14+i,532+i*142,570,109);
}
void SundriftAudioProcessorEditor::timerCallback(){
    syncPresetControls();
    nameDisplay.advance(1.f/30.f);
    const auto text=processor.namStatusText();
    if(captureState.getText()!=text)captureState.setText(text,juce::dontSendNotification);
}
