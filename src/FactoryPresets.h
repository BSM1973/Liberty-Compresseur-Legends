#pragma once
#include "Dynamics.h"
namespace liberty {
inline constexpr int factoryCount=100,categoryCount=10;
struct Category{const char* fr;const char* en;std::array<int,5> models;float threshold,ratio,attack,release,color;const char* prefix;};
inline constexpr std::array<Category,categoryCount> categories{{
 {"Voix","Vocals",{0,1,8,10,11},-22.0f,3.0f,12.0f,160.0f,0.1f,"Lead vocal"},
 {"Batteries","Drums",{3,8,13,16,0},-19.0f,4.0f,20.0f,95.0f,0.18f,"Drum punch"},
 {"Basses","Bass",{1,9,13,6,11},-23.0f,4.0f,12.0f,150.0f,0.15f,"Bass control"},
 {"Guitares","Guitars",{6,11,9,4,10},-20.0f,3.0f,18.0f,150.0f,0.12f,"Guitar level"},
 {"Claviers","Keys",{19,6,10,12,7},-21.0f,2.5f,22.0f,180.0f,0.05f,"Key dynamics"},
 {"Bus","Buses",{2,12,17,4,16},-17.0f,2.5f,27.0f,150.0f,0.12f,"Group glue"},
 {"Mixage","Mixing",{2,12,19,17,14},-15.0f,2.0f,30.0f,170.0f,0.05f,"Mix cohesion"},
 {"Mastering","Mastering",{12,14,19,7,2},-12.0f,1.5f,30.0f,220.0f,0.02f,"Master balance"},
 {"Créatif","Creative",{15,16,9,8,0},-28.0f,6.0f,7.0f,80.0f,0.55f,"Creative density"},
 {"Transitoires","Transients",{18,13,8,3,7},-17.0f,4.0f,8.0f,100.0f,0.05f,"Transient control"}
}};
inline constexpr std::array<const char*,10> variants{{"Gentle","Balanced","Focused","Firm","Dual Stage","Parallel Soft","Parallel Blend","Parallel Punch","Parallel Dense","Parallel Dual"}};
struct FactoryPreset{std::array<Parameters,8> slots{};bool parallel=false;float balance=.5f;};
inline FactoryPreset factoryPreset(int index){
 index=std::clamp(index,0,factoryCount-1);int c=index/10,v=index%10,k=v%5;const auto& cat=categories[size_t(c)];
 FactoryPreset p;for(int i=0;i<8;++i)p.slots[size_t(i)].model=i;
 p.parallel=v>=5;p.balance=p.parallel?.22f+.12f*k:.5f;
 auto make=[&](int model,float threshold,float ratio,float attack,float release,float mix,float color){
  Parameters s;s.model=model;s.enabled=true;s.threshold=std::clamp(threshold,-60.f,0.f);s.ratio=std::clamp(ratio,1.f,20.f);s.attack=std::clamp(attack,.1f,100.f);s.release=std::clamp(release,5.f,2000.f);s.mix=mix;s.color=std::clamp(color,0.f,1.f);s.makeup=0.f;return s;
 };
 p.slots[0]=make(cat.models[size_t(k)],cat.threshold+4.f-2.f*k,cat.ratio*(.8f+.18f*k),cat.attack*(1.25f-.13f*k),cat.release*(1.2f-.07f*k),1.f,cat.color*(.65f+.15f*k));
 if(!p.parallel&&k==4)p.slots[1]=make(cat.models[1],cat.threshold+3.f,1.7f,cat.attack*1.4f,cat.release*1.4f,.7f,cat.color*.5f);
 if(p.parallel){p.slots[4]=make(cat.models[size_t((k+2)%5)],cat.threshold-5.f-1.5f*k,cat.ratio+1.f+.6f*k,cat.attack*.65f,cat.release*.8f,1.f,cat.color);if(k==4)p.slots[5]=make(cat.models[0],cat.threshold,2.f,cat.attack,cat.release*1.5f,.7f,cat.color*.5f);}
 return p;
}
}
