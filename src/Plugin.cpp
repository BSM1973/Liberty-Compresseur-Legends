#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "Dynamics.h"
#include "FactoryPresets.h"
#include "Intelligence.h"
class Processor final: public juce::AudioProcessor {
public:
 using APVTS=juce::AudioProcessorValueTreeState;
 APVTS state;
 liberty::AudioTelemetry telemetry;
 std::array<std::atomic<float>,8> meters{},detectors{};
 std::array<std::array<std::atomic<float>,16>,8> displayMeters{};
 std::array<std::array<std::atomic<float>,6>,8> proMeters{};std::atomic<float> processingRate{48000};
 std::atomic<bool> sidechainAvailable{false};
 static constexpr int midiTargets=66;
 std::array<std::atomic<int>,midiTargets> midiCC{};
 std::atomic<int> learnTarget{-1};
 std::array<juce::RangedAudioParameter*,midiTargets> midiParams{};
 static juce::String id(int slot,const char* name){return "slot"+juce::String(slot+1)+"_"+name;}
 static juce::String bandKey(int band,const char* name){return "b"+juce::String(band+1)+"_"+name;}
 static juce::String crossKey(int i){return "cross"+juce::String(i+1);}
 static constexpr int proGlobals=9,proFields=7;
private:
 std::array<liberty::Rack,5> racks;
 std::array<std::unique_ptr<juce::dsp::Oversampling<float>>,4> oversamplers;
 juce::AudioBuffer<float> scratch,dryScratch;liberty::SampleDelay masterDry;juce::SmoothedValue<float> masterGain,masterWet;std::atomic<float>* outputGain=nullptr;std::atomic<float>* masterBypass=nullptr;int masterDelay=-1;int maxBlock=0,lastQuality=-1;double sampleRate=48000;
 std::atomic<float>* quality=nullptr;std::atomic<float>* lookahead=nullptr;
 std::array<std::array<std::atomic<float>*,6>,8> advancedValues{};
 std::array<std::array<std::atomic<float>*,proGlobals+5+6*proFields>,8> proValues{};
 static APVTS::ParameterLayout layout(){
  APVTS::ParameterLayout out;
  juce::StringArray names;for(auto& model:liberty::models)names.add(model.name);
  auto addFloat=[&](int i,const char* key,const char* label,float lo,float hi,float step,float def){out.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id(i,key),1},juce::String(i+1)+" / "+juce::String::fromUTF8(label),juce::NormalisableRange<float>(lo,hi,step),def));};
  for(int i=0;i<8;++i){
   auto prefix=juce::String(i+1)+" / ";
   out.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{id(i,"on"),1},prefix+juce::String::fromUTF8("Activé"),i==0));
   out.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{id(i,"model"),1},prefix+juce::String::fromUTF8("Modèle"),names,i));
   addFloat(i,"threshold","Seuil",-60,0,.1f,-18);addFloat(i,"ratio","Ratio",1,20,.1f,4);
   addFloat(i,"attack","Attaque (ms)",.1f,100,.1f,10);addFloat(i,"release","Relâchement (ms)",5,2000,1,100);
   addFloat(i,"makeup","Gain (dB)",-12,24,.1f,0);addFloat(i,"mix","Mélange",0,1,.01f,1);addFloat(i,"color","Couleur",0,1,.01f,0);
   out.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{id(i,"external"),1},"Sidechain EXT "+juce::String(i+1),false));
   out.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{id(i,"stereo"),1},"Stereo / Mid / Side "+juce::String(i+1),juce::StringArray{"Stereo","Mid","Side"},0));
   out.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{id(i,"auto"),1},"Auto Gain "+juce::String(i+1),false));
   out.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{id(i,"delta"),1},"Delta "+juce::String(i+1),false));
   out.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{id(i,"analog"),1},"Analog Perfection "+juce::String(i+1),false));
   addFloat(i,"character","Character",0,1,.01f,.65f);
   out.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{id(i,"allButtons"),1},"FET All Buttons "+juce::String(i+1),false));
   addFloat(i,"pro","Pro",0.f,1.f,1.f,0.f);
   addFloat(i,"bandCount","Bands",2.f,6.f,1.f,3.f);
   addFloat(i,"fft","FFT",0.f,3.f,1.f,1.f);
   addFloat(i,"link","Stereo Link",0.f,1.f,0.01f,1.f);
   addFloat(i,"scHP","SC HP",20.f,1000.f,1.f,20.f);
   addFloat(i,"scLP","SC LP",1000.f,20000.f,1.f,20000.f);
   addFloat(i,"scBell","SC Bell",40.f,16000.f,1.f,1000.f);
   addFloat(i,"scGain","SC Gain",-18.f,18.f,0.1f,0.f);
   addFloat(i,"scListen","SC Listen",0.f,1.f,1.f,0.f);
   const float crossDefaults[]={120,350,1000,3500,8000};for(int j=0;j<5;++j){auto k=crossKey(j);addFloat(i,k.toRawUTF8(),"Crossover",20,20000,1,crossDefaults[j]);}
   for(int j=0;j<6;++j){{auto k=bandKey(j,"threshold");addFloat(i,k.toRawUTF8(),"Band Threshold",-60.f,0.f,0.1f,-18.f);}{auto k=bandKey(j,"ratio");addFloat(i,k.toRawUTF8(),"Band Ratio",1.f,20.f,0.1f,4.f);}{auto k=bandKey(j,"attack");addFloat(i,k.toRawUTF8(),"Band Attack",0.1f,100.f,0.1f,10.f);}{auto k=bandKey(j,"release");addFloat(i,k.toRawUTF8(),"Band Release",5.f,2000.f,1.f,100.f);}{auto k=bandKey(j,"solo");addFloat(i,k.toRawUTF8(),"Band Solo",0.f,1.f,1.f,0.f);}{auto k=bandKey(j,"mute");addFloat(i,k.toRawUTF8(),"Band Mute",0.f,1.f,1.f,0.f);}{auto k=bandKey(j,"bypass");addFloat(i,k.toRawUTF8(),"Band Bypass",0.f,1.f,1.f,0.f);}}
  }
  out.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"parallel",1},juce::String::fromUTF8("Parallèle"),false));
  out.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"balance",1},"Balance A/B",juce::NormalisableRange<float>(0,1,.01f),.5f));
  out.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"quality",1},"Oversampling",juce::StringArray{"1x","2x","4x","8x","16x"},0));
  out.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lookahead",1},"Lookahead (ms)",juce::NormalisableRange<float>(0,5,.5f),0));
  out.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"masterGain",1},"Master output (dB)",juce::NormalisableRange<float>(-24,24,.1f),0));
  out.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"masterBypass",1},"Master bypass",false));
  return out;
 }
 std::array<std::array<std::atomic<float>*,9>,8> values{};
 std::array<std::atomic<float>*,8> externalValues{};
 std::atomic<float>* parallel=nullptr;std::atomic<float>* balance=nullptr;
