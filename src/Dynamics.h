#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
namespace liberty {
inline float db(float x){return 20.f*std::log10(std::max(x,1.e-9f));}
inline float gain(float x){return std::pow(10.f,x*.05f);}
struct Model{const char* name;float attackScale,releaseScale,knee,memory,drive,rms;bool feedback;};
inline constexpr int modelCount=20;
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
 {"Digital RMS",1.1f,1.25f,6.0f,0.15f,0.0f,0.95f,false}
}};
struct Parameters{int model=0;bool enabled=false;float threshold=-18.f,ratio=4.f,attack=10.f,release=100.f,makeup=0.f,mix=1.f,color=0.f;};
inline float reduction(float over,float ratio,float knee){
 const float slope=1.f-1.f/std::max(1.f,ratio);
 if(knee<=0.f)return std::max(0.f,over)*slope;
 if(over<=-knee*.5f)return 0.f;
 if(over>=knee*.5f)return over*slope;
 const float x=over+knee*.5f;return slope*x*x/(2.f*knee);
}
class Slot{
 float sr=48000.f,env=0.f,slow=0.f,power=0.f,previous=0.f,applied=0.f;
 static float coefficient(float ms,float sr){return std::exp(-1000.f/(std::max(.05f,ms)*sr));}
public:
 void prepare(float rate){sr=std::max(1.f,rate);reset();}
 void reset(){env=slow=power=previous=applied=0.f;}
 float gr()const{return applied;}
 void process(float& l,float& r,const Parameters& p){
  if(!p.enabled){reset();return;}
  const auto& m=models[static_cast<size_t>(std::clamp(p.model,0,modelCount-1))];
  const float peak=std::max(std::abs(l),std::abs(r));
  const float detector=m.feedback?peak*gain(-previous):peak;
  const float rmsC=coefficient(15.f,sr);
  power=rmsC*power+(1-rmsC)*detector*detector;
  const float level=(1.f-m.rms)*detector+m.rms*std::sqrt(std::max(0.f,power));
  const float ratio=std::clamp(p.ratio,1.f,20.f);
  const float effective=m.feedback?1.f+std::min(100.f,ratio-1.f)*2.f:ratio;
  const float target=reduction(db(level)-p.threshold,effective,m.knee);
  const float a=coefficient(p.attack*m.attackScale,sr),rel=coefficient(p.release*m.releaseScale,sr),slowC=coefficient(p.release*m.releaseScale*5.f,sr);
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
 std::array<Parameters,8> params{};bool parallel=false;float balance=.5f;
 void prepare(float sampleRate){for(auto& s:slots)s.prepare(sampleRate);}
 void reset(){for(auto& s:slots)s.reset();}
 float gr(size_t i)const{return slots.at(i).gr();}
 void process(float& l,float& r){
  if(!parallel){for(size_t i=0;i<8;++i)slots[i].process(l,r,params[i]);return;}
  float aL=l,aR=r,bL=l,bR=r;
  for(size_t i=0;i<4;++i)slots[i].process(aL,aR,params[i]);
  for(size_t i=4;i<8;++i)slots[i].process(bL,bR,params[i]);
  const float b=std::clamp(balance,0.f,1.f);l=aL+(bL-aL)*b;r=aR+(bR-aR)*b;
 }
};
}
