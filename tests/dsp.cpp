#include "Dynamics.h"
#include "FactoryPresets.h"
#include <set>
#include <sstream>
#include <iostream>
#include <stdexcept>
using namespace liberty;
void require(bool b,const char* msg){if(!b)throw std::runtime_error(msg);}
int main(){
 Rack rack;rack.prepare(48000);float l=.75f,r=-.3f;rack.process(l,r);require(l==.75f&&r==-.3f,"bypass");
 for(int i=0;i<modelCount;++i){
  rack.reset();rack.params={};rack.params[0].enabled=true;rack.params[0].model=i;
  for(int n=0;n<96000;++n){l=.8f;r=-.4f;rack.process(l,r);require(std::isfinite(l)&&std::isfinite(r),"finite");}
  if(models[i].architecture!=3&&models[i].architecture!=4&&models[i].architecture!=9)require(std::abs(l)<.8f&&rack.gr(0)>0,"compression");require(std::abs(l+2*r)<1e-4f,"stereo link");
  std::cout<<models[i].name<<": GR "<<rack.gr(0)<<" dB\n";
  for(int n=0;n<480000;++n){l=r=0;rack.process(l,r);}require(rack.gr(0)<.05f,"release");
 }
 rack.reset();rack.params={};rack.parallel=true;rack.params[0].enabled=true;rack.balance=1;l=.7f;r=-.2f;rack.process(l,r);require(l==.7f&&r==-.2f,"parallel B");
 rack.balance=0;for(int n=0;n<96000;++n){l=.7f;r=-.2f;rack.process(l,r);}require(l<.7f,"parallel A");
 rack.params[0].mix=0;l=.7f;r=-.2f;rack.process(l,r);require(l==.7f&&r==-.2f,"dry mix");
 require(reduction(-10,4,0)==0&&std::abs(reduction(12,4,0)-9)<1e-6f,"static curve");
 rack.reset();rack.params={};rack.parallel=false;rack.params[0].enabled=true;rack.params[0].model=7;rack.external[0]=true;
 for(int n=0;n<96000;++n){l=r=.1f;rack.process(l,r,.9f,.9f,true);}require(rack.gr(0)>5.f,"external sidechain drives compression");
 for(int n=0;n<480000;++n){l=r=.1f;rack.process(l,r,0,0,true);}require(rack.gr(0)<.05f,"external silence releases");
 rack.external[0]=false;
 for(int model:{25,32,33,34,35,36,37}){rack.reset();rack.params={};rack.params[0].enabled=true;rack.params[0].model=model;
 for(int n=0;n<96000;++n){l=r=.8f*std::sin(n*.8f);rack.process(l,r);require(std::isfinite(l)&&std::isfinite(r),"new architecture finite");}require(rack.gr(0)>1.f,"new architecture dynamic reduction");}
 require(std::abs(nominalReduction(-6,models[7],Parameters{})-9)<.001f,"nominal curve");
 // Latency-aligned dry, spectral unity and independent Mid/Side.
 rack.params={};rack.parallel=false;rack.configureLatency(24,true);
 int latency=rack.latency();for(int i=0;i<=latency;++i){l=r=i==0?1.f:0.f;rack.process(l,r);require(std::abs(l-(i==latency?1.f:0.f))<1e-6f,"serial latency");}
 rack.reset();rack.parallel=true;rack.balance=.5f;for(int i=0;i<=latency;++i){l=r=i==0?1.f:0.f;rack.process(l,r);require(std::abs(l-(i==latency?1.f:0.f))<1e-6f,"parallel latency");}
 rack.configureLatency(0,false);rack.parallel=false;rack.reset();rack.params={};rack.params[0].enabled=true;rack.params[0].model=56;rack.params[0].ratio=1;for(int i=0;i<1200;++i){l=r=i==300?.5f:0.f;rack.process(l,r);require(std::abs(l-(i==300+Spectral::N?.5f:0.f))<2e-4f,"spectral unity and latency");}
 rack.reset();rack.params={};rack.params[0].enabled=true;rack.params[0].model=7;rack.params[0].delta=true;rack.params[0].ratio=1;for(int i=0;i<4096;++i){l=.5f;r=-.2f;rack.process(l,r);require(std::abs(l)+std::abs(r)<1e-6f,"delta unity null");}
 rack.reset();rack.params[0].delta=false;rack.params[0].stereoMode=1;rack.params[0].ratio=4;for(int i=0;i<96000;++i){l=.8f;r=.2f;rack.process(l,r);require(std::abs((l-r)-.6f)<1e-5f,"mid preserves side");}
 // Meter taps must originate from real band envelopes and FFT bins.
 rack.configureLatency(0,false);rack.parallel=false;rack.reset();rack.params={};rack.params[0].enabled=true;rack.params[0].model=54;
 for(int n=0;n<48000;++n){l=r=.85f*std::sin(6.283185307f*80*n/48000);rack.process(l,r);}
 require(rack.displayReduction(0,0)>0&&rack.displayReduction(0,0)>rack.displayReduction(0,3),"real band taps");
 rack.reset();rack.params[0].model=56;for(int n=0;n<48000;++n){l=r=.8f*std::sin(6.283185307f*1500*n/48000);rack.process(l,r);}
 require(rack.displayReduction(0,1)>0&&rack.displayReduction(0,1)>rack.displayReduction(0,15),"real FFT taps");
 rack.params[0].enabled=false;l=r=0;rack.process(l,r);require(rack.displayReduction(0,1)==0,"disabled meter");
 // New engine: exact disabled mode, harmonic signatures and per-model dynamics.
 std::set<std::string> characterKeys;
 for(int m=0;m<modelCount;++m){
  Parameters p;p.model=m;p.enabled=true;p.analog=false;p.color=.7f;
  Slot legacy,reference;legacy.prepare(48000);reference.prepare(48000);
  for(int n=0;n<8192;++n){float a=.65f*std::sin(n*.13f),b=.4f*std::sin(n*.19f),c=a,d=b;legacy.process(a,b,p);p.character=0;reference.process(c,d,p);p.character=1;require(a==c&&b==d,"legacy ignores character");}
  p.analog=true;p.character=.65f;p.color=.7f;auto md=characterModel(p);auto cp=characterProfile(m);std::ostringstream fingerprint;fingerprint<<md.attackScale<<':'<<md.releaseScale<<':'<<md.knee<<':'<<cp.drive<<':'<<cp.even<<':'<<int(cp.circuit);require(characterKeys.insert(fingerprint.str()).second,"unique character profile");
  Slot fresh;fresh.prepare(48000);for(int n=0;n<16384;++n){float a=.7f*std::sin(n*.13f),b=.4f*std::sin(n*.19f);fresh.process(a,b,p);require(std::isfinite(a)&&std::isfinite(b),"analog finite");}
 }
 // Measure Fourier amplitudes of the real processed tone (1 kHz / 48 kHz).
 auto harmonic=[&](int model){Slot slot;slot.prepare(48000);Parameters p;p.enabled=true;p.model=model;p.analog=true;p.color=1;p.character=1;p.ratio=1;p.threshold=0;
  double re[4]{},im[4]{};for(int n=0;n<96000;++n){float x=.6f*std::sin(6.283185307179586*n/48.),y=x;slot.process(x,y,p);if(n>=48000)for(int h=1;h<=3;++h){double a=6.283185307179586*n*h/48.;re[h]+=x*std::cos(a);im[h]+=x*std::sin(a);}}
  double f=std::hypot(re[1],im[1]);require(f>1,"nonzero fundamental");return std::array<double,2>{std::hypot(re[2],im[2])/f,std::hypot(re[3],im[3])/f};
 };
 auto fet=harmonic(0),tube=harmonic(4),digital=harmonic(7),diode=harmonic(5);
 require(fet[1]>.001&&tube[0]>.001&&diode[0]>.001,"harmonic generation");
 require(tube[0]>fet[0],"tube even signature");require(digital[0]<1e-6&&digital[1]<1e-6,"digital clean");
 std::cout<<"Measured relative H2/H3: FET "<<fet[0]<<'/'<<fet[1]<<" Vari-Mu "<<tube[0]<<'/'<<tube[1]<<" Diode "<<diode[0]<<'/'<<diode[1]<<" Digital "<<digital[0]<<'/'<<digital[1]<<'\\n';
 Parameters ap;ap.model=0;ap.analog=true;ap.ratio=4;ap.allButtons=false;float normal=nominalReduction(-3,characterModel(ap),ap);ap.allButtons=true;require(nominalReduction(-3,characterModel(ap),ap)>normal,"all buttons transfer");
 ap.model=4;ap.allButtons=false;require(characterRatio(24,ap)>characterRatio(0,ap),"variable mu ratio");
 require(characterKeys.size()==60,"sixty characters");
 // PRO FFT exact reconstruction at unity across all resolutions.
 for(int fft:{512,1024,2048,4096}){ProSpectral sp;sp.prepare();ProSettings ps;ps.fft=fft;for(auto& b:ps.bands)b.ratio=1;
  for(int n=0;n<fft*3;++n){float a=n==fft?.5f:0,b=a;sp.process(a,b,a,b,ps,48000,0,0,1,1,false);require(std::abs(a-(n==2*fft?.5f:0))<3e-4f,"PRO FFT unity/latency");}}
 ProSettings ps;ps.count=6;for(auto& b:ps.bands)b.ratio=1;ProMulti pm;pm.reset();
 for(int n=0;n<10000;++n){float a=.7f*std::sin(n*.17f),b=.3f*std::cos(n*.21f),x=a,y=b;pm.process(a,b,a,b,ps,48000,0,0,1,1);require(std::abs(a-x)+std::abs(b-y)<2e-6f,"six-band complementary unity");}
 for(auto& b:ps.bands)b.mute=true;float a=.5f,b=.2f;pm.process(a,b,a,b,ps,48000,0,0,1,1);require(a==0&&b==0,"all muted");
 for(auto& b:ps.bands){b.mute=false;b.ratio=4;}ps.link=0;pm.reset();for(int n=0;n<48000;++n){a=.8f;b=.01f;pm.process(a,b,a,b,ps,48000,0,0,1,1);}require(std::abs(b-.01f)<1e-5f&&a<.8f,"unlinked right unchanged");
 ps.link=1;pm.reset();for(int n=0;n<48000;++n){a=.8f;b=.01f;pm.process(a,b,a,b,ps,48000,0,0,1,1);}require(b<.009f,"linked right follows");
 rack.params={};rack.parallel=true;rack.configureLatency(0,true,4096);int proLatency=rack.latency();for(int n=0;n<=proLatency;++n){l=r=n==0?1.f:0.f;rack.process(l,r);require(std::abs(l-(n==proLatency?1.f:0.f))<1e-6f,"PRO rack delay");}rack.configureLatency(0,false);rack.reset();rack.parallel=false;
 std::set<std::string> fingerprints;int series=0,parallelCount=0;
 for(int i=0;i<factoryCount;++i){auto p=factoryPreset(i);p.parallel?++parallelCount:++series;std::ostringstream key;key<<p.parallel<<':'<<p.balance;
  require(p.slots[0].enabled,"preset A active");require(!p.parallel||p.slots[4].enabled,"preset B active");
  rack.reset();rack.params=p.slots;rack.parallel=p.parallel;rack.balance=p.balance;
  for(const auto& s:p.slots){require(s.model>=0&&s.model<modelCount,"valid model");require(s.threshold>=-60&&s.threshold<=0&&s.ratio>=1&&s.ratio<=20&&s.attack>=.1f&&s.attack<=100&&s.release>=5&&s.release<=2000&&s.mix>=0&&s.mix<=1&&s.color>=0&&s.color<=1,"preset ranges");key<<':'<<s.stereoMode<<':'<<s.autoGain<<':'<<s.delta<<':'<<s.enabled<<':'<<s.model<<':'<<s.threshold<<':'<<s.ratio<<':'<<s.attack<<':'<<s.release<<':'<<s.mix<<':'<<s.color;}
  require(fingerprints.insert(key.str()).second,"unique preset state");
  for(int n=0;n<4096;++n){l=.7f*std::sin(float(n)*.07f);r=.5f*std::sin(float(n)*.11f);rack.process(l,r);require(std::isfinite(l)&&std::isfinite(r),"preset finite");}
 }
 require(series==200&&parallelCount==200,"200 series / 200 parallel");
 std::cout<<"60 profiles / 400 unique presets passed\\n";
 std::cout<<"DSP tests passed\n";
}
