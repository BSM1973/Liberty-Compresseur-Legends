#include "Dynamics.h"
#include <iostream>
#include <stdexcept>
using namespace liberty;
void require(bool b,const char* msg){if(!b)throw std::runtime_error(msg);}
int main(){
 Rack rack;rack.prepare(48000);float l=.75f,r=-.3f;rack.process(l,r);require(l==.75f&&r==-.3f,"bypass");
 for(int i=0;i<8;++i){
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
 std::cout<<"DSP tests passed\n";
}
