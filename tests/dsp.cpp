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
  require(std::abs(l)<.8f&&rack.gr(0)>0,"compression");require(std::abs(l+2*r)<1e-5f,"stereo link");
  std::cout<<models[i].name<<": GR "<<rack.gr(0)<<" dB\n";
  for(int n=0;n<480000;++n){l=r=0;rack.process(l,r);}require(rack.gr(0)<.05f,"release");
 }
 rack.reset();rack.params={};rack.parallel=true;rack.params[0].enabled=true;rack.balance=1;l=.7f;r=-.2f;rack.process(l,r);require(l==.7f&&r==-.2f,"parallel B");
 rack.balance=0;for(int n=0;n<96000;++n){l=.7f;r=-.2f;rack.process(l,r);}require(l<.7f,"parallel A");
 rack.params[0].mix=0;l=.7f;r=-.2f;rack.process(l,r);require(l==.7f&&r==-.2f,"dry mix");
 require(reduction(-10,4,0)==0&&std::abs(reduction(12,4,0)-9)<1e-6f,"static curve");
 std::set<std::string> fingerprints;int series=0,parallelCount=0;
 for(int i=0;i<factoryCount;++i){auto p=factoryPreset(i);p.parallel?++parallelCount:++series;std::ostringstream key;key<<p.parallel<<':'<<p.balance;
  require(p.slots[0].enabled,"preset A active");require(!p.parallel||p.slots[4].enabled,"preset B active");
  rack.reset();rack.params=p.slots;rack.parallel=p.parallel;rack.balance=p.balance;
  for(const auto& s:p.slots){require(s.model>=0&&s.model<modelCount,"valid model");require(s.threshold>=-60&&s.threshold<=0&&s.ratio>=1&&s.ratio<=20&&s.attack>=.1f&&s.attack<=100&&s.release>=5&&s.release<=2000&&s.mix>=0&&s.mix<=1&&s.color>=0&&s.color<=1,"preset ranges");key<<':'<<s.enabled<<':'<<s.model<<':'<<s.threshold<<':'<<s.ratio<<':'<<s.attack<<':'<<s.release<<':'<<s.mix<<':'<<s.color;}
  require(fingerprints.insert(key.str()).second,"unique preset state");
  for(int n=0;n<4096;++n){l=.7f*std::sin(float(n)*.07f);r=.5f*std::sin(float(n)*.11f);rack.process(l,r);require(std::isfinite(l)&&std::isfinite(r),"preset finite");}
 }
 require(series==50&&parallelCount==50,"50 series / 50 parallel");
 std::cout<<"20 profiles / 100 unique presets passed\\n";
 std::cout<<"DSP tests passed\n";
}
