#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <complex>
#include <vector>
#include "Character.h"
#include "ProDSP.h"
#include "Routing.h"
namespace liberty {
inline float db(float x){return 20.f*std::log10(std::max(x,1.e-9f));}
inline float gain(float x){return std::pow(10.f,x*.05f);}
struct Model{const char* name;float attackScale,releaseScale,knee,memory,drive,rms;bool feedback;int architecture=0;};
inline constexpr int modelCount=60;
inline constexpr std::array<Model,modelCount> models {{
 {"FET Impact", .25f, .65f, 2.f, .04f, .23f, .05f, true},
 {"Opto Velvet", 2.5f, 1.8f, 9.f, .72f, .12f, .60f, true},
 {"VCA Glue", 1.f, 1.f, 4.f, .05f, .04f, .45f, false},
 {"VCA Punch", .55f, .7f, 1.f, .02f, .05f, .12f, false},
 {"Vari-Mu Warm", 2.f, 2.2f, 14.f, .45f, .19f, .35f, true},
 {"Diode Bronze", 1.3f, 1.35f, 5.f, .2f, .31f, .2f, true},
 {"Opto Clear", 1.7f, 1.4f, 7.f, .52f, .025f, .7f, false},
 {"Digital Precision", 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, false},
 {"FET Lightning",0.1f,0.42f,1.0f,0.01f,0.18f,0.02f,true},
 {"FET Classic",0.45f,1.1f,3.0f,0.12f,0.28f,0.16f,true},
 {"Opto Silk",3.2f,2.4f,12.0f,0.83f,0.065f,0.85f,true},
 {"Opto Studio",1.2f,1.15f,6.0f,0.35f,0.09f,0.62f,false},
 {"VCA Master",1.4f,1.5f,8.0f,0.08f,0.012f,0.75f,false},
 {"VCA Snap",0.38f,0.52f,0.5f,0.01f,0.045f,0.03f,false},
 {"Vari-Mu Silk",2.8f,2.9f,18.0f,0.65f,0.11f,0.55f,true},
 {"Vari-Mu Density",1.5f,1.9f,11.0f,0.38f,0.35f,0.25f,true},
 {"Diode Iron",0.85f,1.0f,3.0f,0.12f,0.4f,0.13f,true},
 {"Diode Smooth",1.8f,1.9f,9.0f,0.42f,0.21f,0.5f,true},
 {"Digital Peak",0.28f,0.75f,0.0f,0.0f,0.0f,0.0f,false},
 {"Digital RMS",1.1f,1.25f,6.0f,0.15f,0.0f,0.95f,false},
 {"FET Edge",0.16f,0.54f,1.5f,0.03f,0.32f,0.08f,true,0},
 {"FET Round",0.65f,1.45f,5.0f,0.22f,0.22f,0.24f,true,0},
 {"Opto Dream",3.5f,3.2f,15.0f,0.88f,0.07f,0.9f,true,1},
 {"Opto Fast",0.75f,0.95f,5.0f,0.28f,0.09f,0.5f,false,0},
 {"VCA Adaptive",0.85f,1.0f,4.0f,0.2f,0.03f,0.45f,false,1},
 {"VCA Low Guard",1.1f,1.2f,5.0f,0.07f,0.025f,0.5f,false,3},
 {"Vari-Mu Air",2.1f,2.4f,16.0f,0.5f,0.09f,0.6f,true,0},
 {"Vari-Mu Bloom",1.8f,2.0f,12.0f,0.4f,0.28f,0.4f,true,1},
 {"Diode Punch",0.6f,0.9f,2.0f,0.08f,0.38f,0.1f,true,0},
 {"Diode Master",1.7f,1.8f,10.0f,0.35f,0.16f,0.65f,true,0},
 {"Digital Adaptive",1.0f,1.0f,3.0f,0.12f,0.0f,0.35f,false,1},
 {"Digital Crest",0.5f,0.8f,2.0f,0.05f,0.0f,0.25f,false,2},
 {"Digital Low Cut",1.0f,1.0f,4.0f,0.1f,0.0f,0.55f,false,3},
 {"Digital High Focus",0.55f,0.8f,2.0f,0.04f,0.0f,0.2f,false,4},
 {"Dual Band Balance",1.0f,1.0f,5.0f,0.12f,0.0f,0.45f,false,5},
 {"Dual Band Bass",1.4f,1.5f,7.0f,0.18f,0.0f,0.65f,false,5},
 {"Dual Band Punch",0.5f,0.7f,2.0f,0.04f,0.0f,0.15f,false,5},
 {"Dual Band Master",1.6f,1.8f,9.0f,0.22f,0.0f,0.7f,false,5},
 {"Adaptive Parallel",0.7f,0.8f,3.0f,0.12f,0.06f,0.3f,false,1},
 {"Transient Sculpt",0.25f,0.6f,1.0f,0.02f,0.0f,0.1f,false,2},
 {"FET Vintage",0.55f,1.3f,4.0f,0.15f,0.34f,0.18f,true,0},
 {"FET Studio",0.32f,0.9f,3.0f,0.08f,0.12f,0.25f,true,0},
 {"Opto Master",2.0f,2.1f,12.0f,0.6f,0.04f,0.8f,false,0},
 {"Opto Tube",2.7f,2.4f,10.0f,0.72f,0.3f,0.6f,true,1},
 {"VCA Tight",0.45f,0.65f,2.0f,0.04f,0.025f,0.2f,false,0},
 {"VCA Console",1.0f,1.2f,6.0f,0.12f,0.08f,0.6f,false,1},
 {"Vari-Mu Royal",3.0f,2.6f,20.0f,0.7f,0.23f,0.65f,true,0},
 {"Vari-Mu Focus",1.25f,1.7f,10.0f,0.25f,0.17f,0.3f,true,0},
 {"Diode Vintage",1.0f,1.5f,6.0f,0.2f,0.44f,0.25f,true,0},
 {"Diode Velvet",2.2f,2.2f,13.0f,0.45f,0.15f,0.7f,true,1},
 {"PWM Pulse",0.35f,0.8f,2.0f,0.04f,0.04f,0.1f,false,6},
 {"PWM Studio",0.8f,1.1f,5.0f,0.12f,0.03f,0.4f,false,6},
 {"Tri Band Control",1.0f,1.0f,5.0f,0.12f,0.0f,0.45f,false,7},
 {"Tri Band Master",1.5f,1.8f,8.0f,0.2f,0.0f,0.7f,false,7},
 {"Quad Band Focus",0.8f,1.0f,4.0f,0.1f,0.0f,0.35f,false,8},
 {"Quad Band Smooth",1.7f,2.0f,9.0f,0.3f,0.0f,0.75f,false,8},
 {"Spectral Balance",1.0f,1.0f,6.0f,0.12f,0.0f,0.45f,false,9},
 {"Spectral Detail",0.6f,0.8f,3.0f,0.04f,0.0f,0.25f,false,9},
 {"Spectral Master",1.5f,1.8f,10.0f,0.2f,0.0f,0.65f,false,9},
 {"Spectral Focus",0.4f,0.65f,2.0f,0.02f,0.0f,0.15f,false,9}
}};
struct Parameters{int model=0;bool enabled=false;float threshold=-18.f,ratio=4.f,attack=10.f,release=100.f,makeup=0.f,mix=1.f,color=0.f;int stereoMode=0;bool autoGain=false,delta=false;bool analog=false,allButtons=false;float character=.65f;ProSettings pro{};};
inline Model characterModel(const Parameters& p){
 Model m=models[size_t(std::clamp(p.model,0,modelCount-1))];if(!p.analog)return m;
 const auto& c=characterProfile(p.model);float a=std::clamp(p.character,0.f,1.f);
 m.attackScale*=blend(1,c.attack,a);m.releaseScale*=blend(1,c.release,a);m.knee*=blend(1,c.knee,a);
 if(c.circuit==Circuit::OPTO){m.memory=blend(m.memory,std::max(m.memory,.68f),a*.55f);m.rms=blend(m.rms,.78f,a*.25f);}
 else if(c.circuit==Circuit::TUBE){m.knee+=a*3.f;m.memory=blend(m.memory,.65f,a*.3f);}
 else if(c.circuit==Circuit::DIODE){m.memory=blend(m.memory,.34f,a*.25f);}
 if(c.circuit==Circuit::FET&&p.allButtons){m.knee=std::max(4.f,m.knee*1.7f);m.attackScale*=.55f;m.releaseScale*=.62f;}
 return m;
}
inline float characterRatio(float over,const Parameters& p){
 float ratio=std::clamp(p.ratio,1.f,20.f);
 if(!p.analog)return ratio;const auto c=characterProfile(p.model).circuit;float a=std::clamp(p.character,0.f,1.f);
 if(c==Circuit::FET&&p.allButtons)return std::max(ratio,12.f);
 if(c==Circuit::TUBE)return std::min(20.f,1.f+(ratio-1.f)*(1.f+a*.6f*std::clamp(over/24.f,0.f,1.f)));
 if(c==Circuit::PWM)return std::min(20.f,1.f+(ratio-1.f)*(1.f+a*.12f*std::clamp(over/18.f,0.f,1.f)));
 return ratio;
}
inline float reduction(float over,float ratio,float knee){
 const float slope=1.f-1.f/std::max(1.f,ratio);
 if(knee<=0.f)return std::max(0.f,over)*slope;
 if(over<=-knee*.5f)return 0.f;
 if(over>=knee*.5f)return over*slope;
 const float x=over+knee*.5f;return slope*x*x/(2.f*knee);
}
// Nominal static detector-to-output curve; adaptive/weighted/band modes are signal-dependent.
inline float nominalReduction(float inputDb,const Model& m,const Parameters& p,bool external=false){
 const auto target=[&](float over){float ratio=characterRatio(over,p);float effective=m.feedback?1.f+std::min(100.f,ratio-1.f)*2.f:ratio;return reduction(over,effective,m.knee);};
 if(!m.feedback||external)return target(inputDb-p.threshold);
 float lo=0,hi=std::max(0.f,inputDb-p.threshold+m.knee);
 for(int i=0;i<20;++i){float mid=(lo+hi)*.5f;if(target(inputDb-mid-p.threshold)>mid)lo=mid;else hi=mid;}
 return (lo+hi)*.5f;
}

// Streaming 256-point STFT, 50% overlap, sine window, linked bin envelopes.
// Fixed N sample delay; zero allocation during processing.
class Spectral {
 using C=std::complex<float>;
public:
 static constexpr int N=256,H=N/2;
private:
 std::array<float,N> inL{},inR{},detL{},detR{},outL{},outR{},window{};
 std::array<float,N/2+1> env{};
 std::array<C,N> a{},b{},c{},d{};
 int pos=0,hop=0;float gr=0,det=0;
 static void fft(std::array<C,N>& x,bool inverse){
  for(int i=1,j=0;i<N;++i){int bit=N>>1;for(;j&bit;bit>>=1)j^=bit;j^=bit;if(i<j)std::swap(x[i],x[j]);}
  for(int len=2;len<=N;len<<=1){float angle=(inverse?1.f:-1.f)*6.283185307f/len;C step(std::cos(angle),std::sin(angle));
   for(int i=0;i<N;i+=len){C w(1,0);for(int j=0;j<len/2;++j){C u=x[i+j],v=x[i+j+len/2]*w;x[i+j]=u+v;x[i+j+len/2]=u-v;w*=step;}}}
  if(inverse)for(auto& q:x)q/=float(N);
 }
public:
 void prepare(){for(int i=0;i<N;++i)window[i]=std::sin(3.141592654f*(i+.5f)/N);reset();}
 void reset(){inL.fill(0);inR.fill(0);detL.fill(0);detR.fill(0);outL.fill(0);outR.fill(0);env.fill(0);pos=hop=0;gr=det=0;}
 float reductionDb()const{return gr;}float detector()const{return det;}
 float groupReduction(int group)const{if(group<0||group>=16)return 0;float result=0;for(int k=group*8;k<std::min(N/2+1,(group+1)*8+(group==15?1:0));++k)result=std::max(result,env[size_t(k)]);return result;}
 void process(float& l,float& r,float dl,float dr,float sr,const Model& m,const Parameters& p,bool separate){
  const float ol=outL[pos],orr=outR[pos];outL[pos]=outR[pos]=0;
  inL[pos]=l;inR[pos]=r;detL[pos]=dl;detR[pos]=dr;pos=(pos+1)%N;
  if(++hop==H){hop=0;
   for(int i=0;i<N;++i){int j=(pos+i)%N;float w=window[i];a[i]=inL[j]*w;b[i]=inR[j]*w;if(separate){c[i]=detL[j]*w;d[i]=detR[j]*w;}}
   fft(a,false);fft(b,false);if(separate){fft(c,false);fft(d,false);}
   const float at=std::exp(-1000.f*H/(std::max(.05f,p.attack*m.attackScale)*sr)),rel=std::exp(-1000.f*H/(std::max(.05f,p.release*m.releaseScale)*sr));gr=det=0;
   for(int k=0;k<=N/2;++k){float mag=std::max(std::abs(separate?c[k]:a[k]),std::abs(separate?d[k]:b[k]))*(k==0||k==N/2?1.f:2.f)/(N*.63661977f);
    det=std::max(det,mag);float target=reduction(db(mag)-p.threshold,p.ratio,m.knee);float coeff=target>env[k]?at:rel;env[k]=coeff*env[k]+(1-coeff)*target;gr=std::max(gr,env[k]);
    float g=gain(p.makeup-env[k]);a[k]*=g;b[k]*=g;if(k>0&&k<N/2){a[N-k]*=g;b[N-k]*=g;}}
   fft(a,true);fft(b,true);for(int i=0;i<N;++i){int j=(pos+i)%N;outL[j]+=a[i].real()*window[i];outR[j]+=b[i].real()*window[i];}
  }
  l=ol;r=orr;
 }
};
class SampleDelay {
 std::vector<float> l,r;size_t pos=0;
public:
 void prepare(int max){l.assign(size_t(max+1),0);r.assign(size_t(max+1),0);pos=0;}
 void reset(){std::fill(l.begin(),l.end(),0);std::fill(r.begin(),r.end(),0);pos=0;}
 void process(float& a,float& b,int delay){if(l.empty())return;delay=std::clamp(delay,0,int(l.size()-1));l[pos]=a;r[pos]=b;size_t rd=(pos+l.size()-size_t(delay))%l.size();a=l[rd];b=r[rd];pos=(pos+1)%l.size();}
};

class Slot{
 float sr=48000.f,env=0.f,slow=0.f,power=0.f,previous=0.f,applied=0.f;
 float lowL=0,lowR=0,detLowL=0,detLowR=0,bandEnv[4]{},bandPower[4]{},bandSlow[4]{},lastDetector=0;
 float splitL[3]{},splitR[3]{},splitDL[3]{},splitDR[3]{};
 Spectral spectral; ProSpectral proSpectral;ProMulti proMulti;ProEQ proEQ;SampleDelay signalDelay,dryDelay,proMonitorDelay;bool lastPro=false;int lastProSize=0,lastProCount=0;
 float inPower=0,outPower=0,autoDb=0;
 int lastModel=-1;bool wasEnabled=false,lastAnalog=false;
 CharacterDC dcL,dcR;float dcPole=.999f;
 float cachedAttack=-1,cachedRelease=-1,cachedRate=-1,cachedA=0,cachedR=0,cachedSlow=0,cachedRms=0;
 static float coefficient(float ms,float sr){return std::exp(-1000.f/(std::max(.05f,ms)*sr));}
public:
 void prepare(float rate){sr=std::max(1.f,rate);dcPole=std::exp(-6.283185307f*5.f/sr);signalDelay.prepare(int(sr*.01f)+4096+8);dryDelay.prepare(int(sr*.01f)+4096+8);proMonitorDelay.prepare(4096);spectral.prepare();proSpectral.prepare();reset();}
 void reset(){env=slow=power=previous=applied=0.f;lowL=lowR=detLowL=detLowR=lastDetector=0;for(int i=0;i<4;++i)bandEnv[i]=bandPower[i]=bandSlow[i]=0;for(int i=0;i<3;++i)splitL[i]=splitR[i]=splitDL[i]=splitDR[i]=0;signalDelay.reset();dryDelay.reset();spectral.reset();inPower=outPower=autoDb=0;cachedRate=-1;lastModel=-1;wasEnabled=false;lastAnalog=false;dcL.reset();dcR.reset();proSpectral.reset();proMulti.reset();proEQ.reset();proMonitorDelay.reset();lastPro=false;lastProSize=lastProCount=0;}
 float gr()const{return applied;}
 float detectorLevel()const{return lastDetector;}
 float displayReduction(int model,int group)const{if(!wasEnabled)return 0;const auto& m=models[size_t(std::clamp(model,0,modelCount-1))];if(lastPro)return m.architecture==9?proSpectral.meter(group):proMulti.meter(group);if(m.architecture==9)return spectral.groupReduction(group);int n=m.architecture==5?2:m.architecture==7?3:m.architecture==8?4:0;if(group<0||group>=n)return 0;return m.architecture==5?std::max(bandEnv[group],m.memory*bandSlow[group]):bandEnv[group];}
 float proBandMeter(int group)const{return !wasEnabled||!lastPro?0:(models[size_t(std::clamp(lastModel,0,modelCount-1))].architecture==9?proSpectral.bandMeter(group):proMulti.meter(group));}
 void core(float& l,float& r,const Parameters& p,float scL,float scR,bool external){
  if(!p.enabled){applied=lastDetector=0;return;}
  const auto m=characterModel(p);
  const float dl=external?scL:l,dr=external?scR:r;
  if(p.pro.enabled&&(m.architecture==5||m.architecture>=7)){
   proEQ.prepare(p.pro,sr);float a=proEQ.process(dl,0),b=proEQ.process(dr,1);
   float ml=l,mr=r;
   if(m.architecture==9){proSpectral.process(l,r,a,b,p.pro,sr,m.knee,p.makeup,m.attackScale,m.releaseScale,true);applied=proSpectral.reduction();lastDetector=proSpectral.detector();if(p.pro.listen){ml=a;mr=b;proMonitorDelay.process(ml,mr,proFFT(p.pro.fft));l=ml;r=mr;}}
   else{proMulti.process(l,r,a,b,p.pro,sr,m.knee,p.makeup,m.attackScale,m.releaseScale);applied=proMulti.reduction();lastDetector=proMulti.detector();if(p.pro.listen){l=a;r=b;}}
   return;
  }
  if(m.architecture==9){spectral.process(l,r,dl,dr,sr,m,p,external);applied=spectral.reductionDb();lastDetector=spectral.detector();return;}
  if(m.architecture==7||m.architecture==8){
   int bands=m.architecture==7?3:4;const float hz3[]={180,2500,0},hz4[]={120,600,3500};const float* hz=bands==3?hz3:hz4;
   float al[4]{},ar[4]{},bl[4]{},br[4]{},lastL=0,lastR=0,lastDL=0,lastDR=0;
   for(int i=0;i<bands-1;++i){float c=1-std::exp(-6.283185307f*hz[i]/sr);splitL[i]+=c*(l-splitL[i]);splitR[i]+=c*(r-splitR[i]);splitDL[i]+=c*(dl-splitDL[i]);splitDR[i]+=c*(dr-splitDR[i]);al[i]=splitL[i]-lastL;ar[i]=splitR[i]-lastR;bl[i]=splitDL[i]-lastDL;br[i]=splitDR[i]-lastDR;lastL=splitL[i];lastR=splitR[i];lastDL=splitDL[i];lastDR=splitDR[i];}
   al[bands-1]=l-lastL;ar[bands-1]=r-lastR;bl[bands-1]=dl-lastDL;br[bands-1]=dr-lastDR;
   applied=lastDetector=0;float outL=0,outR=0;
   float rmsC=coefficient(15,sr),at=coefficient(p.attack*m.attackScale,sr),rel=coefficient(p.release*m.releaseScale,sr);
   for(int i=0;i<bands;++i){float peak=std::max(std::abs(bl[i]),std::abs(br[i]));lastDetector=std::max(lastDetector,peak);bandPower[i]=rmsC*bandPower[i]+(1-rmsC)*peak*peak;float level=(1-m.rms)*peak+m.rms*std::sqrt(std::max(0.f,bandPower[i]));float target=reduction(db(level)-p.threshold,p.ratio,m.knee);float c=target>bandEnv[i]?at:rel;bandEnv[i]=c*bandEnv[i]+(1-c)*target;applied=std::max(applied,bandEnv[i]);float g=gain(p.makeup-bandEnv[i]);outL+=al[i]*g;outR+=ar[i]*g;}
   l+=(outL-l)*p.mix;r+=(outR-r)*p.mix;return;
  }
  if(m.architecture==5){
   const float split=1.f-std::exp(-6.283185307f*220.f/sr);
   lowL+=split*(l-lowL);lowR+=split*(r-lowR);detLowL+=split*(dl-detLowL);detLowR+=split*(dr-detLowR);
   const float peaks[]={std::max(std::abs(detLowL),std::abs(detLowR)),std::max(std::abs(dl-detLowL),std::abs(dr-detLowR))};
   float g[2]{};lastDetector=std::max(peaks[0],peaks[1]);
   const float rmsC=coefficient(15.f,sr);
   for(int b=0;b<2;++b){
    bandPower[b]=rmsC*bandPower[b]+(1-rmsC)*peaks[b]*peaks[b];
    float level=(1-m.rms)*peaks[b]+m.rms*std::sqrt(std::max(0.f,bandPower[b]));
    float target=reduction(db(level)-p.threshold,std::clamp(p.ratio,1.f,20.f),m.knee);
    float c=coefficient(target>bandEnv[b]?p.attack*m.attackScale:p.release*m.releaseScale,sr);
    bandEnv[b]=c*bandEnv[b]+(1-c)*target;
    float sc=coefficient(p.release*m.releaseScale*5.f,sr);bandSlow[b]=sc*bandSlow[b]+(1-sc)*target;
    g[b]=std::max(bandEnv[b],m.memory*bandSlow[b]);
   }
   applied=std::max(g[0],g[1]);const float a=gain(p.makeup-g[0]),b=gain(p.makeup-g[1]),mix=std::clamp(p.mix,0.f,1.f);
   l+=(lowL*a+(l-lowL)*b-l)*mix;r+=(lowR*a+(r-lowR)*b-r)*mix;return;
  }
  float peak=std::max(std::abs(dl),std::abs(dr));
  if(m.architecture==3||m.architecture==4){
   const float hz=m.architecture==3?100.f:2500.f;const float c=1.f-std::exp(-6.283185307f*hz/sr);
   detLowL+=c*(dl-detLowL);detLowR+=c*(dr-detLowR);
   peak=std::max(std::abs(dl-detLowL),std::abs(dr-detLowR));
  }
  lastDetector=peak;
  const float detector=(m.feedback&&!external)?peak*gain(-previous):peak;
  const float rmsC=coefficient(15.f,sr);
  power=rmsC*power+(1-rmsC)*detector*detector;
  float level=(1.f-m.rms)*detector+m.rms*std::sqrt(std::max(0.f,power));
  if(m.architecture==2){const float rms=std::sqrt(std::max(0.f,power));level*=1.f+.35f*std::clamp(detector/std::max(.001f,rms)-1.f,0.f,3.f);}
  const float ratio=characterRatio(db(level)-p.threshold,p);
  const float effective=m.feedback?1.f+std::min(100.f,ratio-1.f)*2.f:ratio;
  const float target=reduction(db(level)-p.threshold,effective,m.knee);
  if(cachedAttack!=p.attack*m.attackScale||cachedRelease!=p.release*m.releaseScale||cachedRate!=sr){cachedAttack=p.attack*m.attackScale;cachedRelease=p.release*m.releaseScale;cachedRate=sr;cachedA=coefficient(cachedAttack,sr);cachedR=coefficient(cachedRelease,sr);cachedSlow=coefficient(cachedRelease*5,sr);}
  const float a=cachedA,rel=m.architecture==1?coefficient(cachedRelease*(1.f+std::min(3.f,slow*.12f)),sr):cachedR,slowC=cachedSlow;
  const float c=target>env?a:rel;
  env=c*env+(1.f-c)*target;slow=slowC*slow+(1-slowC)*target;
  applied=std::max(env,m.memory*slow);if(m.architecture==6){float duty=gain(-applied);applied=-db(std::clamp(duty,.01f,1.f));}previous=applied;
  const float g=gain(p.makeup-applied),mix=std::clamp(p.mix,0.f,1.f),drive=m.drive*std::clamp(p.color,0.f,1.f);
  auto shape=[&](float x){if(drive<=0.f)return x;const float d=1.f+drive*4.f;return std::tanh(d*x)/d;};
  if(p.analog&&analogCircuit(characterProfile(p.model).circuit)){
   const auto& profile=characterProfile(p.model);
   // Remove only the nonlinear residual's DC; preserve the linear path exactly.
   float a=l*g,b=r*g;
   float residualL=characterShape(a,profile,p.character,p.color)-a,residualR=characterShape(b,profile,p.character,p.color)-b;
   float wa=a+dcL.process(residualL,dcPole),wb=b+dcR.process(residualR,dcPole);
   l+=(wa-l)*mix;r+=(wb-r)*mix;
  }else{l+=(shape(l*g)-l)*mix;r+=(shape(r*g)-r)*mix;}
 }
 void process(float& l,float& r,const Parameters& p,float scL=0,float scR=0,bool external=false,int lookahead=0,int spectralPad=0){
  const auto& m=models[size_t(std::clamp(p.model,0,modelCount-1))];
  if(!p.enabled&&!wasEnabled&&lookahead==0&&!spectralPad){applied=lastDetector=0;return;}
  if(p.model!=lastModel||p.enabled!=wasEnabled){reset();lastModel=p.model;wasEnabled=p.enabled;lastAnalog=p.analog;}if(lastAnalog!=p.analog){dcL.reset();dcR.reset();lastAnalog=p.analog;}
  bool proOn=p.enabled&&p.pro.enabled&&(m.architecture==5||m.architecture>=7);
  int fftSize=proOn&&m.architecture==9?proFFT(p.pro.fft):Spectral::N;
  if(lastPro!=proOn||lastProSize!=fftSize||lastProCount!=proCount(p.pro)){proSpectral.reset();proMulti.reset();proEQ.reset();proMonitorDelay.reset();lastPro=proOn;lastProSize=fftSize;lastProCount=proCount(p.pro);}
  if(!proOn&&!lookahead&&!spectralPad&&m.architecture!=9&&!p.stereoMode&&!p.autoGain&&!p.delta){core(l,r,p,scL,scR,external);return;}
  float detectorL=external?scL:l,detectorR=external?scR:r;
  float dryL=l,dryR=r;const bool spectralOn=p.enabled&&m.architecture==9;
  const int padding=std::max(spectralPad,spectralOn?fftSize:0);const int total=lookahead+padding;
  dryDelay.process(dryL,dryR,total);
  if(!p.enabled){l=dryL;r=dryR;return;}
  signalDelay.process(l,r,lookahead+padding-(spectralOn?fftSize:0));
  // Detector leads the delayed signal by the lookahead only. Spectral padding
  // on nonspectral stages is compensation; delay detector with the signal
  // when no explicit external detector or lookahead is selected.
  bool useDetector=external||lookahead>0;
  if(padding>(spectralOn?fftSize:0)&&!external&&lookahead==0){detectorL=l;detectorR=r;}
  auto wet=p;wet.mix=1;float keep=0;bool monitoring=proOn&&p.pro.listen;int stereoMode=monitoring?0:p.stereoMode;
  if(stereoMode){float mid=.5f*(l+r),side=.5f*(l-r);keep=stereoMode==1?.5f*(dryL-dryR):.5f*(dryL+dryR);l=r=stereoMode==1?mid:side;
   float dd=stereoMode==1?.5f*(detectorL+detectorR):.5f*(detectorL-detectorR);detectorL=detectorR=dd;}
  core(l,r,wet,detectorL,detectorR,useDetector);
  if(stereoMode){float z=.5f*(l+r);if(stereoMode==1){l=z+keep;r=z-keep;}else{l=keep+z;r=keep-z;}}
  if(monitoring)return;
  if(p.autoGain){float c=1-std::exp(-1.f/(.4f*sr));inPower+=c*(.5f*(dryL*dryL+dryR*dryR)-inPower);outPower+=c*(.5f*(l*l+r*r)-outPower);
   float target=inPower>1e-7f&&outPower>1e-9f?std::clamp(10.f*std::log10(inPower/outPower),-12.f,12.f):0;autoDb+=c*(target-autoDb);float g=gain(autoDb);l*=g;r*=g;}
  l=dryL+(l-dryL)*p.mix;r=dryR+(r-dryR)*p.mix;if(p.delta){l=dryL-l;r=dryR-r;}
 }

};
class Rack{
 std::array<Slot,8> slots;SampleDelay parallelDelay;std::array<SampleDelay,4> branchDelays;int lookahead=0,spectralPad=0;RoutePlan plan;std::array<int,4> counts{};std::array<int,8> ordered{{0,1,2,3,4,5,6,7}};int longest=8,populated=1;float routeSmoothing=.002f;std::array<std::array<float,2>,4> currentGain{},targetGain{};bool gainsReady=false;
public:
 std::array<Parameters,8> params{};std::array<bool,8> external{};bool parallel=false;float balance=.5f;std::array<BranchControls,4> branchControls{};
 void prepare(float sampleRate){for(auto& s:slots)s.prepare(sampleRate);parallelDelay.prepare(4*(int(sampleRate*.01f)+4096+8));for(auto& d:branchDelays)d.prepare(8*(int(sampleRate*.01f)+4096+8));routeSmoothing=1-std::exp(-1.f/(.01f*std::max(1.f,sampleRate)));gainsReady=false;}
 void reset(){for(auto& s:slots)s.reset();parallelDelay.reset();for(auto& d:branchDelays)d.reset();gainsReady=false;}
 void configureRouting(const RoutePlan& next,const std::array<BranchControls,4>& controls){branchControls=controls;if(!plan.sameTopology(next)){plan=next;counts=plan.counts();ordered=plan.ordered();longest=*std::max_element(counts.begin(),counts.end());populated=int(std::count_if(counts.begin(),counts.end(),[](int n){return n>0;}));reset();}bool solo=false;for(int b=0;b<4;++b)solo|=counts[b]>0&&controls[b].solo;for(int b=0;b<4;++b){targetGain[b]=routeGain(controls[b],counts[b]>0&&!controls[b].mute&&(!solo||controls[b].solo),populated);if(!gainsReady)currentGain[b]=targetGain[b];}gainsReady=true;}
 void configureLatency(int la,bool spectral,int maxFFT=Spectral::N){int pad=spectral?std::clamp(maxFFT,Spectral::N,4096):0;if(la!=lookahead||pad!=spectralPad){lookahead=la;spectralPad=pad;reset();}}
 int latency()const{return (plan.enabled?longest:8)*(lookahead+spectralPad);}
 float gr(size_t i)const{return slots.at(i).gr();}
 float detectorLevel(size_t i)const{return slots.at(i).detectorLevel();}
 float proBandMeter(size_t i,int group)const{return slots.at(i).proBandMeter(group);}
 float displayReduction(size_t i,int group)const{return slots.at(i).displayReduction(params.at(i).model,group);}
 void process(float& l,float& r,float scL=0.f,float scR=0.f,bool hasExternal=false){
  if(plan.enabled){float inputL=l,inputR=r;l=r=0;for(int b=0;b<4;++b){if(counts[b]==0)continue;float a=inputL,c=inputR;for(int i:ordered)if(plan.branch[i]==b)slots[size_t(i)].process(a,c,params[size_t(i)],scL,scR,hasExternal&&external[size_t(i)],lookahead,spectralPad);branchDelays[b].process(a,c,(longest-counts[b])*(lookahead+spectralPad));for(int ch=0;ch<2;++ch)currentGain[b][ch]+=routeSmoothing*(targetGain[b][ch]-currentGain[b][ch]);l+=a*currentGain[b][0];r+=c*currentGain[b][1];}return;}
  if(!parallel){for(size_t i=0;i<8;++i)slots[i].process(l,r,params[i],scL,scR,hasExternal&&external[i],lookahead,spectralPad);return;}
  float aL=l,aR=r,bL=l,bR=r;
  for(size_t i=0;i<4;++i)slots[i].process(aL,aR,params[i],scL,scR,hasExternal&&external[i],lookahead,spectralPad);
  for(size_t i=4;i<8;++i)slots[i].process(bL,bR,params[i],scL,scR,hasExternal&&external[i],lookahead,spectralPad);
  const float b=std::clamp(balance,0.f,1.f);l=aL+(bL-aL)*b;r=aR+(bR-aR)*b;parallelDelay.process(l,r,4*(lookahead+spectralPad));
 }
};
}
