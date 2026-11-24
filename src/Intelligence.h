#pragma once
#include "Dynamics.h"
#include <atomic>
#include <cstdint>
#include <limits>
namespace liberty {
struct DNAResult{float attackMs=0,releaseMs=0,gr=0,peakLoss=0,h2=0,h3=0,rmsLoss=0;};
inline DNAResult measureDNA(int model,bool analog){
 constexpr int sr=48000,step=48,attackN=24000,releaseN=96000;
 Slot slot;slot.prepare(sr);Parameters p;p.model=model;p.enabled=true;p.analog=analog;p.character=.65f;p.color=.7f;p.threshold=-24;p.ratio=4;p.attack=10;p.release=100;
 DNAResult d;std::array<float,attackN/step> attack{};std::array<float,releaseN/step> release{};
 double input=0,output=0;float peak=0;int steady=0;
 for(int n=0;n<attackN;++n){float x=.7f*std::sin(6.283185307179586*n/48.),l=x,r=x;slot.process(l,r,p);if(n%step==step-1)attack[n/step]=slot.gr();
  if(n>=attackN-4800){input+=x*x;output+=l*l;peak=std::max(peak,std::abs(l));d.gr+=slot.gr();++steady;}}
 d.gr/=float(steady);d.rmsLoss=10.f*std::log10(float(std::max(input,1e-12)/std::max(output,1e-12)));d.peakLoss=db(.7f)-db(peak);
 d.attackMs=500;for(size_t i=0;i<attack.size();++i)if(attack[i]>=d.gr*.63212f){d.attackMs=float(i+1);break;}
 for(int n=0;n<releaseN;++n){float l=0,r=0;slot.process(l,r,p);if(n%step==step-1)release[n/step]=slot.gr();}
 d.releaseMs=2000;for(size_t i=0;i<release.size();++i)if(release[i]<=d.gr*.36788f){d.releaseMs=float(i+1);break;}
 slot.reset();p.ratio=1;p.threshold=0;double re[4]{},im[4]{};
 for(int n=0;n<24000;++n){float l=.6f*std::sin(6.283185307179586*n/48.),r=l;slot.process(l,r,p);
  if(n>=12000)for(int h=1;h<=3;++h){double angle=6.283185307179586*n*h/48.;re[h]+=l*std::cos(angle);im[h]+=l*std::sin(angle);}}
 double fundamental=std::max(1e-12,std::hypot(re[1],im[1]));d.h2=float(100*std::hypot(re[2],im[2])/fundamental);d.h3=float(100*std::hypot(re[3],im[3])/fundamental);return d;
}
struct SignalFeatures{float peak=-180,rms=-180,crest=0,low=0,high=0;};
class FeatureCapture{
 double power=0,lowPower=0,highPower=0;float peak=0,lo[2]{},hi[2]{},lc=0,hc=0;int64_t samples=0,target=1;
public:
 void reset(float sr){power=lowPower=highPower=0;peak=0;lo[0]=lo[1]=hi[0]=hi[1]=0;samples=0;target=std::max<int64_t>(1,int64_t(sr*4));lc=1-std::exp(-6.283185307f*150/sr);hc=1-std::exp(-6.283185307f*3000/sr);}
 bool push(float l,float r){float x[2]={l,r};for(int c=0;c<2;++c){float a=x[c];lo[c]+=lc*(a-lo[c]);hi[c]+=hc*(a-hi[c]);power+=double(a)*a*.5;lowPower+=double(lo[c])*lo[c]*.5;float h=a-hi[c];highPower+=double(h)*h*.5;peak=std::max(peak,std::abs(a));}return ++samples>=target;}
 int progress()const{return int(std::min<int64_t>(100,samples*100/target));}
 SignalFeatures result()const{SignalFeatures f;f.peak=db(peak);f.rms=db(float(std::sqrt(power/std::max<int64_t>(1,samples))));f.crest=std::max(0.f,f.peak-f.rms);f.low=float(std::clamp(lowPower/std::max(1e-15,power),0.,1.));f.high=float(std::clamp(highPower/std::max(1e-15,power),0.,1.));return f;}
};
struct MatchResult{int model=7,second=1;float threshold=-18,ratio=3,attack=15,release=150;float secondThreshold=-18,secondRatio=2,secondAttack=20,secondRelease=200;float distance=0;};
inline std::array<MatchResult,3> matchSignal(const SignalFeatures& f,int goal){
 // Explicit heuristic objectives: smooth leveling / punch / peak control / bus glue.
 goal=std::clamp(goal,0,3);float desiredAttack[]={18,28,3,25},desiredRelease[]={220,100,90,180},desiredRms[]={.75f,.2f,.05f,.55f};
 float a=desiredAttack[goal],r=desiredRelease[goal];if(f.low>.4f)r*=1.35f;if(f.crest>16&&goal==2)a*=.65f;
 std::array<MatchResult,3> best{};for(auto& b:best)b.distance=1e9f;
 for(int i=0;i<modelCount;++i){const auto& m=models[size_t(i)];if(m.architecture>=5)continue; // no added latency or multiband assumptions
  float attack=std::clamp(a/m.attackScale,.1f,100.f),release=std::clamp(r/m.releaseScale,5.f,2000.f);
  float err=std::abs(std::log(std::max(.1f,attack*m.attackScale)/a))+std::abs(std::log(release*m.releaseScale/r));
  err+=std::abs(m.rms-desiredRms[goal])*2+std::abs(m.knee-(goal==0?10.f:goal==2?1.f:5.f))*.045f;
  if(goal==3)err+=m.drive*.8f;if(f.low>.4f&&m.architecture==3)err-=.35f;if(f.high>.3f&&m.architecture==4&&goal==2)err-=.12f;
  if(goal==2&&m.feedback)err+=.12f;err+=std::abs(std::log(std::max(.1f,m.attackScale)))*.08f;
  MatchResult x;x.model=i;x.attack=attack;x.release=release;x.ratio=goal==2?6:goal==3?2:goal==0?2.5f:4;
  float target=goal==3?2:goal==2?5:3;
  x.threshold=std::clamp(f.rms+(goal==2?.8f:.35f)*f.crest-target/(1-1/x.ratio),-60.f,0.f);
  x.second=goal==0?0:1;if(x.second==i)x.second=6;
  x.secondThreshold=std::clamp(f.rms-2.f,-60.f,0.f);x.distance=err;
  for(int j=0;j<3;++j)if(x.distance<best[j].distance){for(int k=2;k>j;--k)best[k]=best[k-1];best[j]=x;break;}
 }return best;
}
struct MeterBiquad{
 double b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;
 void clear(){z1=z2=0;}
 float tick(float x){double y=b0*x+z1;z1=b1*x-a1*y+z2;z2=b2*x-a2*y;return float(y);}
 void shelf(double sr){double K=std::tan(3.141592653589793*1681.974450955533/sr),Q=.7071752369554196,Vh=std::pow(10.,3.999843853973347/20),Vb=std::pow(Vh,.4996667741545416),d=1+K/Q+K*K;b0=(Vh+Vb*K/Q+K*K)/d;b1=2*(K*K-Vh)/d;b2=(Vh-Vb*K/Q+K*K)/d;a1=2*(K*K-1)/d;a2=(1-K/Q+K*K)/d;clear();}
 void highpass(double sr){double K=std::tan(3.141592653589793*38.13547087602444/sr),Q=.5003270373238773,d=1+K/Q+K*K;b0=1;b1=-2;b2=1;a1=2*(K*K-1)/d;a2=(1-K/Q+K*K)/d;clear();}
};
struct TelemetrySnapshot{static constexpr int N=2048;std::array<float,N> inL{},inR{},outL{},outR{};};
class AudioTelemetry{
 static constexpr int N=4096;
 struct Frame{std::atomic<float> inL{0},inR{0},outL{0},outR{0};};
 std::array<Frame,N> ring{};std::atomic<uint64_t> head{0},sequence{0};uint64_t write=0;
 MeterBiquad shelf[2][2],hp[2][2];double energy[2]{},square[2]{},corrL=0,corrR=0,corrLR=0;float peak[2]{},prev[2][2][4]{};
 std::array<std::array<double,30>,2> buckets{};int pos=0,filled=0,count=0,window=4800;float rate=48000;bool active=false;int channels=2;
 FeatureCapture feature;bool capturing=false;
 static float cubic(float a,float b,float c,float d,float t){return b+.5f*t*(c-a+t*(2*a-5*b+4*c-d+t*(3*(b-c)+d-a)));}
 void clearMeter(){for(int k=0;k<2;++k){energy[k]=square[k]=0;peak[k]=0;buckets[k].fill(0);for(int c=0;c<2;++c){shelf[k][c].clear();hp[k][c].clear();for(float& x:prev[k][c])x=0;}}pos=filled=count=0;corrL=corrR=corrLR=0;write=0;head.store(0);for(int k=0;k<2;++k){rms[k].store(-180);truePeak[k].store(-180);momentary[k].store(-180);shortTerm[k].store(-180);}correlation.store(0);meterReady.store(false);shortReady.store(false);}
public:
 std::atomic<bool> enabled{false},request{false},cancel{false},meterReady{false},shortReady{false};
 std::atomic<int> progress{-1};std::array<std::atomic<float>,2> rms{},truePeak{},momentary{},shortTerm{};std::atomic<float> correlation{0},capturePeak{-180},captureRms{-180},captureCrest{0},captureLow{0},captureHigh{0},sampleRate{48000};
 void prepare(float sr){rate=std::max(8000.f,sr);sampleRate.store(rate);window=std::max(1,int(rate*.1f));for(int k=0;k<2;++k)for(int c=0;c<2;++c){shelf[k][c].shelf(rate);hp[k][c].highpass(rate);}clearMeter();active=false;capturing=false;request.store(false);cancel.store(false);progress.store(-1);}
 bool begin(int channelCount){if(cancel.exchange(false)){capturing=false;progress.store(-1);}if(request.exchange(false)){feature.reset(rate);capturing=true;progress.store(0);}bool now=enabled.load();if(now){sequence.fetch_add(1,std::memory_order_acq_rel);if(!active||channels!=channelCount)clearMeter();}active=now;channels=channelCount;return now;}
 void input(float l,float r){if(!capturing)return;bool done=feature.push(l,r);if(!done)progress.store(feature.progress(),std::memory_order_relaxed);if(done){auto f=feature.result();capturePeak.store(f.peak);captureRms.store(f.rms);captureCrest.store(f.crest);captureLow.store(f.low);captureHigh.store(f.high);capturing=false;progress.store(100,std::memory_order_release);}}
 SignalFeatures features()const{return {capturePeak.load(),captureRms.load(),captureCrest.load(),captureLow.load(),captureHigh.load()};}
 void sample(float il,float ir,float ol,float orr){if(!active)return;auto& frame=ring[write%N];frame.inL.store(il,std::memory_order_relaxed);frame.inR.store(ir,std::memory_order_relaxed);frame.outL.store(ol,std::memory_order_relaxed);frame.outR.store(orr,std::memory_order_relaxed);++write;
  float x[2][2]={{il,ir},{ol,orr}};for(int k=0;k<2;++k)for(int c=0;c<channels;++c){float v=x[k][c];square[k]+=double(v)*v/channels;float y=hp[k][c].tick(shelf[k][c].tick(v));energy[k]+=double(y)*y;auto& t=prev[k][c];t[0]=t[1];t[1]=t[2];t[2]=t[3];t[3]=v;peak[k]=std::max(peak[k],std::abs(v));for(int j=1;j<4;++j)peak[k]=std::max(peak[k],std::abs(cubic(t[0],t[1],t[2],t[3],j*.25f)));}
  corrL+=double(ol)*ol;corrR+=double(orr)*orr;corrLR+=double(ol)*orr;
  if(++count>=window){filled=std::min(30,filled+1);for(int k=0;k<2;++k){buckets[k][pos]=energy[k]/count;double m=0,s=0;for(int i=0;i<filled;++i){double e=buckets[k][(pos+30-i)%30];s+=e;if(i<4)m+=e;}rms[k].store(db(float(std::sqrt(square[k]/count))));truePeak[k].store(db(peak[k]));momentary[k].store(float(-.691+10*std::log10(std::max(1e-18,m/std::min(4,filled)))));shortTerm[k].store(float(-.691+10*std::log10(std::max(1e-18,s/filled))));energy[k]=square[k]=0;peak[k]=0;}
   correlation.store(float(std::clamp(corrLR/std::sqrt(std::max(1e-24,corrL*corrR)),-1.,1.)));corrL=corrR=corrLR=0;pos=(pos+1)%30;count=0;meterReady.store(filled>=4);shortReady.store(filled>=30);}
 }
 void end(){if(active){head.store(write,std::memory_order_release);sequence.fetch_add(1,std::memory_order_release);}}
 bool snapshot(TelemetrySnapshot& s)const{auto a=sequence.load(std::memory_order_acquire);if(a&1)return false;auto h=head.load(std::memory_order_acquire);if(h<TelemetrySnapshot::N)return false;
  for(int i=0;i<TelemetrySnapshot::N;++i){auto& f=ring[(h-TelemetrySnapshot::N+i)%N];s.inL[i]=f.inL.load(std::memory_order_relaxed);s.inR[i]=f.inR.load(std::memory_order_relaxed);s.outL[i]=f.outL.load(std::memory_order_relaxed);s.outR[i]=f.outR.load(std::memory_order_relaxed);}return a==sequence.load(std::memory_order_acquire);}
};
inline std::array<float,1025> spectrum(const std::array<float,2048>& l,const std::array<float,2048>& r){
 constexpr int N=2048;using C=std::complex<float>;std::array<C,N> x{};for(int i=0;i<N;++i){float w=.5f-.5f*std::cos(6.283185307f*i/(N-1));x[i]=.5f*(l[i]+r[i])*w;}
 for(int i=1,j=0;i<N;++i){int bit=N>>1;for(;j&bit;bit>>=1)j^=bit;j^=bit;if(i<j)std::swap(x[i],x[j]);}
 for(int len=2;len<=N;len*=2){float a=-6.283185307f/len;C step(std::cos(a),std::sin(a));for(int i=0;i<N;i+=len){C w(1,0);for(int j=0;j<len/2;++j){auto u=x[i+j],v=x[i+j+len/2]*w;x[i+j]=u+v;x[i+j+len/2]=u-v;w*=step;}}}
 std::array<float,1025> result{};for(int i=0;i<=N/2;++i)result[i]=db(std::abs(x[i])*(i==0||i==N/2?2.f:4.f)/N);return result;
}
}
