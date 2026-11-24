#pragma once
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
 enum class Screen{Rack,Detail,Library,History,Routing};
 Screen screen=Screen::Rack;
 int selected=0;bool french=true,viewB=false;
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
 int model(int slot){return juce::jlimit(0,7,int(raw(slot,"model")));}
 bool on(int slot){return raw(slot,"on")>.5f;}
 void set(juce::RangedAudioParameter* a,float v){if(!a)return;a->beginChangeGesture();a->setValueNotifyingHost(juce::jlimit(0.f,1.f,v));a->endChangeGesture();}
 void setRaw(int slot,const char* key,float v){auto* a=param(slot,key);set(a,a->convertTo0to1(v));}
 void swapSlots(int a,int b){if(b<0||b>=8)return;for(auto key:{"on","model","threshold","ratio","attack","release","makeup","mix","color"}){float x=raw(a,key),y=raw(b,key);setRaw(a,key,y);setRaw(b,key,x);}selected=b;}
 void toggleAB(){if(viewB){snapshotB=p.state.copyState();p.state.replaceState(snapshotA.createCopy());}else{snapshotA=p.state.copyState();p.state.replaceState(snapshotB.createCopy());}viewB=!viewB;}
 static juce::Colour color(int m){constexpr unsigned c[]={0xff8d2638,0xff655e50,0xff58616b,0xff536677,0xff643654,0xff286054,0xff756956,0xff41454d};return juce::Colour(c[juce::jlimit(0,7,m)]);}
 static const char* family(int m){constexpr const char* f[]={"FET","OPTO","VCA","VCA","VARI-MU","DIODE","OPTO","DIGITAL"};return f[m];}
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
  drawLabel(g,"TITANIUM RED  /  v0.1.2",R{108,86,500,20},15,muted);
  button(g,R{705,32,155,48},"RACK",[this]{screen=Screen::Rack;},false,screen==Screen::Rack);
  button(g,R{873,32,190,48},tr("BIBLIOTHÈQUE","LIBRARY"),[this]{screen=Screen::Library;},false,screen==Screen::Library);
  button(g,R{1076,32,170,48},tr("ROUTAGE","ROUTING"),[this]{screen=Screen::Routing;},false,screen==Screen::Routing);
  button(g,R{1259,32,130,48},viewB?"A / [B]":"[A] / B",[this]{toggleAB();},false,viewB);
  button(g,R{1402,32,157,48},french?u("Français"):"English",[this]{french=!french;},false,false);
  line(g,20,118,1580,118,goldDark,1.5f);
 }
 void board(juce::Graphics& g){
  using namespace clskin;surface(g,R{28,139,1544,641},navySoft,midnight,18.f,goldDark);
  drawLabel(g,tr("COLLECTION LIBERTY","LIBERTY COLLECTION"),R{65,146,310,27},19,cream,false,juce::Justification::centredLeft,true);
  drawLabel(g,tr("8 PROFILS EXPÉRIMENTAUX","8 EXPERIMENTAL PROFILES"),R{390,146,440,27},17,muted);
  bool par=p.state.getRawParameterValue("parallel")->load()>.5f;
  drawLabel(g,par?"IN > SPLIT A/B > MIX > OUT":"IN > 1 > 2 > 3 > 4 > 5 > 6 > 7 > 8 > OUT",R{860,146,675,27},16,cream,false,juce::Justification::centredRight,true);
  std::array<R,8> boxes;for(int i=0;i<8;++i)boxes[i]=R{56.f+(i%4)*381.f,177.f+(i/4)*294.f,355.f,281.f};
  for(int i=0;i<7;++i)if(i!=3){auto a=boxes[i],b=boxes[i+1];line(g,a.getRight()-12,a.getCentreY(),b.getX()+12,b.getCentreY(),gold,5.f);}
  if(!par){juce::Path cable;cable.startNewSubPath(1560,340);cable.lineTo(1560,466);cable.lineTo(42,466);cable.lineTo(42,610);cable.lineTo(72,610);g.setColour(gold);g.strokePath(cable,juce::PathStrokeType(5.f));}
  for(int i=0;i<8;++i){
   auto b=boxes[i];surface(g,b,navy,midnight,10.f,selected==i?gold:goldDark);
   drawLabel(g,juce::String(i+1).paddedLeft('0',2),R{b.getX()+13,b.getY()+7,45,27},23,cream,true);
   drawLabel(g,family(model(i)),R{b.getX()+61,b.getY()+9,100,23},14,muted,false,juce::Justification::centredLeft,true);
   button(g,R{b.getRight()-180,b.getY()+8,102,25},tr("HISTOIRE","HISTORY"),[this,i]{selected=i;screen=Screen::History;},false,false,13);
   button(g,R{b.getRight()-72,b.getY()+8,62,25},on(i)?"ON":"OFF",[this,i]{setRaw(i,"on",on(i)?0.f:1.f);},false,on(i),13);
   R unit{b.getX()+37,b.getY()+39,b.getWidth()-74,202};enclosure(g,unit,i,model(i),true);hit(unit,[this,i]{selected=i;screen=Screen::Detail;});
   button(g,R{b.getX()+15,b.getBottom()-34,123,26},tr("ÉDITER","EDIT"),[this,i]{selected=i;screen=Screen::Detail;},false,false,13);
   button(g,R{b.getX()+144,b.getBottom()-34,118,26},tr("CHOISIR","CHOOSE"),[this,i]{selected=i;screen=Screen::Library;},false,false,13);
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
  button(g,R{332,520,190,45},tr("REMPLACER","REPLACE"),[this]{screen=Screen::Library;});
  button(g,R{130,579,392,45},tr("HISTOIRE ET CARACTÈRE","HISTORY AND CHARACTER"),[this]{screen=Screen::History;});
  const char* keys[]={"threshold","ratio","attack","release","makeup","mix","color"};
  juce::String names[]={tr("Seuil","Threshold"),"Ratio",tr("Attaque","Attack"),tr("Relâchement","Release"),"Gain",tr("Mélange","Mix"),tr("Couleur","Color")};
  const char* units[]={" dB",":1"," ms"," ms"," dB","",""};
  for(int i=0;i<7;++i)knob(g,selected,keys[i],680.f+(i%4)*210.f,340.f+(i/4)*235.f,42,names[i],units[i]);
  drawLabel(g,tr("Les commandes agissent sur le moteur DSP du slot.","Controls adjust the selected slot DSP."),R{120,805,1360,45},19,muted,false,juce::Justification::centred);
 }
 void library(juce::Graphics& g){
  using namespace clskin;panel(g,tr("BIBLIOTHÈQUE DE COMPRESSEURS","COMPRESSOR LIBRARY")+"  /  "+tr("SLOT","SLOT")+" "+juce::String(selected+1));
  for(int i=0;i<8;++i){R b{115.f+(i%4)*350,245.f+(i/4)*302,330,280};surface(g,b,navy,midnight,9.f,goldDark);enclosure(g,R{b.getX()+16,b.getY()+12,298,206},-1,i,true);button(g,R{b.getX()+30,b.getBottom()-50,270,37},tr("CHARGER","LOAD"),[this,i]{setRaw(selected,"model",float(i));setRaw(selected,"on",1);screen=Screen::Detail;});}
 }
 void history(juce::Graphics& g){
  using namespace clskin;panel(g,tr("HISTOIRE ET CARACTÈRE","HISTORY AND CHARACTER"));
  int m=model(selected);enclosure(g,R{140,260,480,250},selected,m,false);
  const char* fr[]={"Famille FET : attaques rapides et dynamique énergique.","Famille optique : réponse progressive et relâchement dépendant du signal.","Famille VCA : contrôle dynamique précis, adapté aux bus.","Famille VCA : réponse plus vive pour les transitoires.","Famille Vari-Mu : compression douce et caractère harmonique.","Famille pont de diodes : coloration et densité.","Famille optique : enveloppe douce et coloration réduite.","Famille numérique : courbe précise et sans coloration ajoutée."};
  const char* en[]={"FET family: fast attack and energetic dynamics.","Optical family: gradual response and program-dependent release.","VCA family: accurate dynamics for buses.","VCA family: faster transient response.","Vari-Mu family: soft compression and harmonic character.","Diode bridge family: coloration and density.","Optical family: gentle envelope and reduced coloration.","Digital family: precise curve with no added coloration."};
  drawLabel(g,liberty::models[m].name,R{670,260,780,70},40,cream,true);drawLabel(g,tr(fr[m],en[m]),R{670,350,780,90},22,cream);
  drawLabel(g,tr("Profil expérimental original Liberty, pas une émulation mesurée.","Original experimental Liberty profile, not a measured emulation."),R{670,455,780,65},18,muted);
  button(g,R{145,550,470,48},tr("ÉDITER LE COMPRESSEUR","EDIT COMPRESSOR"),[this]{screen=Screen::Detail;});
 }
 void routing(juce::Graphics& g){
  using namespace clskin;panel(g,tr("ROUTAGE DU SIGNAL","SIGNAL ROUTING"));
  bool par=p.state.getRawParameterValue("parallel")->load()>.5f;
  button(g,R{155,248,260,50},tr("SÉRIE","SERIES"),[this]{set(p.state.getParameter("parallel"),0);},false,!par);
  button(g,R{435,248,290,50},tr("PARALLÈLE A/B","PARALLEL A/B"),[this]{set(p.state.getParameter("parallel"),1);},false,par);
  for(int i=0;i<8;++i){R r{160.f+(par?i%4:i)*155.f,par?360.f+(i/4)*190.f:430.f,135,120};surface(g,r,color(model(i)),midnight,8.f,goldDark);drawLabel(g,juce::String(i+1)+" / "+family(model(i)),r.reduced(9),20,cream,false,juce::Justification::centred,true);if(i!=7&&(!par||i!=3))line(g,r.getRight(),r.getCentreY(),r.getRight()+20,r.getCentreY(),gold,4);}
  auto* a=p.state.getParameter("balance");knobDraw(g,820,739,36,a->getValue(),"BALANCE A/B",juce::String(int(a->getValue()*100))+" % B",true,a);
 }
 void timerCallback()override{repaint();}
 juce::Point<float> logical(juce::Point<float> pt)const{return {(pt.x-xOffset)/scale,(pt.y-yOffset)/scale};}
public:
 explicit Editor(Processor& processor):AudioProcessorEditor(&processor),p(processor){
  setOpaque(true);setResizable(false,false);snapshotA=p.state.copyState();snapshotB=snapshotA.createCopy();setSize(1120,651);startTimerHz(20);
 }
 ~Editor()override{stopTimer();if(dragging)dragging->endChangeGesture();}
 void resized()override{scale=std::min(getWidth()/1600.f,getHeight()/930.f);xOffset=(getWidth()-1600*scale)*.5f;yOffset=(getHeight()-930*scale)*.5f;}
 void paint(juce::Graphics& g)override{g.fillAll(clskin::midnight);hits.clear();juce::Graphics::ScopedSaveState save(g);g.addTransform(juce::AffineTransform::scale(scale).translated(xOffset,yOffset));backplate(g);switch(screen){case Screen::Rack:board(g);break;case Screen::Detail:detail(g);break;case Screen::Library:library(g);break;case Screen::History:history(g);break;case Screen::Routing:routing(g);break;}}
 void mouseDown(const juce::MouseEvent& e)override{if(!e.mods.isLeftButtonDown())return;auto pt=logical(e.position);for(auto it=hits.rbegin();it!=hits.rend();++it)if(it->bounds.contains(pt)){if(it->param){dragging=it->param;startValue=dragging->getValue();startY=pt.y;dragging->beginChangeGesture();}else if(it->click){auto f=it->click;f();}repaint();return;}}
 void mouseDrag(const juce::MouseEvent& e)override{if(dragging){dragging->setValueNotifyingHost(juce::jlimit(0.f,1.f,startValue+(startY-logical(e.position).y)/160.f));repaint();}}
 void mouseUp(const juce::MouseEvent&)override{if(dragging){dragging->endChangeGesture();dragging=nullptr;}}
 void mouseDoubleClick(const juce::MouseEvent& e)override{auto pt=logical(e.position);for(auto it=hits.rbegin();it!=hits.rend();++it)if(it->bounds.contains(pt)){if(it->param)set(it->param,it->param->getDefaultValue());return;}}
};
