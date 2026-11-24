#pragma once
#include <algorithm>
#include <array>
#include <cmath>
namespace liberty {
struct BranchControls{float db=0,pan=0;bool solo=false,mute=false,invert=false;};
struct RoutePlan{
 bool enabled=false;std::array<int,8> branch{{0,0,0,0,1,1,1,1}},order{{0,1,2,3,4,5,6,7}};
 bool sameTopology(const RoutePlan& p)const{return enabled==p.enabled&&branch==p.branch&&order==p.order;}
 std::array<int,4> counts()const{std::array<int,4> n{};for(int b:branch)++n[size_t(std::clamp(b,0,3))];return n;}
 std::array<int,8> ordered()const{std::array<int,8> x{{0,1,2,3,4,5,6,7}};std::sort(x.begin(),x.end(),[&](int a,int b){return order[a]==order[b]?a<b:order[a]<order[b];});return x;}
};
inline std::array<float,2> routeGain(const BranchControls& c,bool audible,int populated){
 float g=audible?std::pow(10.f,std::clamp(c.db,-60.f,12.f)*.05f)/std::max(1,populated):0.f;if(c.invert)g=-g;
 float pan=std::clamp(c.pan,-1.f,1.f);return {g*(1-std::max(0.f,pan)),g*(1+std::min(0.f,pan))};
}
}