public:
 Processor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withInput("Sidechain",juce::AudioChannelSet::stereo(),false).withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"Parameters",layout()){
  const char* keys[]={"on","model","threshold","ratio","attack","release","makeup","mix","color"};
  for(int i=0;i<8;++i){meters[i].store(0);for(int j=0;j<9;++j)values[i][j]=state.getRawParameterValue(id(i,keys[j]));}
  parallel=state.getRawParameterValue("parallel");balance=state.getRawParameterValue("balance");
  for(auto& cc:midiCC)cc.store(-1);
  const char* learnKeys[]={"threshold","ratio","attack","release","makeup","mix","color"};
  for(int i=0;i<8;++i){externalValues[i]=state.getRawParameterValue(id(i,"external"));detectors[i].store(0);for(int k=0;k<7;++k)midiParams[i*7+k]=state.getParameter(id(i,learnKeys[k]));}midiParams[56]=state.getParameter("balance");midiParams[57]=state.getParameter("masterGain");outputGain=state.getRawParameterValue("masterGain");masterBypass=state.getRawParameterValue("masterBypass");for(auto& slot:displayMeters)for(auto& meter:slot)meter.store(0);quality=state.getRawParameterValue("quality");lookahead=state.getRawParameterValue("lookahead");for(int i=0;i<8;++i){advancedValues[i][0]=state.getRawParameterValue(id(i,"stereo"));advancedValues[i][1]=state.getRawParameterValue(id(i,"auto"));advancedValues[i][2]=state.getRawParameterValue(id(i,"delta"));advancedValues[i][3]=state.getRawParameterValue(id(i,"analog"));advancedValues[i][4]=state.getRawParameterValue(id(i,"character"));advancedValues[i][5]=state.getRawParameterValue(id(i,"allButtons"));midiParams[58+i]=state.getParameter(id(i,"character"));
   const char* keys[]={"pro","bandCount","fft","link","scHP","scLP","scBell","scGain","scListen"};for(int k=0;k<proGlobals;++k)proValues[i][k]=state.getRawParameterValue(id(i,keys[k]));
   for(int k=0;k<5;++k)proValues[i][proGlobals+k]=state.getRawParameterValue(id(i,crossKey(k).toRawUTF8()));
   const char* fields[]={"threshold","ratio","attack","release","solo","mute","bypass"};for(int b=0;b<6;++b){proMeters[i][b].store(0);for(int k=0;k<proFields;++k)proValues[i][proGlobals+5+b*proFields+k]=state.getRawParameterValue(id(i,bandKey(b,fields[k]).toRawUTF8()));}
  }
 }
 int midiIndex(juce::RangedAudioParameter* a)const{for(int i=0;i<midiTargets;++i)if(midiParams[i]==a)return i;return -1;}
 void handleMidi(const juce::MidiBuffer& midi){
  for(const auto meta:midi){auto msg=meta.getMessage();if(!msg.isController())continue;
   const int cc=(msg.getChannel()-1)*128+msg.getControllerNumber();
   const int target=learnTarget.exchange(-1);
   if(target>=0&&target<midiTargets){for(auto& existing:midiCC)if(existing.load()==cc)existing.store(-1);midiCC[target].store(cc);}
   for(int i=0;i<midiTargets;++i)if(midiCC[i].load()==cc&&midiParams[i])midiParams[i]->setValueNotifyingHost(msg.getControllerValue()/127.f);
  }
 }
 void loadFactory(int index){
  const auto preset=liberty::factoryPreset(index);
  auto put=[&](const juce::String& key,float value){if(auto* a=state.getParameter(key)){a->beginChangeGesture();a->setValueNotifyingHost(a->convertTo0to1(value));a->endChangeGesture();}};
  for(int i=0;i<8;++i){const auto& s=preset.slots[size_t(i)];put(id(i,"on"),s.enabled?1.f:0.f);put(id(i,"model"),float(s.model));put(id(i,"threshold"),s.threshold);put(id(i,"ratio"),s.ratio);put(id(i,"attack"),s.attack);put(id(i,"release"),s.release);put(id(i,"makeup"),s.makeup);put(id(i,"mix"),s.mix);put(id(i,"color"),s.color);put(id(i,"external"),0.f);put(id(i,"stereo"),float(s.stereoMode));put(id(i,"auto"),s.autoGain?1.f:0.f);put(id(i,"delta"),s.delta?1.f:0.f);put(id(i,"analog"),0.f);put(id(i,"character"),.65f);put(id(i,"allButtons"),0.f);
  const char* keys[]={"pro","bandCount","fft","link","scHP","scLP","scBell","scGain","scListen"};const float defaults[]={0.f,3.f,1.f,1.f,20.f,20000.f,1000.f,0.f,0.f};for(int k=0;k<proGlobals;++k)put(id(i,keys[k]),defaults[k]);
  const float cross[]={120,350,1000,3500,8000};for(int k=0;k<5;++k)put(id(i,crossKey(k).toRawUTF8()),cross[k]);
  const char* fields[]={"threshold","ratio","attack","release","solo","mute","bypass"};const float bd[]={-18.f,4.f,10.f,100.f,0.f,0.f,0.f};for(int b=0;b<6;++b)for(int k=0;k<proFields;++k)put(id(i,bandKey(b,fields[k]).toRawUTF8()),bd[k]);
 }
  put("parallel",preset.parallel?1.f:0.f);put("balance",preset.balance);put("quality",0);put("lookahead",0);put("masterGain",0);put("masterBypass",0);
 }
 const juce::String getName() const override{return "Compresseur Legends";}
 void prepareToPlay(double sr,int block)override{sampleRate=sr;telemetry.prepare(float(sr));maxBlock=std::max(1,block);scratch.setSize(4,maxBlock);dryScratch.setSize(2,maxBlock);masterDry.prepare(int(sr*.05)+32768+128);masterDelay=-1;masterGain.reset(sr,.02);masterWet.reset(sr,.02);masterGain.setCurrentAndTargetValue(liberty::gain(outputGain->load()));masterWet.setCurrentAndTargetValue(masterBypass->load()>.5f?0.f:1.f);for(int i=0;i<5;++i){racks[i].prepare(float(sr*(1<<i)));if(i){auto os=std::make_unique<juce::dsp::Oversampling<float>>(4,size_t(i),juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,true,true);os->initProcessing(size_t(maxBlock));os->reset();oversamplers[i-1]=std::move(os);}}lastQuality=-1;setLatencySamples(0);}
 void releaseResources() override{for(auto& rack:racks)rack.reset();for(auto& m:meters)m.store(0);for(auto& slot:displayMeters)for(auto& m:slot)m.store(0);masterDry.reset();for(auto& slot:proMeters)for(auto& m:slot)m.store(0);}
 bool isBusesLayoutSupported(const BusesLayout& b)const override{const auto main=b.getMainOutputChannelSet();const auto sc=b.getChannelSet(true,1);return b.getMainInputChannelSet()==main&&(main==juce::AudioChannelSet::mono()||main==juce::AudioChannelSet::stereo())&&(sc.isDisabled()||sc==juce::AudioChannelSet::mono()||sc==juce::AudioChannelSet::stereo());}
 void processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi)override{
  juce::ScopedNoDenormals guard;handleMidi(midi);midi.clear();
  const int q=std::clamp(int(quality->load()),0,4),factor=1<<q;auto& rack=racks[q];if(q!=lastQuality){rack.reset();if(q&&oversamplers[q-1])oversamplers[q-1]->reset();lastQuality=q;}
  rack.parallel=parallel->load()>.5f;rack.balance=balance->load();
  for(int i=0;i<8;++i){auto& p=rack.params[i];const auto& v=values[i];p.enabled=v[0]->load()>.5f;p.model=static_cast<int>(v[1]->load());p.threshold=v[2]->load();p.ratio=v[3]->load();p.attack=v[4]->load();p.release=v[5]->load();p.makeup=v[6]->load();p.mix=v[7]->load();p.color=v[8]->load();rack.external[i]=externalValues[i]->load()>.5f;p.stereoMode=int(advancedValues[i][0]->load());p.autoGain=advancedValues[i][1]->load()>.5f;p.delta=advancedValues[i][2]->load()>.5f;p.analog=advancedValues[i][3]->load()>.5f;p.character=advancedValues[i][4]->load();p.allButtons=advancedValues[i][5]->load()>.5f;
  auto& z=p.pro;const auto& w=proValues[i];z.enabled=w[0]->load()>.5f;z.count=int(w[1]->load());z.fft=512<<std::clamp(int(w[2]->load()),0,3);z.link=w[3]->load();z.hp=w[4]->load();z.lp=w[5]->load();z.bellHz=w[6]->load();z.bellGain=w[7]->load();z.listen=w[8]->load()>.5f;
  for(int k=0;k<5;++k)z.cross[k]=w[proGlobals+k]->load();for(int b=0;b<6;++b){auto& band=z.bands[b];int k=proGlobals+5+b*proFields;band.threshold=w[k]->load();band.ratio=w[k+1]->load();band.attack=w[k+2]->load();band.release=w[k+3]->load();band.solo=w[k+4]->load()>.5f;band.mute=w[k+5]->load()>.5f;band.bypass=w[k+6]->load()>.5f;}}
  bool spectral=false;int maxFFT=liberty::Spectral::N;for(auto& s:rack.params)if(s.enabled&&liberty::models[size_t(std::clamp(s.model,0,liberty::modelCount-1))].architecture==9){spectral=true;if(s.pro.enabled)maxFFT=std::max(maxFFT,liberty::proFFT(s.pro.fft));}processingRate.store(float(sampleRate*factor));
  int la=int(std::round(sampleRate*lookahead->load()*.001))*factor;rack.configureLatency(la,spectral,maxFFT);
  int latency=rack.latency()/factor+(q&&oversamplers[q-1]?int(std::round(oversamplers[q-1]->getLatencyInSamples())):0);if(latency!=getLatencySamples())setLatencySamples(latency);
  auto main=getBusBuffer(buffer,false,0);auto side=getBusBuffer(buffer,true,1);
  const bool has=side.getNumChannels()>0;sidechainAvailable.store(has);
  if(main.getNumChannels()==0)return;
  auto* l=main.getWritePointer(0);auto* r=main.getNumChannels()>1?main.getWritePointer(1):nullptr;
  const float* sl=has?side.getReadPointer(0):nullptr;const float* sr=side.getNumChannels()>1?side.getReadPointer(1):sl;
  if(maxBlock<=0)return;
  const bool analyze=telemetry.begin(r?2:1);
  if(masterDelay!=latency){masterDry.reset();masterDelay=latency;}
  masterGain.setTargetValue(liberty::gain(outputGain->load()));masterWet.setTargetValue(masterBypass->load()>.5f?0.f:1.f);
  for(int offset=0;offset<main.getNumSamples();offset+=maxBlock){
   int n=std::min(maxBlock,main.getNumSamples()-offset);
   for(int j=0;j<n;++j){int i=offset+j;float dl=l[i],dr=r?r[i]:dl;telemetry.input(dl,dr);masterDry.process(dl,dr,latency);dryScratch.setSample(0,j,dl);dryScratch.setSample(1,j,dr);}
   if(q==0||!oversamplers[q-1]){for(int j=0;j<n;++j){int i=offset+j;float left=l[i],right=r?r[i]:left;float a=sl?sl[i]:0,b=sr?sr[i]:a;rack.process(left,right,a,b,has);l[i]=left;if(r)r[i]=right;}}
   else{
    for(int j=0;j<n;++j){int i=offset+j;scratch.setSample(0,j,l[i]);scratch.setSample(1,j,r?r[i]:l[i]);scratch.setSample(2,j,sl?sl[i]:0);scratch.setSample(3,j,sr?sr[i]:0);}
    juce::dsp::AudioBlock<float> block(scratch);auto sub=block.getSubBlock(0,size_t(n));auto up=oversamplers[q-1]->processSamplesUp(sub);auto* ul=up.getChannelPointer(0);auto* ur=up.getChannelPointer(1);auto* usl=up.getChannelPointer(2);auto* usr=up.getChannelPointer(3);
    for(size_t j=0;j<up.getNumSamples();++j)rack.process(ul[j],ur[j],usl[j],usr[j],has);
    oversamplers[q-1]->processSamplesDown(sub);for(int j=0;j<n;++j){l[offset+j]=scratch.getSample(0,j);if(r)r[offset+j]=scratch.getSample(1,j);}
   }
   for(int j=0;j<n;++j){int i=offset+j;float gain=masterGain.getNextValue(),wet=masterWet.getNextValue();if(wet==1.f){l[i]*=gain;if(r)r[i]*=gain;}else{float dl=dryScratch.getSample(0,j),dr=dryScratch.getSample(1,j);l[i]=dl+(l[i]*gain-dl)*wet;if(r)r[i]=dr+(r[i]*gain-dr)*wet;}if(analyze)telemetry.sample(dryScratch.getSample(0,j),dryScratch.getSample(1,j),l[i],r?r[i]:l[i]);}
  }
  telemetry.end();
  for(int i=0;i<8;++i){meters[i].store(rack.gr(i),std::memory_order_relaxed);detectors[i].store(rack.detectorLevel(i),std::memory_order_relaxed);for(int group=0;group<16;++group)displayMeters[i][group].store(rack.displayReduction(size_t(i),group),std::memory_order_relaxed);for(int b=0;b<6;++b)proMeters[i][b].store(rack.proBandMeter(size_t(i),b),std::memory_order_relaxed);}
 }
 bool hasEditor()const override{return true;}
 juce::AudioProcessorEditor* createEditor()override;
 bool acceptsMidi()const override{return true;}bool producesMidi()const override{return false;}bool isMidiEffect()const override{return false;}
 double getTailLengthSeconds()const override{return 0;}
 int getNumPrograms()override{return 1;}int getCurrentProgram()override{return 0;}void setCurrentProgram(int)override{}const juce::String getProgramName(int)override{return "Initial";}void changeProgramName(int,const juce::String&)override{}
 void getStateInformation(juce::MemoryBlock& data)override{auto tree=state.copyState();for(int i=0;i<midiTargets;++i)tree.setProperty(juce::Identifier("cc"+juce::String(i)),midiCC[i].load(),nullptr);if(auto xml=tree.createXml())copyXmlToBinary(*xml,data);}
 void setStateInformation(const void* data,int size)override{if(auto xml=getXmlFromBinary(data,size))if(xml->hasTagName(state.state.getType())){auto tree=juce::ValueTree::fromXml(*xml);for(int i=0;i<midiTargets;++i)midiCC[i].store(juce::jlimit(-1,2047,int(tree.getProperty(juce::Identifier("cc"+juce::String(i)),-1))));learnTarget.store(-1);for(int i=0;i<8;++i)for(auto key:{"analog","character","allButtons"}){auto pid=id(i,key);bool found=false;for(int j=0;j<tree.getNumChildren();++j)if(tree.getChild(j).getProperty("id").toString()==pid){found=true;break;}if(!found){juce::ValueTree child("PARAM");child.setProperty("id",pid,nullptr);child.setProperty("value",juce::String(key)=="character"?.65:0.,nullptr);tree.addChild(child,-1,nullptr);}}for(auto* parameter:getParameters()){auto* a=dynamic_cast<juce::RangedAudioParameter*>(parameter);if(!a)continue;auto pid=a->getParameterID();bool found=false;for(int j=0;j<tree.getNumChildren();++j)if(tree.getChild(j).getProperty("id").toString()==pid){found=true;break;}if(!found){juce::ValueTree child("PARAM");child.setProperty("id",pid,nullptr);child.setProperty("value",a->convertFrom0to1(a->getDefaultValue()),nullptr);tree.addChild(child,-1,nullptr);}}state.replaceState(tree); }}
};
#include "Editor.h"
juce::AudioProcessorEditor* Processor::createEditor(){return new Editor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new Processor();}
