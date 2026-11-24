#include "Dynamics.h"
#include "FactoryPresets.h"
#include "Intelligence.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <filesystem>
#include <vector>
using namespace liberty;
namespace fs=std::filesystem;
void check(bool ok,const char* msg){if(!ok)throw std::runtime_error(msg);}
constexpr double PI=3.14159265358979323846;
struct Stats{double in=0,out=0;float peak=0,gr=0;int64_t n=0;void add(float x,float y,float g){check(std::isfinite(y),"nonfinite output");in+=double(x)*x;out+=double(y)*y;peak=std::max(peak,std::abs(y));gr=std::max(gr,g);++n;}double loss()const{return 10*std::log10(std::max(in,1e-20)/std::max(out,1e-20));}};
int main(){fs::create_directories("qc");std::ofstream modelsCsv("qc/models.csv"),presetsCsv("qc/presets.csv"),cpu("qc/cpu.csv"),meter("qc/meter.csv");check(modelsCsv.good()&&presetsCsv.good()&&cpu.good()&&meter.good(),"reports writable");modelsCsv<<"model,id,mode,attack_ms,release_ms,gr_db,rms_loss_db,h2_pct,h3_pct,thd_h2_h9_pct,alias_probe_dbc,peak\n";presetsCsv<<"id,parallel,rate,peak,rms_loss_db,max_gr_db\n";cpu<<"scenario,rate,block,audio_seconds,elapsed_seconds,mean_budget_pct,p95_budget_pct,max_budget_pct,overruns\n";meter<<"rate,test,expected,measured,error\n";
 // Character measurements and high-frequency alias probe at base rate (not a full alias spectrum).
 for(int id=0;id<modelCount;++id)for(bool analog:{false,true}){auto dna=measureDNA(id,analog);Slot slot;slot.prepare(48000);Parameters p;p.enabled=true;p.model=id;p.analog=analog;p.color=1;p.character=1;p.ratio=1;p.threshold=0;double re[10]{},im[10]{},ar=0,ai=0,fr=0,fi=0;float peak=0;
 for(int n=0;n<48000;++n){float l=.65f*std::sin(2*PI*1000*n/48000),r=l;slot.process(l,r,p);check(std::isfinite(l)&&std::isfinite(r),"model tone finite");peak=std::max(peak,std::abs(l));if(n>=24000)for(int h=1;h<=9;++h){double a=2*PI*1000*h*n/48000;re[h]+=l*std::cos(a);im[h]+=l*std::sin(a);}}
 slot.reset();for(int n=0;n<48000;++n){float l=.65f*std::sin(2*PI*11000*n/48000),r=l;slot.process(l,r,p);check(std::isfinite(l),"high tone finite");if(n>=24000){double a=2*PI*15000*n/48000,b=2*PI*11000*n/48000;ar+=l*std::cos(a);ai+=l*std::sin(a);fr+=l*std::cos(b);fi+=l*std::sin(b);}}
 double fundamental=std::max(1e-12,std::hypot(re[1],im[1])),sum=0;for(int h=2;h<=9;++h)sum+=re[h]*re[h]+im[h]*im[h];double thd=100*std::sqrt(sum)/fundamental,alias=20*std::log10(std::max(1e-12,std::hypot(ar,ai))/std::max(1e-12,std::hypot(fr,fi)));check(std::isfinite(thd)&&std::isfinite(alias)&&std::isfinite(dna.gr),"model metrics finite");modelsCsv<<models[id].name<<','<<id<<','<<analog<<','<<dna.attackMs<<','<<dna.releaseMs<<','<<dna.gr<<','<<dna.rmsLoss<<','<<dna.h2<<','<<dna.h3<<','<<thd<<','<<alias<<','<<peak<<'\n';}
 // Every preset at three sample rates; deterministic transient and multitone stimulus.
 for(int sr:{44100,48000,96000}){Rack rack;rack.prepare(float(sr));for(int i=0;i<factoryCount;++i){auto preset=factoryPreset(i);rack.reset();rack.params=preset.slots;rack.parallel=preset.parallel;rack.balance=preset.balance;bool spectral=false;for(auto& p:rack.params)if(p.enabled&&models[p.model].architecture==9)spectral=true;rack.configureLatency(0,spectral);Stats z;for(int n=0;n<sr/4;++n){float x=.35f*std::sin(2*PI*83*n/sr)+.22f*std::sin(2*PI*997*n/sr)+.1f*std::sin(2*PI*6301*n/sr);if(n%(sr/16)==0)x+=.25f;float l=x,r=x*.72f;rack.process(l,r);check(std::isfinite(r),"preset right finite");float gr=0;for(int k=0;k<8;++k)gr=std::max(gr,rack.gr(k));z.add(x,l,gr);}presetsCsv<<i<<','<<preset.parallel<<','<<sr<<','<<z.peak<<','<<z.loss()<<','<<z.gr<<'\n';}}
 // Full racks: 60 seconds of processed audio, 128-sample callback simulation.
 for(int scenario=0;scenario<3;++scenario){int sr=48000,block=128;Rack rack;rack.prepare(float(sr));for(int i=0;i<8;++i){auto& p=rack.params[i];p.enabled=true;p.model=scenario==0?i:scenario==1?(i%2?56:54):i;p.analog=scenario==0;p.color=.5f;if(scenario==1){p.pro.enabled=true;p.pro.fft=4096;p.pro.count=6;}}
 RoutePlan route;route.enabled=scenario==2;route.branch={{0,0,0,0,1,2,3,3}};rack.configureRouting(route,{});rack.configureLatency(scenario==1?240:0,scenario==1,4096);
 std::vector<double> timings;timings.reserve(sr*60/block);double total=0,mx=0;int overruns=0;auto budget=double(block)/sr;float phase=0;for(int n=0;n<sr*60;n+=block){auto start=std::chrono::steady_clock::now();for(int j=0;j<block;++j){phase+=.013f;if(phase>6.2831853f)phase-=6.2831853f;float l=.6f*std::sin(phase),r=l*.8f;rack.process(l,r);check(std::isfinite(l)&&std::isfinite(r),"stress finite");}double t=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();timings.push_back(t);total+=t;mx=std::max(mx,t);if(t>budget)++overruns;}
 std::sort(timings.begin(),timings.end());cpu<<scenario<<','<<sr<<','<<block<<",60,"<<total<<','<<100*total/60<<','<<100*timings[size_t(.95*(timings.size()-1))]/budget<<','<<100*mx/budget<<','<<overruns<<'\n';}
 // Independent reference tone checks for K weighting and true peak at multiple rates.
 for(int sr:{44100,48000,96000}){AudioTelemetry t;t.prepare(float(sr));t.enabled.store(true);for(int n=0;n<sr*4;n+=100){t.begin(1);for(int j=0;j<100&&n+j<sr*4;++j){float x=std::sin(2*PI*997*(n+j)/sr);t.sample(x,x,x,x);}t.end();}
 float m=t.shortTerm[0].load();meter<<sr<<",mono_997Hz_lufs,-3.01,"<<m<<','<<m+3.01f<<'\n';check(std::abs(m+3.01f)<.20f,"997 Hz loudness anchor");
 TruePeakFIR fir;fir.prepare();float peak=0;for(int n=0;n<sr;++n){float x=.5f*std::sin(PI*.5*n+PI*.25);float p=fir.process(x);if(n>100)peak=std::max(peak,p);}float d=db(peak);meter<<sr<<",fs4_truepeak,-6.0206,"<<d<<','<<d+6.0206f<<'\n';check(std::abs(d+6.0206f)<.25f,"intersample peak anchor");}
 std::cout<<"QC: 120 model measurements, 1200 preset executions, 3 x 60s simulated rack stress, meter anchors. See qc/*.csv\n";
}
