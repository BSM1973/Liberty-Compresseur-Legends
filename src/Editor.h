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
 enum class Screen{Rack,Detail,Library,History,Routing,Presets,Analysis};
 Screen screen=Screen::Rack;
 int selected=0;int historyPage=0,libraryPage=0,presetPage=0,presetCategory=-1,lastPreset=-1;bool onlyFavorites=false;std::array<bool,liberty::factoryCount> favorites{};juce::TextEditor presetSearch;
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
 void swapSlots(int a,int b){if(b<0||b>=8)return;for(auto key:{"on","model","threshold","ratio","attack","release","makeup","mix","color","external"}){float x=raw(a,key),y=raw(b,key);setRaw(a,key,y);setRaw(b,key,x);}selected=b;}
 void toggleAB(){if(viewB){snapshotB=p.state.copyState();p.state.replaceState(snapshotA.createCopy());}else{snapshotA=p.state.copyState();p.state.replaceState(snapshotB.createCopy());}viewB=!viewB;}
 static juce::Colour color(int m){constexpr unsigned c[]={0xff8d2638,0xff655e50,0xff58616b,0xff536677,0xff643654,0xff286054,0xff756956,0xff41454d,0xff8d2638,0xff763344,0xff6c6352,0xff756956,0xff58616b,0xff536677,0xff643654,0xff75445e,0xff286054,0xff31675b,0xff41454d,0xff4b505b,0xff8d2638,0xff763344,0xff655e50,0xff756956,0xff58616b,0xff536677,0xff643654,0xff75445e,0xff286054,0xff31675b,0xff41454d,0xff4b505b,0xff41454d,0xff4b505b,0xff64538e,0xff64538e,0xff7462b0,0xff64538e,0xff41454d,0xff4b505b};return juce::Colour(c[juce::jlimit(0,liberty::modelCount-1,m)]);}
 static const char* family(int m){constexpr const char* f[]={"FET","OPTO","VCA","VCA","VARI-MU","DIODE","OPTO","DIGITAL","FET","FET","OPTO","OPTO","VCA","VCA","VARI-MU","VARI-MU","DIODE","DIODE","DIGITAL","DIGITAL","FET","FET","OPTO","OPTO","VCA","VCA","VARI-MU","VARI-MU","DIODE","DIODE","DIGITAL","DIGITAL","DIGITAL","DIGITAL","DUAL BAND","DUAL BAND","DUAL BAND","DUAL BAND","DIGITAL","DIGITAL"};return f[m];}
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
    const R shadow{x-radius+2.f,y-radius+3.f,radius*2,radius*2};
    g.setColour(midnight.withAlpha(.7f));g.fillEllipse(shadow);
    g.setGradientFill(juce::ColourGradient(goldLight,{x-radius,y-radius},goldDark,{x+radius,y+radius},false));
    g.fillEllipse(x-radius,y-radius,2*radius,2*radius);
    g.setColour(ink);g.drawEllipse(x-radius+3.f,y-radius+3.f,2*radius-6,2*radius-6,2.f);
    g.setGradientFill(juce::ColourGradient(cream,{x-radius*.65f,y-radius*.8f},gold,{x+radius*.6f,y+radius*.8f},false));
    g.fillEllipse(x-radius*.7f,y-radius*.7f,radius*1.4f,radius*1.4f);
    const float angle=(.75f+std::clamp(value,0.f,1.f)*1.5f)*pi;
    line(g,x,y,x+std::cos(angle)*radius*.62f,y+std::sin(angle)*radius*.62f,ink,2.7f);
    for(int k=0;k<=10;++k) {
        const float a=(.75f+1.5f*float(k)/10.f)*pi;
        const auto dx=std::cos(a),dy=std::sin(a);
        line(g,x+dx*(radius+2.f),y+dy*(radius+2.f),x+dx*(radius+6.f),y+dy*(radius+6.f),goldDark,1.f);
    }
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
  auto* a=param(slot,key);knobDraw(g,x,y,radius,a->getValue(),title,juce::String(raw(slot,key),1)+suffix,true,a);
 }
 void meter(juce::Graphics& g,R r,float gr){
  using namespace clskin;surface(g,r,paper,paper.darker(.22f),7.f,goldDark);
  const float x=r.getCentreX(),y=r.getBottom()-13,rad=std::min(r.getWidth()*.42f,r.getHeight()*.76f);
  for(int i=0;i<=12;++i){float angle=(-155.f+130.f*i/12)*pi/180;float c=std::cos(angle),s=std::sin(angle);line(g,x+c*(rad-8),y+s*(rad-8),x+c*rad,y+s*rad,ink,1.2f);}
  float a=(-155.f+130.f*(1.f-juce::jlimit(0.f,30.f,gr)/30.f))*pi/180;
  line(g,x,y,x+std::cos(a)*(rad-3),y+std::sin(a)*(rad-3),gold,2.f);
  g.setColour(ink);g.fillEllipse(x-4,y-4,8,8);drawLabel(g,"GR",R{x-22,r.getY()+4,44,16},12,ink,false,juce::Justification::centred,true);
 }
 void enclosure(juce::Graphics& g,R r,int slot,int m,bool mini){
  using namespace clskin;auto c=color(m);surface(g,r,c.brighter(.15f),c.darker(.3f),12.f,goldDark);
  for(float y=r.getY()+12;y<r.getBottom()-10;y+=4)line(g,r.getX()+7,y,r.getRight()-7,y,cream.withAlpha(.025f));
  for(float x:{r.getX()+13,r.getRight()-13})for(float y:{r.getY()+12,r.getBottom()-12}){g.setColour(goldLight);g.fillEllipse(x-3,y-3,6,6);line(g,x-2,y+1,x+2,y-1,ink,1);}
  drawLabel(g,liberty::models[m].name,R{r.getX()+18,r.getY()+8,r.getWidth()-36,32},mini?23:31,cream,true,juce::Justification::centred,true);
  drawLabel(g,family(m),R{r.getX()+18,r.getY()+40,r.getWidth()-36,18},13,cream,false,juce::Justification::centred,true);
  float mx=r.getCentreX();meter(g,R{mx-63,r.getY()+64,126,72},slot>=0?p.meters[slot].load():0.f);
  if(slot>=0){drawLabel(g,juce::String(p.meters[slot].load(),1)+" dB",R{mx-67,r.getY()+141,134,24},16,cream,false,juce::Justification::centred,true);
   g.setColour(on(slot)?amber:muted.darker());g.fillEllipse(r.getX()+18,r.getBottom()-25,9,9);
   drawLabel(g,on(slot)?tr("ACTIF","ACTIVE"):"BYPASS",R{r.getX()+32,r.getBottom()-30,85,20},12,cream,false,juce::Justification::centredLeft,true);}
 }
 void backplate(juce::Graphics& g){
  using namespace clskin;
  g.setGradientFill(juce::ColourGradient(sapphire,{0.f,0.f},midnight,{0.f,930.f},false));g.fillRect(R{0,0,1600,930});
  for(int n=1;n<100;++n)line(g,0.f,float(n)*9.35f,1600.f,float(n)*9.35f,cream.withAlpha(n%3==0?.021f:.007f));
  juce::Path badge;badge.addQuadrilateral(61,27,85,54,61,80,37,54);g.setColour(gold);g.strokePath(badge,juce::PathStrokeType(3.f));
  drawLabel(g,"L",R{45,32,33,41},30,goldLight,true,juce::Justification::centred,true);
  drawLabel(g,"LIBERTY MUSIC SOFTWARE",R{106,17,430,20},15,goldLight,false,juce::Justification::centredLeft,true);
  drawLabel(g,"COMPRESSEUR LEGENDS",R{106,40,565,44},38,cream,true);
  drawLabel(g,"TITANIUM RED  /  v0.3.0",R{108,86,500,20},15,muted);
  button(g,R{680,32,105,48},"RACK",[this]{screen=Screen::Rack;},false,screen==Screen::Rack);
  button(g,R{796,32,175,48},tr("BIBLIOTHÈQUE","LIBRARY"),[this]{libraryPage=0;screen=Screen::Library;},false,screen==Screen::Library);
  button(g,R{982,32,135,48},tr("ROUTAGE","ROUTING"),[this]{screen=Screen::Routing;},false,screen==Screen::Routing);
  button(g,R{1290,32,100,48},viewB?"A / [B]":"[A] / B",[this]{toggleAB();},false,viewB);
  button(g,R{1402,32,157,48},french?u("Français"):"English",[this]{french=!french;},false,false);
  button(g,R{1128,32,151,48},"PRESETS 200",[this]{presetPage=0;screen=Screen::Presets;},false,screen==Screen::Presets);
  if(p.learnTarget.load()>=0)drawLabel(g,tr("MIDI LEARN : bougez un CC","MIDI LEARN: move a CC"),R{710,83,650,28},18,amber,false,juce::Justification::centred);
  line(g,20,118,1580,118,goldDark,1.5f);
 }
 void board(juce::Graphics& g){
  using namespace clskin;surface(g,R{28,139,1544,641},navySoft,midnight,18.f,goldDark);
  drawLabel(g,tr("COLLECTION LIBERTY","LIBERTY COLLECTION"),R{65,146,310,27},19,cream,false,juce::Justification::centredLeft,true);
  drawLabel(g,tr("40 PROFILS EXPÉRIMENTAUX","40 EXPERIMENTAL PROFILES"),R{390,146,440,27},17,muted);
  bool par=p.state.getRawParameterValue("parallel")->load()>.5f;
  drawLabel(g,par?"IN > SPLIT A/B > MIX > OUT":"IN > 1 > 2 > 3 > 4 > 5 > 6 > 7 > 8 > OUT",R{860,146,675,27},16,cream,false,juce::Justification::centredRight,true);
  std::array<R,8> boxes;for(int i=0;i<8;++i)boxes[i]=R{56.f+(i%4)*381.f,177.f+(i/4)*294.f,355.f,281.f};
  for(int i=0;i<7;++i)if(i!=3){auto a=boxes[i],b=boxes[i+1];line(g,a.getRight()-12,a.getCentreY(),b.getX()+12,b.getCentreY(),gold,5.f);}
  if(!par){juce::Path cable;cable.startNewSubPath(1560,340);cable.lineTo(1560,466);cable.lineTo(42,466);cable.lineTo(42,610);cable.lineTo(72,610);g.setColour(gold);g.strokePath(cable,juce::PathStrokeType(5.f));}
  for(int i=0;i<8;++i){
   auto b=boxes[i];surface(g,b,navy,midnight,10.f,selected==i?gold:goldDark);
   drawLabel(g,juce::String(i+1).paddedLeft('0',2),R{b.getX()+13,b.getY()+7,45,27},23,cream,true);
   drawLabel(g,family(model(i)),R{b.getX()+61,b.getY()+9,100,23},14,muted,false,juce::Justification::centredLeft,true);
   button(g,R{b.getRight()-180,b.getY()+8,102,25},tr("HISTOIRE","HISTORY"),[this,i]{selected=i;historyPage=0;screen=Screen::History;},false,false,13);
   button(g,R{b.getRight()-72,b.getY()+8,62,25},on(i)?"ON":"OFF",[this,i]{setRaw(i,"on",on(i)?0.f:1.f);},false,on(i),13);
   R unit{b.getX()+37,b.getY()+39,b.getWidth()-74,202};enclosure(g,unit,i,model(i),true);hit(unit,[this,i]{selected=i;screen=Screen::Detail;});
   button(g,R{b.getX()+15,b.getBottom()-34,123,26},tr("ÉDITER","EDIT"),[this,i]{selected=i;screen=Screen::Detail;},false,false,13);
   button(g,R{b.getX()+144,b.getBottom()-34,118,26},tr("CHOISIR","CHOOSE"),[this,i]{selected=i;libraryPage=0;screen=Screen::Library;},false,false,13);
   button(g,R{b.getX()+269,b.getBottom()-34,32,26},"<",[this,i]{swapSlots(i,i-1);});
   button(g,R{b.getX()+307,b.getBottom()-34,32,26},">",[this,i]{swapSlots(i,i+1);});
  }
  surface(g,R{23,790,1553,126},sapphire,midnight,8.f,goldDark);
  drawLabel(g,tr("SIGNAL MASTER","MASTER SIGNAL"),R{42,816,225,52},22,cream,true);
  button(g,R{288,820,168,48},tr("SÉRIE","SERIES"),[this]{set(p.state.getParameter("parallel"),0);},false,!par);
  button(g,R{469,820,205,48},tr("PARALLÈLE","PARALLEL"),[this]{set(p.state.getParameter("parallel"),1);},false,par);
  auto* b=p.state.getParameter("balance");knobDraw(g,760,823,20,b->getValue(),"BALANCE A/B",juce::String(int(b->getValue()*100))+" % B",true,b);
  drawLabel(g,tr("RÉDUCTION DE GAIN","GAIN REDUCTION"),R{914,808,340,27},17,cream,false,juce::Justification::centredLeft,true);
  for(int i=0;i<8;++i){R r{915.f+i*78,850,68,18};g.setColour(midnight);g.fillRoundedRectangle(r,3);g.setColour(gold);g.fillRoundedRectangle(r.withWidth(r.getWidth()*juce::jlimit(0.f,1.f,p.meters[i].load()/30.f)),3);drawLabel(g,juce::String(i+1),R{r.getX(),874,68,18},13,muted,false,juce::Justification::centred);}
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
  drawLabel(g,tr("Clic droit sur un potentiomètre : MIDI Learn.","Right-click a knob: MIDI Learn."),R{120,805,1360,45},19,muted,false,juce::Justification::centred);
 }
 void library(juce::Graphics& g){
  using namespace clskin;panel(g,tr("BIBLIOTHÈQUE DE COMPRESSEURS","COMPRESSOR LIBRARY")+"  /  "+juce::String(selected+1));
  for(int j=0;j<8;++j){int i=libraryPage*8+j;if(i>=liberty::modelCount)break;R b{115.f+(j%4)*350,239.f+(j/4)*273,330,262};surface(g,b,navy,midnight,9.f,goldDark);enclosure(g,R{b.getX()+16,b.getY()+8,298,196},-1,i,true);button(g,R{b.getX()+30,b.getBottom()-47,270,35},tr("CHARGER","LOAD"),[this,i]{setRaw(selected,"model",float(i));setRaw(selected,"on",1);screen=Screen::Detail;});}
  for(int t=0;t<5;++t)button(g,R{530.f+t*110,825,95,43},juce::String(t+1)+" / 5",[this,t]{libraryPage=t;},false,libraryPage==t);
 }
 juce::String presetName(int i)const{return juce::String(liberty::categories[size_t((i%100)/10)].prefix)+" - "+liberty::variants[size_t(i%10)]+(i>=100?" XL":"");}
 static juce::File favoritesFile(){return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("Liberty Music Software").getChildFile("Compresseur Legends").getChildFile("favorites.txt");}
 void readFavorites(){auto file=favoritesFile();if(!file.existsAsFile())return;auto s=file.loadFileAsString();for(int i=0;i<liberty::factoryCount;++i)favorites[size_t(i)]=i<s.length()&&s[i]=='1';}
 void toggleFavorite(int i){readFavorites();favorites[size_t(i)]=!favorites[size_t(i)];juce::String s;for(bool fav:favorites)s+=fav?"1":"0";auto file=favoritesFile();if(file.getParentDirectory().createDirectory().wasOk())file.replaceWithText(s);}
 void syncSearch(){presetSearch.setVisible(screen==Screen::Presets);presetSearch.setTextToShowWhenEmpty(tr("Rechercher un preset...","Search presets..."),clskin::muted);}
 void presets(juce::Graphics& g){
  using namespace clskin;panel(g,tr("PRESETS D'USINE","FACTORY PRESETS")+"  /  200");
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
  drawLabel(g,(m==7||m==18||m==19||m>=30)?tr("ÈRE NUMÉRIQUE","DIGITAL ERA"):u(entry.era),R{127,550,416,46},32,cream,true,juce::Justification::centred);
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
 void analysis(juce::Graphics& g){
  using namespace clskin;panel(g,tr("ANALYSE DE LA COMPRESSION","COMPRESSION ANALYSIS")+"  /  "+juce::String(selected+1));
  const int m=model(selected);const auto& md=liberty::models[size_t(m)];liberty::Parameters settings;settings.model=m;settings.threshold=raw(selected,"threshold");settings.ratio=raw(selected,"ratio");settings.makeup=raw(selected,"makeup");settings.mix=raw(selected,"mix");
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
 void paint(juce::Graphics& g)override{g.fillAll(clskin::midnight);hits.clear();juce::Graphics::ScopedSaveState save(g);g.addTransform(juce::AffineTransform::scale(scale).translated(xOffset,yOffset));backplate(g);switch(screen){case Screen::Rack:board(g);break;case Screen::Detail:detail(g);break;case Screen::Library:library(g);break;case Screen::History:history(g);break;case Screen::Routing:routing(g);break;case Screen::Presets:presets(g);break;case Screen::Analysis:analysis(g);break;}}
 void mouseDown(const juce::MouseEvent& e)override{auto pt=logical(e.position);if(e.mods.isPopupMenu()){for(auto it=hits.rbegin();it!=hits.rend();++it)if(it->bounds.contains(pt)&&it->param){int idx=p.midiIndex(it->param);if(idx<0)return;juce::PopupMenu menu;menu.addItem(1,"MIDI Learn");menu.addItem(2,tr("Effacer l'affectation","Clear mapping"),p.midiCC[idx].load()>=0);menu.addItem(3,tr("Annuler l'apprentissage","Cancel learn"),p.learnTarget.load()>=0);auto safe=juce::Component::SafePointer<Editor>(this);menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),[safe,idx](int choice){if(safe==nullptr)return;if(choice==1)safe->p.learnTarget.store(idx);if(choice==2)safe->p.midiCC[idx].store(-1);if(choice==3)safe->p.learnTarget.store(-1);safe->repaint();});return;}return;}if(!e.mods.isLeftButtonDown())return;for(auto it=hits.rbegin();it!=hits.rend();++it)if(it->bounds.contains(pt)){if(it->param){dragging=it->param;startValue=dragging->getValue();startY=pt.y;dragging->beginChangeGesture();}else if(it->click){auto f=it->click;f();syncSearch();}repaint();return;}}
 void mouseDrag(const juce::MouseEvent& e)override{if(dragging){dragging->setValueNotifyingHost(juce::jlimit(0.f,1.f,startValue+(startY-logical(e.position).y)/160.f));repaint();}}
 void mouseUp(const juce::MouseEvent&)override{if(dragging){dragging->endChangeGesture();dragging=nullptr;}}
 void mouseDoubleClick(const juce::MouseEvent& e)override{auto pt=logical(e.position);for(auto it=hits.rbegin();it!=hits.rend();++it)if(it->bounds.contains(pt)){if(it->param)set(it->param,it->param->getDefaultValue());return;}}
};
