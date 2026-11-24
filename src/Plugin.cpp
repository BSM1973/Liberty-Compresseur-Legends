#include <juce_audio_utils/juce_audio_utils.h>
#include "Dynamics.h"
class Processor final: public juce::AudioProcessor {
public:
 using APVTS=juce::AudioProcessorValueTreeState;
 APVTS state;
 std::array<std::atomic<float>,8> meters{};
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
  }
  out.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"parallel",1},juce::String::fromUTF8("Parallèle"),false));
  out.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"balance",1},"Balance A/B",juce::NormalisableRange<float>(0,1,.01f),.5f));
  return out;
 }
 std::array<std::array<std::atomic<float>*,9>,8> values{};
 std::atomic<float>* parallel=nullptr;std::atomic<float>* balance=nullptr;
public:
 Processor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"Parameters",layout()){
  const char* keys[]={"on","model","threshold","ratio","attack","release","makeup","mix","color"};
  for(int i=0;i<8;++i){meters[i].store(0);for(int j=0;j<9;++j)values[i][j]=state.getRawParameterValue(id(i,keys[j]));}
  parallel=state.getRawParameterValue("parallel");balance=state.getRawParameterValue("balance");
 }
 const juce::String getName() const override{return "Compresseur Legends";}
 void prepareToPlay(double sr,int)override{rack.prepare(static_cast<float>(sr));}
 void releaseResources() override{rack.reset();for(auto& m:meters)m.store(0);}
 bool isBusesLayoutSupported(const BusesLayout& b)const override{return b.getMainInputChannelSet()==b.getMainOutputChannelSet()&&(b.getMainOutputChannelSet()==juce::AudioChannelSet::mono()||b.getMainOutputChannelSet()==juce::AudioChannelSet::stereo());}
 void processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&)override{
  juce::ScopedNoDenormals guard;
  rack.parallel=parallel->load()>.5f;rack.balance=balance->load();
  for(int i=0;i<8;++i){auto& p=rack.params[i];const auto& v=values[i];p.enabled=v[0]->load()>.5f;p.model=static_cast<int>(v[1]->load());p.threshold=v[2]->load();p.ratio=v[3]->load();p.attack=v[4]->load();p.release=v[5]->load();p.makeup=v[6]->load();p.mix=v[7]->load();p.color=v[8]->load();}
  if(buffer.getNumChannels()==0)return;
  auto* l=buffer.getWritePointer(0);auto* r=buffer.getNumChannels()>1?buffer.getWritePointer(1):nullptr;
  for(int i=0;i<buffer.getNumSamples();++i){float left=l[i],right=r?r[i]:left;rack.process(left,right);l[i]=left;if(r)r[i]=right;}
  for(int i=0;i<8;++i)meters[i].store(rack.gr(i),std::memory_order_relaxed);
 }
 bool hasEditor()const override{return true;}
 juce::AudioProcessorEditor* createEditor()override;
 bool acceptsMidi()const override{return false;}bool producesMidi()const override{return false;}bool isMidiEffect()const override{return false;}
 double getTailLengthSeconds()const override{return 0;}
 int getNumPrograms()override{return 1;}int getCurrentProgram()override{return 0;}void setCurrentProgram(int)override{}const juce::String getProgramName(int)override{return "Initial";}void changeProgramName(int,const juce::String&)override{}
 void getStateInformation(juce::MemoryBlock& data)override{if(auto xml=state.copyState().createXml())copyXmlToBinary(*xml,data);}
 void setStateInformation(const void* data,int size)override{if(auto xml=getXmlFromBinary(data,size))if(xml->hasTagName(state.state.getType()))state.replaceState(juce::ValueTree::fromXml(*xml));}
};
#include "Editor.h"
juce::AudioProcessorEditor* Processor::createEditor(){return new Editor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new Processor();}
