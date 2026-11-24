#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
namespace liberty {
inline float db(float x){return 20.f*std::log10(std::max(x,1.e-9f));}
inline float gain(float x){return std::pow(10.f,x*.05f);}
struct Model{const char* name;float attackScale,releaseScale,knee,memory,drive,rms;bool feedback;int architecture=0;};
inline constexpr int modelCount=40;
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
 {"Transient Sculpt",0.25f,0.6f,1.0f,0.02f,0.0f,0.1f,false,2}
}};
struct Parameters{int model=0;bool enabled=false;float threshold=-18.f,ratio=4.f,attack=10.f,release=100.f,makeup=0.f,mix=1.f,color=0.f;};
inline float reduction(float over,float ratio,float knee){
 const float slope=1.f-1.f/std::max(1.f,ratio);
 if(knee<=0.f)return std::max(0.f,over)*slope;
 if(over<=-knee*.5f)return 0.f;
 if(over>=knee*.5f)return over*slope;
 const float x=over+knee*.5f;return slope*x*x/(2.f*knee);
}
// Nominal static detector-to-output curve; adaptive/weighted/band modes are signal-dependent.
inline float nominalReduction(float inputDb,const Model& m,const Parameters& p,bool external=false){
 const float ratio=std::clamp(p.ratio,1.f,20.f);
 const float effective=m.feedback?1.f+std::min(100.f,ratio-1.f)*2.f:ratio;
 if(!m.feedback||external)return reduction(inputDb-p.threshold,effective,m.knee);
 float lo=0,hi=std::max(0.f,inputDb-p.threshold+m.knee);
 for(int i=0;i<20;++i){float mid=(lo+hi)*.5f;if(reduction(inputDb-mid-p.threshold,effective,m.knee)>mid)lo=mid;else hi=mid;}
 return (lo+hi)*.5f;
}
class Slot{
 float sr=48000.f,env=0.f,slow=0.f,power=0.f,previous=0.f,applied=0.f;
 float lowL=0,lowR=0,detLowL=0,detLowR=0,bandEnv[2]{},bandPower[2]{},bandSlow[2]{},lastDetector=0;
 static float coefficient(float ms,float sr){return std::exp(-1000.f/(std::max(.05f,ms)*sr));}
public:
 void prepare(float rate){sr=std::max(1.f,rate);reset();}
 void reset(){env=slow=power=previous=applied=0.f;lowL=lowR=detLowL=detLowR=lastDetector=0;for(int i=0;i<2;++i)bandEnv[i]=bandPower[i]=bandSlow[i]=0;}
 float gr()const{return applied;}
 float detectorLevel()const{return lastDetector;}
 void process(float& l,float& r,const Parameters& p,float scL=0.f,float scR=0.f,bool external=false){
  if(!p.enabled){reset();return;}
  const auto& m=models[static_cast<size_t>(std::clamp(p.model,0,modelCount-1))];
  const float dl=external?scL:l,dr=external?scR:r;
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
  const float ratio=std::clamp(p.ratio,1.f,20.f);
  const float effective=m.feedback?1.f+std::min(100.f,ratio-1.f)*2.f:ratio;
  const float target=reduction(db(level)-p.threshold,effective,m.knee);
  const float a=coefficient(p.attack*m.attackScale,sr),rel=coefficient(p.release*m.releaseScale*(m.architecture==1?1.f+std::min(3.f,slow*.12f):1.f),sr),slowC=coefficient(p.release*m.releaseScale*5.f,sr);
  const float c=target>env?a:rel;
  env=c*env+(1.f-c)*target;slow=slowC*slow+(1-slowC)*target;
  applied=std::max(env,m.memory*slow);previous=applied;
  const float g=gain(p.makeup-applied),mix=std::clamp(p.mix,0.f,1.f),drive=m.drive*std::clamp(p.color,0.f,1.f);
  auto shape=[&](float x){if(drive<=0.f)return x;const float d=1.f+drive*4.f;return std::tanh(d*x)/d;};
  l+=(shape(l*g)-l)*mix;r+=(shape(r*g)-r)*mix;
 }
};
class Rack{
 std::array<Slot,8> slots;
public:
 std::array<Parameters,8> params{};std::array<bool,8> external{};bool parallel=false;float balance=.5f;
 void prepare(float sampleRate){for(auto& s:slots)s.prepare(sampleRate);}
 void reset(){for(auto& s:slots)s.reset();}
 float gr(size_t i)const{return slots.at(i).gr();}
 float detectorLevel(size_t i)const{return slots.at(i).detectorLevel();}
 void process(float& l,float& r,float scL=0.f,float scR=0.f,bool hasExternal=false){
  if(!parallel){for(size_t i=0;i<8;++i)slots[i].process(l,r,params[i],scL,scR,hasExternal&&external[i]);return;}
  float aL=l,aR=r,bL=l,bR=r;
  for(size_t i=0;i<4;++i)slots[i].process(aL,aR,params[i],scL,scR,hasExternal&&external[i]);
  for(size_t i=4;i<8;++i)slots[i].process(bL,bR,params[i],scL,scR,hasExternal&&external[i]);
  const float b=std::clamp(balance,0.f,1.f);l=aL+(bL-aL)*b;r=aR+(bR-aR)*b;
 }
};
}
