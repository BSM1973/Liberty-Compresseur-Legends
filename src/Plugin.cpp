#include <juce_audio_utils/juce_audio_utils.h>
#include "Dynamics.h"
#include "FactoryPresets.h"
class Processor final: public juce::AudioProcessor {
public:
 using APVTS=juce::AudioProcessorValueTreeState;
 APVTS state;
 std::array<std::atomic<float>,8> meters{},detectors{};
 std::atomic<bool> sidechainAvailable{false};
 static constexpr int midiTargets=57;
 std::array<std::atomic<int>,midiTargets> midiCC{};
 std::atomic<int> learnTarget{-1};
 std::array<juce::RangedAudioParameter*,midiTargets> midiParams{};
 static juce::String id(int slot,const char* name){return "slot"+juce::String(slot+1)+"_"+name;}
private:
 liberty::Rack rack;
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
  }
  out.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"parallel",1},juce::String::fromUTF8("Parallèle"),false));
  out.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"balance",1},"Balance A/B",juce::NormalisableRange<float>(0,1,.01f),.5f));
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
  for(int i=0;i<8;++i){externalValues[i]=state.getRawParameterValue(id(i,"external"));detectors[i].store(0);for(int k=0;k<7;++k)midiParams[i*7+k]=state.getParameter(id(i,learnKeys[k]));}midiParams[56]=state.getParameter("balance");
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
  for(int i=0;i<8;++i){const auto& s=preset.slots[size_t(i)];put(id(i,"on"),s.enabled?1.f:0.f);put(id(i,"model"),float(s.model));put(id(i,"threshold"),s.threshold);put(id(i,"ratio"),s.ratio);put(id(i,"attack"),s.attack);put(id(i,"release"),s.release);put(id(i,"makeup"),s.makeup);put(id(i,"mix"),s.mix);put(id(i,"color"),s.color);put(id(i,"external"),0.f);}
  put("parallel",preset.parallel?1.f:0.f);put("balance",preset.balance);
 }
 const juce::String getName() const override{return "Compresseur Legends";}
 void prepareToPlay(double sr,int)override{rack.prepare(static_cast<float>(sr));}
 void releaseResources() override{rack.reset();for(auto& m:meters)m.store(0);}
 bool isBusesLayoutSupported(const BusesLayout& b)const override{const auto main=b.getMainOutputChannelSet();const auto sc=b.getChannelSet(true,1);return b.getMainInputChannelSet()==main&&(main==juce::AudioChannelSet::mono()||main==juce::AudioChannelSet::stereo())&&(sc.isDisabled()||sc==juce::AudioChannelSet::mono()||sc==juce::AudioChannelSet::stereo());}
 void processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi)override{
  juce::ScopedNoDenormals guard;handleMidi(midi);midi.clear();
  rack.parallel=parallel->load()>.5f;rack.balance=balance->load();
  for(int i=0;i<8;++i){auto& p=rack.params[i];const auto& v=values[i];p.enabled=v[0]->load()>.5f;p.model=static_cast<int>(v[1]->load());p.threshold=v[2]->load();p.ratio=v[3]->load();p.attack=v[4]->load();p.release=v[5]->load();p.makeup=v[6]->load();p.mix=v[7]->load();p.color=v[8]->load();rack.external[i]=externalValues[i]->load()>.5f;}
  auto main=getBusBuffer(buffer,false,0);auto side=getBusBuffer(buffer,true,1);
  const bool has=side.getNumChannels()>0;sidechainAvailable.store(has);
  if(main.getNumChannels()==0)return;
  auto* l=main.getWritePointer(0);auto* r=main.getNumChannels()>1?main.getWritePointer(1):nullptr;
  const float* sl=has?side.getReadPointer(0):nullptr;const float* sr=side.getNumChannels()>1?side.getReadPointer(1):sl;
  for(int i=0;i<main.getNumSamples();++i){float left=l[i],right=r?r[i]:left;float a=sl?sl[i]:0,b=sr?sr[i]:a;rack.process(left,right,a,b,has);l[i]=left;if(r)r[i]=right;}
  for(int i=0;i<8;++i){meters[i].store(rack.gr(i),std::memory_order_relaxed);detectors[i].store(rack.detectorLevel(i),std::memory_order_relaxed);}
 }
 bool hasEditor()const override{return true;}
 juce::AudioProcessorEditor* createEditor()override;
 bool acceptsMidi()const override{return true;}bool producesMidi()const override{return false;}bool isMidiEffect()const override{return false;}
 double getTailLengthSeconds()const override{return 0;}
 int getNumPrograms()override{return 1;}int getCurrentProgram()override{return 0;}void setCurrentProgram(int)override{}const juce::String getProgramName(int)override{return "Initial";}void changeProgramName(int,const juce::String&)override{}
 void getStateInformation(juce::MemoryBlock& data)override{auto tree=state.copyState();for(int i=0;i<midiTargets;++i)tree.setProperty(juce::Identifier("cc"+juce::String(i)),midiCC[i].load(),nullptr);if(auto xml=tree.createXml())copyXmlToBinary(*xml,data);}
 void setStateInformation(const void* data,int size)override{if(auto xml=getXmlFromBinary(data,size))if(xml->hasTagName(state.state.getType())){auto tree=juce::ValueTree::fromXml(*xml);for(int i=0;i<midiTargets;++i)midiCC[i].store(juce::jlimit(-1,2047,int(tree.getProperty(juce::Identifier("cc"+juce::String(i)),-1))));learnTarget.store(-1);state.replaceState(tree); }}
};
#include "Editor.h"
juce::AudioProcessorEditor* Processor::createEditor(){return new Editor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new Processor();}
