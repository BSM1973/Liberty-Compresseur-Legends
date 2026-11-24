#pragma once
#include "History.h"
#include <functional>
#include <vector>
#include <utility>
namespace clskin {
using R=juce::Rectangle<float>;
const juce::Colour midnight{0xff15171b},navy{0xff24282e},sapphire{0xff343a43},
 navySoft{0xff424852},cream{0xfff0f1f3},paper{0xffe4e6e9},
 gold{0xffa9253f},goldLight{0xffe1e5ea},goldDark{0xff777e89},
 ink{0xff191c21},muted{0xffaab2bc},amber{0xff49c5b0};
constexpr float pi=juce::MathConstants<float>::pi;
void surface(juce::Graphics& g,R r,juce::Colour top,juce::Colour bottom,float radius=12.f,
             juce::Colour outline=goldDark) {
    g.setColour(midnight.withAlpha(.6f)); g.fillRoundedRectangle(r.translated(1.7f,4.f),radius);
    g.setGradientFill(juce::ColourGradient(top,r.getTopLeft(),bottom,r.getBottomRight(),false));
    g.fillRoundedRectangle(r,radius);
    g.setColour(outline.withAlpha(.8f));g.drawRoundedRectangle(r.reduced(.9f),radius,1.5f);
    g.setColour(goldLight.withAlpha(.15f));g.drawRoundedRectangle(r.reduced(3.f),radius-2.f,.8f);
}
void line(juce::Graphics& g,float x1,float y1,float x2,float y2,juce::Colour c,float thick=1.f) {
    g.setColour(c);g.drawLine(x1,y1,x2,y2,thick);
}

}
class Editor final:public juce::AudioProcessorEditor,private juce::Timer {
 using R=clskin::R;
 using AP=Processor::APVTS;
 Processor& p;
 enum class Screen{Rack,Detail,Library,History,Routing,Presets,Analysis,Advanced,Character,Pro};
 Screen screen=Screen::Rack;
 int selected=0,proBand=0;bool proEQPage=false;int historyPage=0,libraryPage=0,presetPage=0,presetCategory=-1,lastPreset=-1;bool onlyFavorites=false;std::array<bool,liberty::factoryCount> favorites{};juce::TextEditor presetSearch;
 std::array<std::array<float,120>,8> grHistory{};int grHead=0;bool french=true,viewB=false;
 juce::ValueTree snapshotA,snapshotB;
 struct Hot {R bounds;std::function<void()> click;juce::RangedAudioParameter* param=nullptr;};
 std::vector<Hot> hits;
 juce::RangedAudioParameter* dragging=nullptr;
 float startY=0,startValue=0;
 float scale=1.f,xOffset=0,yOffset=0;
 static juce::String u(const char* s){return juce::String::fromUTF8(s);}
 juce::String tr(const char* fr,const char* en)const{return u(french?fr:en);}
 juce::RangedAudioParameter* param(int slot,const char* key){return p.state.getParameter(Processor::id(slot,key));}
 float raw(int slot,const char* key){return p.state.getRawParameterValue(Processor::id(slot,key))->load();}
 int model(int slot){return juce::jlimit(0,liberty::modelCount-1,int(raw(slot,"model")));}
 bool on(int slot){return raw(slot,"on")>.5f;}
 void set(juce::RangedAudioParameter* a,float v){if(!a)return;a->beginChangeGesture();a->setValueNotifyingHost(juce::jlimit(0.f,1.f,v));a->endChangeGesture();}
 void setRaw(int slot,const char* key,float v){auto* a=param(slot,key);set(a,a->convertTo0to1(v));}
 void swapSlots(int a,int b){if(b<0||b>=8)return;for(auto key:{"on","model","threshold","ratio","attack","release","makeup","mix","color","external","stereo","auto","delta","analog","character","allButtons"}){float x=raw(a,key),y=raw(b,key);setRaw(a,key,y);setRaw(b,key,x);}for(auto key:{"pro","bandCount","fft","link","scHP","scLP","scBell","scGain","scListen"}){float x=raw(a,key),y=raw(b,key);setRaw(a,key,y);setRaw(b,key,x);}for(int j=0;j<5;++j){auto key=Processor::crossKey(j);float x=raw(a,key.toRawUTF8()),y=raw(b,key.toRawUTF8());setRaw(a,key.toRawUTF8(),y);setRaw(b,key.toRawUTF8(),x);}for(int j=0;j<6;++j)for(auto field:{"threshold","ratio","attack","release","solo","mute","bypass"}){auto key=Processor::bandKey(j,field);float x=raw(a,key.toRawUTF8()),y=raw(b,key.toRawUTF8());setRaw(a,key.toRawUTF8(),y);setRaw(b,key.toRawUTF8(),x);}selected=b;}
 void toggleAB(){if(viewB){snapshotB=p.state.copyState();p.state.replaceState(snapshotA.createCopy());}else{snapshotA=p.state.copyState();p.state.replaceState(snapshotB.createCopy());}viewB=!viewB;}
 static juce::Colour color(int m){constexpr unsigned c[]={0xff8d2638,0xff655e50,0xff58616b,0xff536677,0xff643654,0xff286054,0xff756956,0xff41454d,0xff8d2638,0xff763344,0xff6c6352,0xff756956,0xff58616b,0xff536677,0xff643654,0xff75445e,0xff286054,0xff31675b,0xff41454d,0xff4b505b,0xff8d2638,0xff763344,0xff655e50,0xff756956,0xff58616b,0xff536677,0xff643654,0xff75445e,0xff286054,0xff31675b,0xff41454d,0xff4b505b,0xff41454d,0xff4b505b,0xff64538e,0xff64538e,0xff7462b0,0xff64538e,0xff41454d,0xff4b505b,0xff8d2638,0xff763344,0xff655e50,0xff756956,0xff58616b,0xff536677,0xff643654,0xff75445e,0xff286054,0xff31675b,0xffaa6039,0xff94582f,0xff64538e,0xff64538e,0xff7462b0,0xff64538e,0xff59498b,0xff6857a5,0xff59498b,0xff7462b0};return juce::Colour(c[juce::jlimit(0,liberty::modelCount-1,m)]);}
 static const char* family(int m){constexpr const char* f[]={"FET","OPTO","VCA","VCA","VARI-MU","DIODE","OPTO","DIGITAL","FET","FET","OPTO","OPTO","VCA","VCA","VARI-MU","VARI-MU","DIODE","DIODE","DIGITAL","DIGITAL","FET","FET","OPTO","OPTO","VCA","VCA","VARI-MU","VARI-MU","DIODE","DIODE","DIGITAL","DIGITAL","DIGITAL","DIGITAL","DUAL BAND","DUAL BAND","DUAL BAND","DUAL BAND","DIGITAL","DIGITAL","FET","FET","OPTO","OPTO","VCA","VCA","VARI-MU","VARI-MU","DIODE","DIODE","PWM-INSPIRED","PWM-INSPIRED","MULTIBAND","MULTIBAND","MULTIBAND","MULTIBAND","SPECTRAL","SPECTRAL","SPECTRAL","SPECTRAL"};return f[m];}
 void drawLabel(juce::Graphics& g,const juce::String& s,R r,float size,juce::Colour colour,bool serif=false,juce::Justification alignment=juce::Justification::centredLeft,bool bold=false){
  g.setColour(colour);auto name=serif?juce::Font::getDefaultSerifFontName():juce::Font::getDefaultSansSerifFontName();
  g.setFont(juce::Font(juce::FontOptions(name,size,bold?juce::Font::bold:juce::Font::plain)));g.drawFittedText(s,r.toNearestInt(),alignment,1,.8f);
 }
 void hit(R r,std::function<void()> f){hits.push_back({r,std::move(f),nullptr});}
 void button(juce::Graphics& g,R r,const juce::String& s,std::function<void()> fn,bool filled=false,bool active=false,float size=17.f){
  using namespace clskin;surface(g,r,active?gold:(filled?sapphire:navySoft),active?gold.darker(.45f):navy,6.f,active?goldLight:goldDark);
  drawLabel(g,s,r.reduced(4.f),size,cream,false,juce::Justification::centred,true);if(fn)hit(r,std::move(fn));
 }
 void knobDraw(juce::Graphics& g,float x,float y,float radius,float value,const juce::String& title,const juce::String& display,bool darkBackground,juce::RangedAudioParameter* a=nullptr){
  using namespace clskin;
    machined(g,x,y,radius,value);
    // Compact MASTER knobs use non-overlapping caption rectangles.
    const float captionHalfWidth=radius<=20.f?53.f:radius+45.f;
    drawLabel(g,title,R{x-captionHalfWidth,y+radius+13.f,2.f*captionHalfWidth,19.f},13.f,darkBackground?cream:ink,false,
              juce::Justification::centred,true);
    if(display.isNotEmpty()) {
        const auto box=R{x-54.f,y+radius+38.f,108.f,26.f};
        g.setColour(goldDark.withAlpha(.12f));g.fillRoundedRectangle(box,4.f);
        g.setColour(goldDark.withAlpha(.35f));g.drawRoundedRectangle(box,4.f,.9f);
        drawLabel(g,display,box,13.f,darkBackground?cream:ink,false,juce::Justification::centred);
    }

  if(a)hits.push_back({R{x-radius-7,y-radius-7,2*(radius+7),2*(radius+7)},{},a});
 }
 void knob(juce::Graphics& g,int slot,const char* key,float x,float y,float radius,const juce::String& title,const juce::String& suffix){
  auto* a=param(slot,key);knobDraw(g,x,y,radius,a->getValue(),title,valueText(slot,key),true,a);
 }
 struct Finish{juce::Colour top,bottom,text;bool darkMeter=false;int detail=0;};
 Finish familyFinish(int m){
  using C=juce::Colour;const juce::String fam=family(m);
  if(fam=="OPTO")return {C(0xffc8c9c6),C(0xff757c80),C(0xff16191c),false,1};
  if(fam=="VCA")return {C(0xff7d8289),C(0xff30343a),C(0xfffafafa),false,2};
  if(fam=="VARI-MU")return {C(0xff76272c),C(0xff321519),C(0xffffece2),false,3};
  if(fam=="DIODE")return {C(0xff8d7854),C(0xff39352c),C(0xfff9e7c9),false,1};
  if(fam=="PWM-INSPIRED")return {C(0xff555960),C(0xff15191e),C(0xfff0f0f0),true,4};
  if(fam=="MULTIBAND"||fam=="DUAL BAND")return {C(0xffbabdc3),C(0xff666b75),C(0xff111318),false,2};
  if(fam=="SPECTRAL"||fam=="DIGITAL")return {C(0xff474b55),C(0xff15181d),C(0xffeceff2),true,4};
  return {C(0xff45474a),C(0xff1c1d20),C(0xfff4f0e8),false,0};
 }
 // Individual face trim, serial and control cap style remain tied to model ID.
 juce::Colour modelTrim(int m){auto c=color(m);return c.interpolatedWith(juce::Colour(0xffb8bdc4),.15f+float(m%5)*.09f);}
 Finish finish(int m){auto f=familyFinish(m);float amount=.04f+float(m%7)*.022f;auto c=modelTrim(m);f.top=f.top.interpolatedWith(c,amount);f.bottom=f.bottom.interpolatedWith(c,amount*.8f);return f;}
 juce::String modelSerial(int m){return "CL-"+juce::String(m+1).paddedLeft('0',3);}
 void signatureTrim(juce::Graphics& g,R r,int m){
  using namespace clskin;auto c=modelTrim(m);line(g,r.getX()+18,r.getY()+39,r.getRight()-18,r.getY()+39,c,1.6f);
  for(int k=0;k<1+m%4;++k){float x=r.getX()+18+5*k;line(g,x,r.getBottom()-17,x,r.getBottom()-12,c,1.8f);}
 }
 void screw(juce::Graphics& g,float x,float y,float radius=5){
  using namespace clskin;g.setColour(midnight);g.fillEllipse(x-radius-1,y-radius-1,2*radius+2,2*radius+2);
  g.setGradientFill(juce::ColourGradient(goldLight,{x-radius,y-radius},goldDark,{x+radius,y+radius},false));g.fillEllipse(x-radius,y-radius,2*radius,2*radius);
  line(g,x-radius*.55f,y+radius*.45f,x+radius*.55f,y-radius*.45f,ink,1.5f);
 }
 void brushed(juce::Graphics& g,R r,juce::Colour top,juce::Colour bottom,juce::Colour edge){
  using namespace clskin;surface(g,r,top,bottom,8,edge);
  // Deterministic fine metal grain, native vector paint; no image or animation noise.
  juce::Graphics::ScopedSaveState save(g);g.reduceClipRegion(r.reduced(5).toNearestInt());
  for(int i=0;i<int(r.getHeight()-10);++i){unsigned k=unsigned(i)*1664525u+1013904223u;float y=r.getY()+5+i;float alpha=.018f+float((k>>24)&31)/650.f;line(g,r.getX()+6,y,r.getRight()-6,y,(i%3?juce::Colours::white:juce::Colours::black).withAlpha(alpha),.65f);}
 }
 void led(juce::Graphics& g,float x,float y,bool active){
  using namespace clskin;g.setColour(ink);g.fillEllipse(x-7,y-7,14,14);auto c=active?amber:juce::Colour(0xff48504e);
  if(active){g.setColour(c.withAlpha(.18f));g.fillEllipse(x-10,y-10,20,20);}g.setGradientFill(juce::ColourGradient(c.brighter(.5f),{x-3,y-3},c.darker(.5f),{x+4,y+4},true));g.fillEllipse(x-5,y-5,10,10);
 }
 void machined(juce::Graphics& g,float x,float y,float rad,float value,int modelId=-1){
  using namespace clskin;g.setColour(juce::Colours::black.withAlpha(.8f));g.fillEllipse(x-rad-2,y-rad+3,2*rad+4,2*rad+3);
  g.setGradientFill(juce::ColourGradient(juce::Colour(0xffe5e7e7),{x-rad,y-rad},juce::Colour(0xff30343a),{x+rad,y+rad},false));g.fillEllipse(x-rad,y-rad,2*rad,2*rad);
  g.setColour(ink);g.drawEllipse(x-rad+2,y-rad+2,2*rad-4,2*rad-4,1.3f);
  float rr=rad*.82f;g.setGradientFill(juce::ColourGradient(juce::Colour(0xffe8e8e4),{x-rr,y-rr},juce::Colour(0xff666b70),{x+rr,y+rr},false));g.fillEllipse(x-rr,y-rr,2*rr,2*rr);
  for(int i=0;i<24;++i){float a=i*2*pi/24;line(g,x+std::cos(a)*rr*.22f,y+std::sin(a)*rr*.22f,x+std::cos(a)*rr*.93f,y+std::sin(a)*rr*.93f,juce::Colours::white.withAlpha(.055f),.5f);}
  if(modelId>=0){auto c=modelTrim(modelId);g.setColour(c);g.drawEllipse(x-rad+1,y-rad+1,2*rad-2,2*rad-2,1.8f);
   if(modelId%3==1){g.setColour(juce::Colour(0xff32353a));g.fillEllipse(x-rr*.53f,y-rr*.53f,rr*1.06f,rr*1.06f);}
   else if(modelId%3==2){g.setColour(c.darker(.35f));g.fillEllipse(x-rr*.40f,y-rr*.40f,rr*.80f,rr*.80f);}
  }
  float a=(.75f+1.5f*std::clamp(value,0.f,1.f))*pi;line(g,x+std::cos(a)*rad*.42f,y+std::sin(a)*rad*.42f,x+std::cos(a)*rad*.86f,y+std::sin(a)*rad*.86f,ink,2.4f);
  for(int i=0;i<=10;++i){float t=(.75f+1.5f*i/10)*pi;line(g,x+std::cos(t)*(rad+2),y+std::sin(t)*(rad+2),x+std::cos(t)*(rad+5),y+std::sin(t)*(rad+5),goldLight.withAlpha(.8f),.9f);}
 }
 juce::String valueText(int slot,const char* key){
  float value=raw(slot,key);juce::String k(key);
  if(k=="mix"||k=="color"||k=="character")return juce::String(int(std::round(value*100)))+" %";
  if(k=="ratio")return juce::String(value,1)+":1";
  if(k=="attack"||k=="release")return juce::String(value,value<10?1:0)+" ms";
  return juce::String(value,1)+" dB";
 }
 void rackKnob(juce::Graphics& g,int slot,const char* key,float x,float y,const juce::String& label,juce::Colour text){
  auto* a=param(slot,key);machined(g,x,y,17,a->getValue(),model(slot));
  drawLabel(g,label,R{x-42,y+22,84,15},12.8f,text,false,juce::Justification::centred,true);
  drawLabel(g,valueText(slot,key),R{x-42,y+38,84,15},12.5f,text,false,juce::Justification::centred);
  hits.push_back({R{x-23,y-23,46,46},{},a});
 }
 void meter(juce::Graphics& g,R r,float gr,bool dark=false,bool compact=false){
  using namespace clskin;surface(g,r,juce::Colour(0xff343436),juce::Colour(0xff101113),6,juce::Colour(0xff777b7d));
  auto face=r.reduced(6);auto tint=dark?juce::Colour(0xff33191d):juce::Colour(0xfff6d9a2);auto fg=dark?juce::Colour(0xffff655d):juce::Colour(0xff302719);
  g.setGradientFill(juce::ColourGradient(tint.brighter(.16f),face.getCentre(),tint.darker(.35f),face.getBottomRight(),true));g.fillRoundedRectangle(face,3);
  float x=face.getCentreX(),y=face.getBottom()-5,rad=std::min(face.getWidth()*.48f,face.getHeight()*.92f);
  const float marks[]={30,20,10,7,5,3,1,0};
  auto angle=[](float db){return (-155.f+130.f*(1.f-std::clamp(db,0.f,30.f)/30.f))*clskin::pi/180.f;};
  for(float mark:marks){float a=angle(mark),dx=std::cos(a),dy=std::sin(a);line(g,x+dx*(rad-7),y+dy*(rad-7),x+dx*rad,y+dy*rad,fg,1.2f);
   if(!compact&&(mark==30||mark==20||mark==10||mark==5||mark==0))drawLabel(g,juce::String(int(mark)),R{x+dx*(rad+6)-10,y+dy*(rad+6)-7,20,12},9.5f,fg,false,juce::Justification::centred);}
  float a=angle(gr);line(g,x,y,x+std::cos(a)*(rad-2),y+std::sin(a)*(rad-2),dark?juce::Colour(0xffff6d63):juce::Colour(0xff8c1f24),1.7f);g.setColour(fg);g.fillEllipse(x-3,y-3,6,6);
  if(!compact)drawLabel(g,"GR (dB)",R{x-39,face.getY()+4,78,12},10,fg,false,juce::Justification::centred,true);
  g.setColour(juce::Colours::white.withAlpha(.12f));g.fillRoundedRectangle(face.withHeight(face.getHeight()*.20f),2);
 }
 void display(juce::Graphics& g,R r,int slot,int m){
  using namespace clskin;const auto& md=liberty::models[size_t(m)];auto skin=finish(m);
  if(md.architecture==9){
   surface(g,r,juce::Colour(0xff414449),juce::Colour(0xff0b1011),5,goldDark);auto plot=r.reduced(8);plot.removeFromTop(14);
   drawLabel(g,"FFT / GR (dB)",R{r.getX()+6,r.getY()+3,r.getWidth()-12,13},10,amber,false,juce::Justification::centred,true);
   for(int j=0;j<=3;++j){float y=plot.getY()+plot.getHeight()*j/3;line(g,plot.getX(),y,plot.getRight(),y,amber.withAlpha(.15f));}
   for(int k=0;k<16;++k){float val=slot>=0&&on(slot)?p.displayMeters[slot][k].load():0;float h=plot.getHeight()*std::clamp(val/30.f,0.f,1.f),w=plot.getWidth()/16;g.setColour(amber.withAlpha(.9f));g.fillRect(R{plot.getX()+k*w+1,plot.getBottom()-h,w-2,h});}
  }else if(md.architecture==5||md.architecture==7||md.architecture==8){
   int n=slot>=0&&raw(slot,"pro")>.5f?juce::jlimit(2,6,int(raw(slot,"bandCount"))):md.architecture==5?2:md.architecture==7?3:4;float w=r.getWidth()/n;for(int k=0;k<n;++k){float gr=slot>=0&&on(slot)?p.displayMeters[slot][k].load():0;meter(g,R{r.getX()+k*w+2,r.getY(),w-4,r.getHeight()-15},gr,false,true);drawLabel(g,"B"+juce::String(k+1),R{r.getX()+k*w,r.getBottom()-14,w,13},11,skin.text,false,juce::Justification::centred,true);}
  }else meter(g,r,slot>=0&&on(slot)?p.meters[slot].load():0,skin.darkMeter);
 }
 void vents(juce::Graphics& g,R r,int kind,bool active){
  using namespace clskin;
  if(kind==3){surface(g,r,juce::Colour(0xff392c26),ink,4,goldDark);for(int i=0;i<5;++i){float x=r.getX()+4+i*(r.getWidth()-8)/5;g.setColour((active?juce::Colour(0xffe5a04d):juce::Colour(0xff59402c)).withAlpha(.6f));g.fillRoundedRectangle(R{x,r.getY()+5,2.5f,r.getHeight()-10},1);}}
  else if(kind==4){for(int j=0;j<5;++j)for(int i=0;i<3;++i){float x=r.getX()+3+i*8+(j%2?2:0),y=r.getY()+3+j*8;g.setColour(ink);g.fillEllipse(x,y,5,5);}}
  else for(int i=0;i<4;++i){float y=r.getY()+5+i*9;g.setColour(ink);g.fillRoundedRectangle(R{r.getX(),y,r.getWidth(),4},2);line(g,r.getX()+2,y+4,r.getRight()-2,y+4,goldLight.withAlpha(.4f));}
 }
 void enclosure(juce::Graphics& g,R r,int slot,int m,bool mini){
  using namespace clskin;auto skin=finish(m);brushed(g,r,skin.top,skin.bottom,goldDark);signatureTrim(g,r,m);
  for(float x:{r.getX()+10,r.getRight()-10})for(float y:{r.getY()+10,r.getBottom()-10})screw(g,x,y);
  drawLabel(g,family(m),R{r.getX()+18,r.getY()+8,r.getWidth()-36,24},mini?17:22,skin.text,false,juce::Justification::centred,true);
  surface(g,R{r.getX()+32,r.getY()+35,r.getWidth()-64,34},juce::Colour(0xff802c37),juce::Colour(0xff3d141a),4,goldDark);
  drawLabel(g,liberty::models[m].name,R{r.getX()+36,r.getY()+36,r.getWidth()-72,31},mini?21:28,cream,true,juce::Justification::centred,true);
  display(g,R{r.getX()+22,r.getY()+79,r.getWidth()-44,r.getHeight()-117},slot,m);
  if(slot>=0){led(g,r.getX()+24,r.getBottom()-19,on(slot));drawLabel(g,juce::String(p.meters[slot].load(),1)+" dB GR",R{r.getX()+43,r.getBottom()-30,r.getWidth()-86,23},15,skin.text,false,juce::Justification::centred,true);}
  else drawLabel(g,"LIBERTY STUDIO / "+modelSerial(m),R{r.getX()+25,r.getBottom()-29,r.getWidth()-50,23},12,skin.text,false,juce::Justification::centred,true);
 }
 void backplate(juce::Graphics& g){
  using namespace clskin;
  g.setGradientFill(juce::ColourGradient(sapphire,{0.f,0.f},midnight,{0.f,930.f},false));g.fillRect(R{0,0,1600,930});
  for(int n=1;n<100;++n)line(g,0.f,float(n)*9.35f,1600.f,float(n)*9.35f,cream.withAlpha(n%3==0?.021f:.007f));
  juce::Path badge;badge.addQuadrilateral(61,27,85,54,61,80,37,54);g.setColour(gold);g.strokePath(badge,juce::PathStrokeType(3.f));
  drawLabel(g,"L",R{45,32,33,41},30,goldLight,true,juce::Justification::centred,true);
  drawLabel(g,"LIBERTY MUSIC SOFTWARE",R{106,17,430,20},15,goldLight,false,juce::Justification::centredLeft,true);
  drawLabel(g,"COMPRESSEUR LEGENDS",R{106,40,565,44},38,cream,true);
  drawLabel(g,"SPECTRAL & MULTIBAND PRO  /  v0.6.0",R{108,86,500,20},15,muted);
  button(g,R{680,32,105,48},"RACK",[this]{screen=Screen::Rack;},false,screen==Screen::Rack);
  button(g,R{796,32,175,48},tr("BIBLIOTHÈQUE","LIBRARY"),[this]{libraryPage=0;screen=Screen::Library;},false,screen==Screen::Library);
  button(g,R{982,32,135,48},tr("ROUTAGE","ROUTING"),[this]{screen=Screen::Routing;},false,screen==Screen::Routing);
  button(g,R{1290,32,100,48},viewB?"A / [B]":"[A] / B",[this]{toggleAB();},false,viewB);
  button(g,R{1402,32,157,48},french?u("Français"):"English",[this]{french=!french;},false,false);
  button(g,R{1128,32,151,48},"PRESETS 400",[this]{presetPage=0;screen=Screen::Presets;},false,screen==Screen::Presets);
  if(p.learnTarget.load()>=0)drawLabel(g,tr("MIDI LEARN : bougez un CC","MIDI LEARN: move a CC"),R{710,83,650,28},18,amber,false,juce::Justification::centred);
  line(g,20,118,1580,118,goldDark,1.5f);
 }
 void board(juce::Graphics& g){
  using namespace clskin;bool par=p.state.getRawParameterValue("parallel")->load()>.5f;
  surface(g,R{12,126,1576,655},navy,midnight,10,goldDark);
  for(int i=0;i<8;++i){
   const int m=model(i);auto skin=finish(m);float x=20.f+(i%4)*393.f,y=134.f+(i/4)*321.f;R r{x,y,381,313};
   brushed(g,r,skin.top,skin.bottom,selected==i?gold:goldDark);line(g,x+18,y+38,x+363,y+38,modelTrim(m),1.6f);
   for(float sx:{x+9,x+372})for(float sy:{y+9,y+304})screw(g,sx,sy);
   drawLabel(g,juce::String(i+1).paddedLeft('0',2)+" / "+family(m),R{x+19,y+8,122,25},13.5f,skin.text,false,juce::Justification::centredLeft,true);
   surface(g,R{x+145,y+7,156,28},modelTrim(m).darker(.6f),juce::Colour(0xff18191b),4,modelTrim(m));
   drawLabel(g,liberty::models[m].name,R{x+149,y+8,148,26},17,cream,true,juce::Justification::centred,true);
   led(g,x+315,y+20,on(i));
   button(g,R{x+327,y+7,37,26},on(i)?"ON":"OFF",[this,i]{setRaw(i,"on",on(i)?0.f:1.f);},false,on(i),11);
   bool special=liberty::models[size_t(m)].architecture>=7||liberty::models[size_t(m)].architecture==5;
   if(!special){vents(g,R{x+24,y+61,31,43},skin.detail,on(i));vents(g,R{x+326,y+61,31,43},skin.detail,on(i));}
   display(g,special?R{x+24,y+42,333,88}:R{x+64,y+42,253,88},i,m);
   const char* keys[]={"threshold","ratio","attack","release","makeup","mix","color"};
   juce::String names[]={tr("SEUIL","THRESHOLD"),"RATIO",tr("ATTAQUE","ATTACK"),tr("RELÂCHEMENT","RELEASE"),"GAIN",tr("MÉLANGE","MIX"),tr("COULEUR","COLOR")};
   for(int k=0;k<7;++k){float px=x+48+(k%4)*95.f;float py=y+(k<4?151.f:224.f);rackKnob(g,i,keys[k],px,py,names[k],skin.text);}
   button(g,R{x+297,y+211,65,25},tr("ÉDITER","EDIT"),[this,i]{selected=i;screen=Screen::Detail;},false,false,11);
   button(g,R{x+297,y+242,65,25},tr("CHOISIR","LOAD"),[this,i]{selected=i;libraryPage=model(i)/8;screen=Screen::Library;},false,false,11);
   button(g,R{x+18,y+282,115,24},tr("HISTOIRE","HISTORY"),[this,i]{selected=i;historyPage=0;screen=Screen::History;},false,false,12);
   if(isProModel(m))button(g,R{x+140,y+282,106,24},raw(i,"pro")>.5f?"PRO ON":"PRO",[this,i]{selected=i;proBand=0;proEQPage=false;screen=Screen::Pro;},false,raw(i,"pro")>.5f,12);else button(g,R{x+140,y+282,106,24},raw(i,"analog")>.5f?"ANALOG ON":"ANALOG",[this,i]{selected=i;screen=Screen::Character;},false,raw(i,"analog")>.5f,12);
   button(g,R{x+258,y+282,46,24},"<",[this,i]{swapSlots(i,i-1);},false,false,15);
   button(g,R{x+311,y+282,46,24},">",[this,i]{swapSlots(i,i+1);},false,false,15);
  }
  brushed(g,R{12,786,1576,132},juce::Colour(0xff747a80),juce::Colour(0xff32363b),goldDark);
  screw(g,23,797);screw(g,1577,797);screw(g,23,907);screw(g,1577,907);
  drawLabel(g,"MASTER",R{35,810,142,38},28,cream,true,juce::Justification::centred,true);
  drawLabel(g,tr("ROUTAGE GLOBAL","GLOBAL ROUTING"),R{32,850,148,23},13,cream,false,juce::Justification::centred,true);
  line(g,185,803,185,902,goldDark);
  drawLabel(g,tr("MODE DE ROUTAGE","ROUTING MODE"),R{196,805,260,24},15,cream,false,juce::Justification::centred,true);
  button(g,R{201,839,118,39},tr("SÉRIE","SERIES"),[this]{set(p.state.getParameter("parallel"),0);},false,!par);
  button(g,R{325,839,135,39},tr("PARALLÈLE","PARALLEL"),[this]{set(p.state.getParameter("parallel"),1);},false,par);
  line(g,473,803,473,902,goldDark);
  auto* balance=p.state.getParameter("balance");machined(g,545,847,23,balance->getValue());hits.push_back({R{516,818,58,58},{},balance});
  drawLabel(g,"BALANCE A/B",R{578,818,166,25},15,cream,false,juce::Justification::centred,true);
  drawLabel(g,juce::String(int(balance->getValue()*100))+" % B",R{578,851,166,28},19,cream,false,juce::Justification::centred);
  line(g,755,803,755,902,goldDark);
  drawLabel(g,tr("RÉDUCTION DE GAIN / SLOTS","GAIN REDUCTION / SLOTS"),R{767,800,443,28},16,cream,false,juce::Justification::centred,true);
  for(int i=0;i<8;++i){R r{773.f+i*54,839,47,22};surface(g,r,ink,midnight,3,goldDark);float gr=p.meters[i].load();g.setColour(amber);g.fillRoundedRectangle(r.reduced(3).withWidth(41.f*std::clamp(gr/30.f,0.f,1.f)),2);drawLabel(g,juce::String(i+1),R{r.getX(),865,47,20},13,cream,false,juce::Justification::centred);}
  line(g,1210,803,1210,902,goldDark);
  auto* gain=p.state.getParameter("masterGain");machined(g,1262,846,23,gain->getValue());hits.push_back({R{1233,817,58,58},{},gain});
  drawLabel(g,tr("SORTIE","OUTPUT"),R{1219,877,86,17},13,cream,false,juce::Justification::centred,true);
  drawLabel(g,juce::String(gain->convertFrom0to1(gain->getValue()),1)+" dB",R{1305,826,89,32},15,cream,false,juce::Justification::centred);
  auto* bypass=p.state.getParameter("masterBypass");button(g,R{1400,809,160,39},viewB?"A / [B]":"[A] / B",[this]{toggleAB();},false,viewB);
  button(g,R{1400,857,160,39},tr("BYPASS GLOBAL","MASTER BYPASS"),[this,bypass]{set(bypass,bypass->getValue()>.5f?0.f:1.f);},false,bypass->getValue()>.5f,13);
 }
 void panel(juce::Graphics& g,const juce::String& title){using namespace clskin;surface(g,R{80,140,1440,758},sapphire,midnight,18.f,goldDark);drawLabel(g,title,R{112,157,1200,53},35,cream,true);button(g,R{1370,161,119,39},tr("RETOUR","BACK"),[this]{screen=Screen::Rack;});line(g,112,219,1488,219,goldDark,1.5f);}
 void detail(juce::Graphics& g){
  using namespace clskin;panel(g,tr("ÉDITION DU COMPRESSEUR","COMPRESSOR EDITOR")+"  /  "+juce::String(selected+1));
  enclosure(g,R{120,258,415,230},selected,model(selected),false);
  button(g,R{130,520,190,45},on(selected)?tr("ACTIF","ACTIVE"):"BYPASS",[this]{setRaw(selected,"on",on(selected)?0.f:1.f);},false,on(selected));
  button(g,R{332,520,190,45},tr("REMPLACER","REPLACE"),[this]{libraryPage=0;screen=Screen::Library;});
  button(g,R{130,579,392,45},tr("HISTOIRE ET CARACTÈRE","HISTORY AND CHARACTER"),[this]{historyPage=0;screen=Screen::History;});
  const char* keys[]={"threshold","ratio","attack","release","makeup","mix","color"};
  juce::String names[]={tr("Seuil","Threshold"),"Ratio",tr("Attaque","Attack"),tr("Relâchement","Release"),"Gain",tr("Mélange","Mix"),tr("Couleur","Color")};
  const char* units[]={" dB",":1"," ms"," ms"," dB","",""};
  for(int i=0;i<7;++i)knob(g,selected,keys[i],680.f+(i%4)*210.f,340.f+(i/4)*235.f,42,names[i],units[i]);
  button(g,R{130,642,392,45},raw(selected,"external")>.5f?"SIDECHAIN EXT":"SIDECHAIN INT",[this]{setRaw(selected,"external",raw(selected,"external")>.5f?0.f:1.f);},false,raw(selected,"external")>.5f);
  drawLabel(g,p.sidechainAvailable.load()?tr("Entrée externe active","External input active"):tr("Sans entrée externe : détection interne","No external input: internal detection"),R{130,692,392,35},16,muted);
  button(g,R{130,742,392,45},tr("COURBES & ANALYSE","CURVES & ANALYSIS"),[this]{screen=Screen::Analysis;});
  button(g,R{580,805,440,45},tr("TRAITEMENTS AVANCÉS","ADVANCED PROCESSING"),[this]{screen=Screen::Advanced;});
  if(isProModel(model(selected)))button(g,R{1050,805,420,45},"SPECTRAL / MULTIBAND PRO",[this]{proBand=0;proEQPage=false;screen=Screen::Pro;},false,raw(selected,"pro")>.5f);else button(g,R{1050,805,420,45},"ANALOG PERFECTION",[this]{screen=Screen::Character;},false,raw(selected,"analog")>.5f);
 }
 void library(juce::Graphics& g){
  using namespace clskin;panel(g,tr("BIBLIOTHÈQUE DE COMPRESSEURS","COMPRESSOR LIBRARY")+"  /  "+juce::String(selected+1));
  for(int j=0;j<8;++j){int i=libraryPage*8+j;if(i>=liberty::modelCount)break;R b{115.f+(j%4)*350,239.f+(j/4)*273,330,262};surface(g,b,navy,midnight,9.f,goldDark);enclosure(g,R{b.getX()+16,b.getY()+8,298,196},-1,i,true);button(g,R{b.getX()+30,b.getBottom()-47,270,35},tr("CHARGER","LOAD"),[this,i]{setRaw(selected,"model",float(i));setRaw(selected,"on",1);screen=Screen::Detail;});}
  for(int t=0;t<8;++t)button(g,R{360.f+t*110,825,95,43},juce::String(t+1)+" / 8",[this,t]{libraryPage=t;},false,libraryPage==t);
 }
 juce::String presetName(int i)const{return juce::String(liberty::categories[size_t((i%100)/10)].prefix)+" - "+liberty::variants[size_t(i%10)]+(i>=300?" Ultimate Pro":i>=200?" Ultimate":i>=100?" XL":"");}
 static juce::File favoritesFile(){return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("Liberty Music Software").getChildFile("Compresseur Legends").getChildFile("favorites.txt");}
 void readFavorites(){auto file=favoritesFile();if(!file.existsAsFile())return;auto s=file.loadFileAsString();for(int i=0;i<liberty::factoryCount;++i)favorites[size_t(i)]=i<s.length()&&s[i]=='1';}
 void toggleFavorite(int i){readFavorites();favorites[size_t(i)]=!favorites[size_t(i)];juce::String s;for(bool fav:favorites)s+=fav?"1":"0";auto file=favoritesFile();if(file.getParentDirectory().createDirectory().wasOk())file.replaceWithText(s);}
 void syncSearch(){presetSearch.setVisible(screen==Screen::Presets);presetSearch.setTextToShowWhenEmpty(tr("Rechercher un preset...","Search presets..."),clskin::muted);}
 void presets(juce::Graphics& g){
  using namespace clskin;panel(g,tr("PRESETS D'USINE","FACTORY PRESETS")+"  /  400");
  button(g,R{114,245,242,40},tr("TOUS","ALL"),[this]{presetCategory=-1;presetPage=0;},false,presetCategory<0);
  for(int c=0;c<liberty::categoryCount;++c){const auto& cat=liberty::categories[size_t(c)];button(g,R{114,293.f+c*43,242,37},u(french?cat.fr:cat.en),[this,c]{presetCategory=c;presetPage=0;},false,presetCategory==c);}
  button(g,R{114,742,242,43},tr("FAVORIS","FAVORITES"),[this]{onlyFavorites=!onlyFavorites;presetPage=0;},false,onlyFavorites);
  std::vector<int> found;auto query=presetSearch.getText().trim();
  for(int i=0;i<liberty::factoryCount;++i){const auto& cat=liberty::categories[size_t((i%100)/10)];auto info=presetName(i)+" "+u(cat.fr)+" "+u(cat.en)+" "+(i%10>=5?"Parallel Parallèle":"Series Série");if((presetCategory<0||(i%100)/10==presetCategory)&&(!onlyFavorites||favorites[size_t(i)])&&(query.isEmpty()||info.containsIgnoreCase(query)))found.push_back(i);}
  int pages=std::max(1,int((found.size()+9)/10));presetPage=juce::jlimit(0,pages-1,presetPage);
  drawLabel(g,tr("NOM DU PRESET","PRESET NAME"),R{390,300,480,31},17,muted,false,juce::Justification::centredLeft,true);
  drawLabel(g,tr("ROUTAGE","ROUTING"),R{970,300,178,31},17,muted,false,juce::Justification::centred,true);
  for(int j=0;j<10;++j){int pos=presetPage*10+j;if(pos>=int(found.size()))break;int i=found[size_t(pos)];float y=336.f+j*44.f;surface(g,R{382,y,1090,39},i==lastPreset?sapphire:navy,midnight,5.f,i==lastPreset?gold:goldDark);
   drawLabel(g,juce::String(i+1).paddedLeft('0',3),R{395,y+2,65,35},17,muted);
   drawLabel(g,presetName(i),R{470,y+2,480,35},20,cream,false,juce::Justification::centredLeft,true);
   drawLabel(g,i%10>=5?tr("PARALLÈLE","PARALLEL"):tr("SÉRIE","SERIES"),R{965,y+2,175,35},16,cream,false,juce::Justification::centred);
   button(g,R{1150,y+3,102,32},"FAV",[this,i]{toggleFavorite(i);},false,favorites[size_t(i)],14);
   button(g,R{1262,y+3,192,32},tr("CHARGER","LOAD"),[this,i]{p.loadFactory(i);lastPreset=i;selected=0;screen=Screen::Rack;},true,false,15);
  }
  if(found.empty())drawLabel(g,tr("Aucun résultat","No results"),R{382,365,1090,80},24,muted,false,juce::Justification::centred);
  button(g,R{382,802,85,43},"<",[this]{presetPage=std::max(0,presetPage-1);});
  button(g,R{600,802,85,43},">",[this,pages]{presetPage=std::min(pages-1,presetPage+1);});
  drawLabel(g,juce::String(presetPage+1)+" / "+juce::String(pages),R{477,802,110,43},19,cream,false,juce::Justification::centred,true);
  drawLabel(g,juce::String(int(found.size()))+" "+tr("résultats","results"),R{720,802,280,43},18,muted);
  drawLabel(g,tr("Favoris sauvegardés","Saved favorites"),R{1080,802,380,43},17,muted,false,juce::Justification::centredRight);
 }
 void history(juce::Graphics& g){
  using namespace clskin;panel(g,tr("HISTOIRE ET CARACTÈRE","HISTORY AND CHARACTER"));
  const int m=model(selected);const auto& entry=clhistory::entries[size_t(m)];
  enclosure(g,R{120,251,430,250},selected,m,false);
  drawLabel(g,tr("REPÈRES HISTORIQUES","HISTORICAL LANDMARKS"),R{127,517,416,27},19,muted,false,juce::Justification::centred,true);
  drawLabel(g,(m==7||m==18||m==19||(m>=30&&m<40)||m>=50)?tr("ÈRE NUMÉRIQUE","DIGITAL ERA"):u(entry.era),R{127,550,416,46},32,cream,true,juce::Justification::centred);
  drawLabel(g,u(entry.reference),R{127,607,416,38},20,cream,false,juce::Justification::centred,true);
  drawLabel(g,tr("Référence de famille, pas une copie","Family reference, not a clone"),R{127,650,416,27},17,muted,false,juce::Justification::centred);
  button(g,R{125,709,420,48},tr("ÉDITER LE COMPRESSEUR","EDIT COMPRESSOR"),[this]{screen=Screen::Detail;});
  const juce::String tabs[]={tr("HISTOIRE","HISTORY"),tr("TECHNOLOGIE","TECHNOLOGY"),tr("RÉGLAGES & USAGES","SETTINGS & USES")};
  for(int t=0;t<3;++t)button(g,R{580.f+t*299.f,247,284,46},tabs[t],[this,t]{historyPage=t;},false,historyPage==t,18);
  const char* frTitles[]={"Origines et évolution","Place dans les studios","Moteur Liberty","Caractère et limites","Point de départ","Conseils et chaînes"};
  const char* enTitles[]={"Origins and evolution","Studio legacy","Liberty engine","Character and limits","Starting point","Tips and chains"};
  const auto& paragraphs=french?entry.fr:entry.en;
  for(int i=0;i<2;++i){
   int k=historyPage*2+i;float y=315.f+i*218.f;R section{580,y,882,204};
   surface(g,section,navy,midnight,10,goldDark);
   drawLabel(g,tr(frTitles[k],enTitles[k]),R{603,y+13,836,34},26,cream,true);
   g.setColour(cream);g.setFont(juce::Font(juce::FontOptions(23.f)));
   g.drawFittedText(u(paragraphs[size_t(k)]),R{603,y+56,836,133}.toNearestInt(),juce::Justification::topLeft,5,.90f);
  }
  g.setColour(muted);g.setFont(juce::Font(juce::FontOptions(17.f)));
  g.drawFittedText(tr("Les valeurs proposées concernent les commandes Liberty. Ajustez le seuil au signal et comparez à volume égal. Les références historiques ne sont pas des émulations mesurées.","Suggested values refer to Liberty controls. Adjust threshold to the signal and compare at matched levels. Historical references are not measured emulations."),R{580,762,882,81}.toNearestInt(),juce::Justification::topLeft,3,.94f);
 }
 void routing(juce::Graphics& g){
  using namespace clskin;panel(g,tr("ROUTAGE DU SIGNAL","SIGNAL ROUTING"));
  bool par=p.state.getRawParameterValue("parallel")->load()>.5f;
  button(g,R{155,248,260,50},tr("SÉRIE","SERIES"),[this]{set(p.state.getParameter("parallel"),0);},false,!par);
  button(g,R{435,248,290,50},tr("PARALLÈLE A/B","PARALLEL A/B"),[this]{set(p.state.getParameter("parallel"),1);},false,par);
  for(int i=0;i<8;++i){R r{160.f+(par?i%4:i)*155.f,par?360.f+(i/4)*190.f:430.f,135,120};surface(g,r,color(model(i)),midnight,8.f,goldDark);drawLabel(g,juce::String(i+1)+" / "+family(model(i)),r.reduced(9),20,cream,false,juce::Justification::centred,true);if(i!=7&&(!par||i!=3))line(g,r.getRight(),r.getCentreY(),r.getRight()+20,r.getCentreY(),gold,4);}
  auto* a=p.state.getParameter("balance");knobDraw(g,820,739,36,a->getValue(),"BALANCE A/B",juce::String(int(a->getValue()*100))+" % B",true,a);
 }
 void advanced(juce::Graphics& g){
  using namespace clskin;panel(g,tr("TRAITEMENTS AVANCÉS","ADVANCED PROCESSING")+" / "+juce::String(selected+1));
  button(g,R{125,235,210,42},tr("ÉDITER","EDIT"),[this]{screen=Screen::Detail;});
  drawLabel(g,tr("TRAITEMENT DU SLOT","SLOT PROCESSING"),R{140,310,620,40},26,cream,true);
  const char* modes[]={"STEREO","MID","SIDE"};
  for(int i=0;i<3;++i)button(g,R{140.f+i*205,366,192,60},modes[i],[this,i]{setRaw(selected,"stereo",float(i));},false,int(raw(selected,"stereo"))==i);
  button(g,R{140,455,295,58},"AUTO GAIN",[this]{setRaw(selected,"auto",raw(selected,"auto")>.5f?0:1);},false,raw(selected,"auto")>.5f);
  button(g,R{450,455,295,58},"DELTA",[this]{setRaw(selected,"delta",raw(selected,"delta")>.5f?0:1);},false,raw(selected,"delta")>.5f);
  drawLabel(g,tr("Mid ou Side : l'autre composante reste intacte hors Auto Gain.","Mid or Side: other component unchanged except Auto Gain."),R{140,540,605,60},19,muted);
  drawLabel(g,tr("QUALITÉ GLOBALE DU RACK","GLOBAL RACK QUALITY"),R{825,310,600,40},26,cream,true);
  auto* q=p.state.getParameter("quality");for(int i=0;i<5;++i)button(g,R{825.f+i*120,366,108,60},juce::String(1<<i)+"x",[this,i]{auto* a=p.state.getParameter("quality");set(a,a->convertTo0to1(float(i)));},false,int(q->convertFrom0to1(q->getValue()))==i);
  auto* a=p.state.getParameter("lookahead");knobDraw(g,1000,535,43,a->getValue(),"LOOKAHEAD / SLOT",juce::String(a->convertFrom0to1(a->getValue()),1)+" ms",true,a);
  drawLabel(g,tr("Latence déclarée à l'hôte","Reported host latency"),R{1155,480,275,40},19,muted);
  drawLabel(g,juce::String(p.getLatencySamples())+" samples",R{1155,528,275,50},26,cream,true);
  drawLabel(g,tr("Modifier qualité, lookahead ou modèle spectral à l'arrêt. Latence cumulée sur huit slots, même en parallèle.","Change quality, lookahead or spectral model while stopped. Latency is aligned over eight slots, including parallel routing."),R{140,760,1290,65},20,muted,false,juce::Justification::centred);
 }
 void characterPage(juce::Graphics& g){
  using namespace clskin;int m=model(selected);auto profile=liberty::characterProfile(m);const bool enabled=raw(selected,"analog")>.5f;const bool hasHarmonics=liberty::analogCircuit(profile.circuit);const bool fet=profile.circuit==liberty::Circuit::FET;
  panel(g,"ANALOG PERFECTION / "+juce::String(selected+1));
  button(g,R{125,235,210,42},tr("ÉDITER","EDIT"),[this]{screen=Screen::Detail;});
  enclosure(g,R{130,302,448,252},selected,m,false);
  drawLabel(g,modelSerial(m)+"  /  "+family(m),R{130,566,448,38},24,cream,true,juce::Justification::centred);
  button(g,R{130,620,215,52},tr("COMPATIBILITÉ","LEGACY"),[this]{setRaw(selected,"analog",0.f);},false,!enabled);
  button(g,R{356,620,222,52},"ANALOG",[this]{setRaw(selected,"analog",1.f);},false,enabled);
  button(g,R{130,696,448,48},tr("COURBES & ANALYSE","CURVES & ANALYSIS"),[this]{screen=Screen::Analysis;});
  drawLabel(g,tr("CARACTÈRE DU MODÈLE","MODEL CHARACTER"),R{650,305,760,40},27,cream,true);
  knob(g,selected,"character",760,413,42,tr("Caractère","Character"),"");
  if(hasHarmonics)knob(g,selected,"color",1000,413,42,tr("Harmoniques","Harmonics"),"");
  else drawLabel(g,tr("Moteur propre, sans saturation ajoutée","Clean engine, no added saturation"),R{875,386,550,80},22,muted);
  if(fet)button(g,R{1200,387,244,52},"ALL BUTTONS",[this]{setRaw(selected,"allButtons",raw(selected,"allButtons")>.5f?0.f:1.f);},false,raw(selected,"allButtons")>.5f);
  const char* fr="Enveloppes et courbe personnalisées. Le caractère dose leur influence.";
  const char* en="Custom envelopes and curve. Character controls their influence.";
  switch(profile.circuit){
   case liberty::Circuit::FET:fr="FET : saturation impaire dominante, asymétrie contrôlée et mode All Buttons inspiré des limiteurs FET.";en="FET: dominant odd saturation, controlled asymmetry and FET-inspired All Buttons mode.";break;
   case liberty::Circuit::OPTO:fr="OPTO : mémoire de relâchement renforcée et saturation douce asymétrique.";en="OPTO: enhanced release memory and soft asymmetric saturation.";break;
   case liberty::Circuit::VCA:fr="VCA : enveloppes ajustées et coloration volontairement légère.";en="VCA: tailored envelopes and deliberately mild coloration.";break;
   case liberty::Circuit::TUBE:fr="Vari-Mu : ratio progressif dépendant du niveau et harmoniques paires renforcées.";en="Vari-Mu: level-dependent progressive ratio and enhanced even harmonics.";break;
   case liberty::Circuit::DIODE:fr="Diode : saturation de type arctangente et asymétrie propre au modèle.";en="Diode: arctangent saturation and model-specific asymmetry.";break;
   case liberty::Circuit::PWM:fr="PWM : courbe de gain inspirée du rapport cyclique, sans simulation de porteuse.";en="PWM: duty-inspired transfer without carrier simulation.";break;
   default:break;
  }
  g.setColour(cream);g.setFont(juce::Font(juce::FontOptions(23.f)));g.drawFittedText(tr(fr,en),R{650,560,790,115}.toNearestInt(),juce::Justification::topLeft,4,.9f);
  g.setColour(muted);g.setFont(juce::Font(juce::FontOptions(18.f)));g.drawFittedText(tr("En compatibilité, le rendu v0.4.1 est conservé. Analog active les nouveaux algorithmes. Les profils ne sont pas des émulations mesurées. Comparer à niveau égal ; utiliser l'oversampling pour la coloration prononcée.","Legacy preserves v0.4.1 processing. Analog enables the new algorithms. Profiles are not measured hardware emulations. Compare at matched level; use oversampling for strong coloration."),R{650,715,790,110}.toNearestInt(),juce::Justification::topLeft,4,.9f);
 }
 bool isProModel(int m){int a=liberty::models[size_t(m)].architecture;return a==5||a>=7;}
 void proKnob(juce::Graphics& g,const juce::String& key,float x,float y,float radius,const juce::String& name,const juce::String& unit,float multiplier=1){
  auto* a=param(selected,key.toRawUTF8());knobDraw(g,x,y,radius,a->getValue(),name,juce::String(raw(selected,key.toRawUTF8())*multiplier,unit==" Hz"?0:1)+unit,true,a);
 }
 void proPage(juce::Graphics& g){
  using namespace clskin;int m=model(selected);bool spectral=liberty::models[size_t(m)].architecture==9;
  panel(g,spectral?"SPECTRAL PRO":"MULTIBAND PRO");
  button(g,R{125,235,145,40},tr("ÉDITER","EDIT"),[this]{screen=Screen::Detail;});
  button(g,R{280,235,180,40},tr("COMPATIBILITÉ","LEGACY"),[this]{setRaw(selected,"pro",0);setRaw(selected,"scListen",0);},false,raw(selected,"pro")<.5f,15);
  button(g,R{470,235,110,40},"PRO",[this]{setRaw(selected,"pro",1);},false,raw(selected,"pro")>.5f);
  button(g,R{590,235,180,40},tr("BANDES","BANDS"),[this]{proEQPage=false;},false,!proEQPage);
  button(g,R{780,235,200,40},"SIDECHAIN EQ",[this]{proEQPage=true;},false,proEQPage);
  button(g,R{990,235,175,40},"ANALOG",[this]{screen=Screen::Character;});
  drawLabel(g,juce::String(p.getLatencySamples())+" samples",R{1180,235,285,40},18,muted,false,juce::Justification::centredRight);
  if(proEQPage){
   drawLabel(g,tr("FILTRES DU SIGNAL DE DÉTECTION","DETECTOR SIGNAL FILTERS"),R{140,305,1300,44},27,cream,true);
   proKnob(g,"scHP",250,450,46,"HP"," Hz");proKnob(g,"scLP",480,450,46,"LP"," Hz");proKnob(g,"scBell",710,450,46,tr("Fréquence","Frequency")," Hz");proKnob(g,"scGain",940,450,46,tr("Gain cloche","Bell gain")," dB");proKnob(g,"link",1170,450,46,"Stereo Link"," %",100);
   button(g,R{450,645,610,57},tr("ÉCOUTE SIDECHAIN","SIDECHAIN LISTEN"),[this]{setRaw(selected,"scListen",raw(selected,"scListen")>.5f?0:1);},false,raw(selected,"scListen")>.5f);
   drawLabel(g,tr("Écoute au niveau du slot : les modules suivants restent actifs. Les filtres ne traitent que la détection.","Slot monitoring: downstream modules remain active. Filters affect only detection."),R{130,755,1340,70},20,muted,false,juce::Justification::centred);
   return;
  }
  int bands=juce::jlimit(2,6,int(raw(selected,"bandCount")));proBand=juce::jlimit(0,bands-1,proBand);
  drawLabel(g,tr("BANDES","BANDS"),R{135,292,130,35},17,muted);for(int i=2;i<=6;++i)button(g,R{270.f+(i-2)*65,292,58,35},juce::String(i),[this,i]{setRaw(selected,"bandCount",float(i));},false,bands==i,15);
  if(spectral){drawLabel(g,"FFT",R{640,292,65,35},18,muted);for(int i=0;i<4;++i)button(g,R{710.f+i*118,292,110,35},juce::String(512<<i),[this,i]{setRaw(selected,"fft",float(i));},false,int(raw(selected,"fft"))==i,15);}
  drawLabel(g,tr("Cliquer sur une bande pour l'éditer","Click a band to edit"),R{1190,290,277,39},16,muted,false,juce::Justification::centredRight);
  liberty::ProSettings config;config.count=bands;for(int i=0;i<5;++i)config.cross[i]=raw(selected,Processor::crossKey(i).toRawUTF8());auto cross=liberty::proCross(config,p.processingRate.load());
  R plot{140,345,1315,235};surface(g,plot,navy,midnight,6,goldDark);
  float nyquist=std::max(1000.f,p.processingRate.load()*.5f),maxHz=std::min(20000.f,nyquist);
  auto px=[&](float hz){return plot.getX()+plot.getWidth()*std::clamp(std::log(std::max(20.f,hz)/20.f)/std::log(maxHz/20.f),0.f,1.f);};
  float left=plot.getX();for(int b=0;b<bands;++b){float right=b==bands-1?plot.getRight():px(cross[b]);if(right>left){R zone{left,plot.getY()+3,right-left,plot.getHeight()-6};g.setColour((b==proBand?gold:amber).withAlpha(b==proBand?.20f:.055f));g.fillRect(zone);hit(zone,[this,b]{proBand=b;});line(g,right,plot.getY(),right,plot.getBottom(),goldLight.withAlpha(.5f));float gr=p.proMeters[selected][b].load();g.setColour(amber.withAlpha(.70f));float h=std::clamp(gr/30.f,0.f,1.f)*155;g.fillRect(R{left+2,plot.getY()+43,std::max(0.f,right-left-4),h});drawLabel(g,"B"+juce::String(b+1)+" / "+juce::String(gr,1)+" dB",R{left+2,plot.getY()+8,std::max(0.f,right-left-4),24},15,cream,false,juce::Justification::centred);}left=right;}
  if(spectral)for(int k=0;k<16;++k){float hz=(k+.5f)*nyquist/16;float x=px(hz);float h=std::clamp(p.displayMeters[selected][k].load()/30.f,0.f,1.f)*150;line(g,x,plot.getY()+43,x,plot.getY()+43+h,goldLight,2);}
  for(float hz:{20.f,100.f,1000.f,10000.f,20000.f}){float x=px(hz);drawLabel(g,juce::String(int(hz)),R{x-32,plot.getBottom()-24,64,20},12,muted,false,juce::Justification::centred);}
  float y=622;int b=proBand;
  proKnob(g,Processor::bandKey(b,"threshold"),245,y+44,32,tr("Seuil","Threshold")," dB");proKnob(g,Processor::bandKey(b,"ratio"),425,y+44,32,"Ratio",":1");proKnob(g,Processor::bandKey(b,"attack"),605,y+44,32,tr("Attaque","Attack")," ms");proKnob(g,Processor::bandKey(b,"release"),785,y+44,32,tr("Relâchement","Release")," ms");
  const char* fields[]={"solo","mute","bypass"};const char* labels[]={"SOLO","MUTE","BYPASS"};for(int k=0;k<3;++k){auto key=Processor::bandKey(b,fields[k]);button(g,R{950.f+k*165,y+20,153,42},labels[k],[this,key]{setRaw(selected,key.toRawUTF8(),raw(selected,key.toRawUTF8())>.5f?0:1);},false,raw(selected,key.toRawUTF8())>.5f);}
  if(b<bands-1)proKnob(g,Processor::crossKey(b),1090,755,25,tr("Crossover haut","Upper crossover")," Hz");
  if(b>0)proKnob(g,Processor::crossKey(b-1),1300,755,25,tr("Crossover bas","Lower crossover")," Hz");
  drawLabel(g,tr("Modifier FFT et nombre de bandes à l'arrêt. Les crossovers sont triés dans le DSP.","Change FFT and band count while stopped. Crossovers are sorted in DSP."),R{140,825,815,45},16,muted);
 }
 void analysis(juce::Graphics& g){
  using namespace clskin;panel(g,tr("ANALYSE DE LA COMPRESSION","COMPRESSION ANALYSIS")+"  /  "+juce::String(selected+1));
  const int m=model(selected);liberty::Parameters settings;settings.model=m;settings.threshold=raw(selected,"threshold");settings.ratio=raw(selected,"ratio");settings.makeup=raw(selected,"makeup");settings.mix=raw(selected,"mix");settings.analog=raw(selected,"analog")>.5f;settings.character=raw(selected,"character");settings.allButtons=raw(selected,"allButtons")>.5f;const auto md=liberty::characterModel(settings);
  button(g,R{125,235,210,42},tr("ÉDITER","EDIT"),[this]{screen=Screen::Detail;});
  drawLabel(g,liberty::models[m].name,R{360,235,600,42},24,cream,true);
  R a{150,320,560,430},b{825,320,610,430};surface(g,a,navy,midnight,8,goldDark);surface(g,b,navy,midnight,8,goldDark);
  auto point=[&](float x,float y){return juce::Point<float>(a.getX()+(x+60)/60*a.getWidth(),a.getBottom()-(y+60)/60*a.getHeight());};
  for(int n=0;n<=6;++n){float k=-60+10.f*n;auto q=point(k,k);line(g,q.x,a.getY(),q.x,a.getBottom(),muted.withAlpha(.18f));line(g,a.getX(),q.y,a.getRight(),q.y,muted.withAlpha(.18f));drawLabel(g,juce::String(int(k)),R{q.x-20,a.getBottom()+5,40,23},13,muted,false,juce::Justification::centred);drawLabel(g,juce::String(int(k)),R{a.getX()-45,q.y-10,40,20},13,muted,false,juce::Justification::centredRight);}
  line(g,a.getX(),a.getBottom(),a.getRight(),a.getY(),muted.withAlpha(.55f),1.5f);
  juce::Path path;for(int i=0;i<=120;++i){float x=-60.f+i*.5f;float y=x-liberty::nominalReduction(x,md,settings,raw(selected,"external")>.5f&&p.sidechainAvailable.load());auto q=point(x,std::clamp(y,-60.f,0.f));if(i==0)path.startNewSubPath(q);else path.lineTo(q);}
  g.setColour(amber);g.strokePath(path,juce::PathStrokeType(3.f));
  const float live=std::clamp(liberty::db(p.detectors[selected].load()),-60.f,0.f),gr=p.meters[selected].load();auto q=point(live,std::clamp(live-gr,-60.f,0.f));g.setColour(gold);g.fillEllipse(q.x-5,q.y-5,10,10);
  for(int n=0;n<=6;++n){float yy=b.getY()+b.getHeight()*n/6;line(g,b.getX(),yy,b.getRight(),yy,muted.withAlpha(.2f));drawLabel(g,juce::String(-n*5),R{b.getX()-45,yy-10,40,20},13,muted,false,juce::Justification::centredRight);}
  juce::Path trace;for(int i=0;i<120;++i){float x=b.getX()+b.getWidth()*i/119;float y=b.getY()+b.getHeight()*juce::jlimit(0.f,1.f,grHistory[selected][(grHead+i)%120]/30.f);if(i==0)trace.startNewSubPath(x,y);else trace.lineTo(x,y);}g.setColour(goldLight);g.strokePath(trace,juce::PathStrokeType(2.f));
  drawLabel(g,tr("Courbe nominale avant gain et mélange","Nominal curve before gain and mix"),R{150,280,560,32},20,cream,false,juce::Justification::centred);
  drawLabel(g,tr("Réduction réelle - dernières 6 s","Actual reduction - last 6 s"),R{825,280,610,32},20,cream,false,juce::Justification::centred);
  drawLabel(g,tr("Point rouge : niveau détecté et réduction réelle. Modes adaptatifs, filtrés et deux bandes : réponse dépendante du signal.","Red dot: detector level and actual gain reduction. Adaptive, filtered and dual-band modes are signal-dependent."),R{120,798,1370,50},19,muted,false,juce::Justification::centred);
 }
 void timerCallback()override{for(int i=0;i<8;++i)grHistory[i][grHead]=p.meters[i].load();grHead=(grHead+1)%120;repaint();}
 juce::Point<float> logical(juce::Point<float> pt)const{return {(pt.x-xOffset)/scale,(pt.y-yOffset)/scale};}
public:
 explicit Editor(Processor& processor):AudioProcessorEditor(&processor),p(processor){
  setOpaque(true);readFavorites();
  // Preserve the Legends canvas ratio; fit the available desktop, not physical pixel count.
  int availableW=1600,availableH=930;
  if(auto* display=juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()){
   availableW=std::max(480,display->userArea.getWidth()-32);
   availableH=std::max(280,display->userArea.getHeight()-72);
  }
  const float maxScale=std::min({1.25f,availableW/1600.f,availableH/930.f});
  const float minScale=std::min(.70f,maxScale);
  const float initialScale=std::min(.90f,maxScale);
  setResizable(true,true);
  setResizeLimits(int(std::floor(1600*minScale)),int(std::floor(930*minScale)),int(std::floor(1600*maxScale)),int(std::floor(930*maxScale)));
  if(auto* c=getConstrainer())c->setFixedAspectRatio(1600.0/930.0);
  presetSearch.setColour(juce::TextEditor::backgroundColourId,clskin::navy);presetSearch.setColour(juce::TextEditor::textColourId,clskin::cream);presetSearch.setColour(juce::TextEditor::outlineColourId,clskin::goldDark);presetSearch.onTextChange=[this]{presetPage=0;repaint();};addChildComponent(presetSearch);syncSearch();
  snapshotA=p.state.copyState();snapshotB=snapshotA.createCopy();
  setSize(int(std::floor(1600*initialScale)),int(std::floor(930*initialScale)));startTimerHz(20);
 }
 ~Editor()override{stopTimer();if(dragging)dragging->endChangeGesture();}
 void resized()override{scale=std::min(getWidth()/1600.f,getHeight()/930.f);xOffset=(getWidth()-1600*scale)*.5f;yOffset=(getHeight()-930*scale)*.5f;presetSearch.setBounds(juce::Rectangle<float>{382,245,1090,43}.transformedBy(juce::AffineTransform::scale(scale).translated(xOffset,yOffset)).toNearestInt());presetSearch.setFont(juce::Font(juce::FontOptions(20.f*scale)));}
 void paint(juce::Graphics& g)override{g.fillAll(clskin::midnight);hits.clear();juce::Graphics::ScopedSaveState save(g);g.addTransform(juce::AffineTransform::scale(scale).translated(xOffset,yOffset));backplate(g);switch(screen){case Screen::Rack:board(g);break;case Screen::Detail:detail(g);break;case Screen::Library:library(g);break;case Screen::History:history(g);break;case Screen::Routing:routing(g);break;case Screen::Presets:presets(g);break;case Screen::Analysis:analysis(g);break;case Screen::Advanced:advanced(g);break;case Screen::Character:characterPage(g);break;case Screen::Pro:proPage(g);break;}}
 void mouseDown(const juce::MouseEvent& e)override{auto pt=logical(e.position);if(e.mods.isPopupMenu()){for(auto it=hits.rbegin();it!=hits.rend();++it)if(it->bounds.contains(pt)&&it->param){int idx=p.midiIndex(it->param);if(idx<0)return;juce::PopupMenu menu;menu.addItem(1,"MIDI Learn");menu.addItem(2,tr("Effacer l'affectation","Clear mapping"),p.midiCC[idx].load()>=0);menu.addItem(3,tr("Annuler l'apprentissage","Cancel learn"),p.learnTarget.load()>=0);auto safe=juce::Component::SafePointer<Editor>(this);menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),[safe,idx](int choice){if(safe==nullptr)return;if(choice==1)safe->p.learnTarget.store(idx);if(choice==2)safe->p.midiCC[idx].store(-1);if(choice==3)safe->p.learnTarget.store(-1);safe->repaint();});return;}return;}if(!e.mods.isLeftButtonDown())return;for(auto it=hits.rbegin();it!=hits.rend();++it)if(it->bounds.contains(pt)){if(it->param){dragging=it->param;startValue=dragging->getValue();startY=pt.y;dragging->beginChangeGesture();}else if(it->click){auto f=it->click;f();syncSearch();}repaint();return;}}
 void mouseDrag(const juce::MouseEvent& e)override{if(dragging){dragging->setValueNotifyingHost(juce::jlimit(0.f,1.f,startValue+(startY-logical(e.position).y)/160.f));repaint();}}
 void mouseUp(const juce::MouseEvent&)override{if(dragging){dragging->endChangeGesture();dragging=nullptr;}}
 void mouseDoubleClick(const juce::MouseEvent& e)override{auto pt=logical(e.position);for(auto it=hits.rbegin();it!=hits.rend();++it)if(it->bounds.contains(pt)){if(it->param)set(it->param,it->param->getDefaultValue());return;}}
};
