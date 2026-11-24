#include <juce_audio_utils/juce_audio_utils.h>
#include <iostream>
#include <stdexcept>
extern juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();
void check(bool ok,const char* msg){if(!ok)throw std::runtime_error(msg);}
int main(){juce::ScopedJuceInitialiser_GUI init;std::unique_ptr<juce::AudioProcessor> a(createPluginFilter()),b(createPluginFilter());check(a&&b,"processors");auto set=[&](juce::AudioProcessor& p,const juce::String& id,float value){for(auto* x:p.getParameters())if(auto* q=dynamic_cast<juce::RangedAudioParameter*>(x))if(q->getParameterID()==id){q->setValueNotifyingHost(q->convertTo0to1(value));return;}throw std::runtime_error("parameter missing");};
 set(*a,"slot1_model",56);set(*a,"slot1_on",1);set(*a,"slot1_pro",1);set(*a,"slot1_fft",2);set(*a,"designer",1);set(*a,"slot1_routeBranch",3);set(*a,"route4_pan",.31f);set(*a,"masterGain",-3);
 juce::MemoryBlock data;a->getStateInformation(data);b->setStateInformation(data.getData(),int(data.getSize()));auto& ap=a->getParameters();auto& bp=b->getParameters();check(ap.size()==bp.size(),"parameter count");for(int i=0;i<ap.size();++i)check(std::abs(ap[i]->getValue()-bp[i]->getValue())<1e-5f,"roundtrip parameter");
 const char garbage[]="not a preset";b->setStateInformation(garbage,sizeof garbage);for(int i=0;i<ap.size();++i)check(std::abs(ap[i]->getValue()-bp[i]->getValue())<1e-5f,"invalid state ignored");
 for(int quality=0;quality<5;++quality){set(*b,"quality",float(quality));b->prepareToPlay(48000,128);juce::AudioBuffer<float> buffer(2,128);juce::MidiBuffer midi;for(int block=0;block<192;++block){for(int j=0;j<128;++j){float v=.5f*std::sin(float(block*128+j)*.13f);buffer.setSample(0,j,v);buffer.setSample(1,j,v*.7f);}b->processBlock(buffer,midi);for(int c=0;c<2;++c)for(int j=0;j<128;++j)check(std::isfinite(buffer.getSample(c,j)),"processor finite");}check(b->getLatencySamples()>=0,"latency nonnegative");b->releaseResources();}
 std::cout<<"Processor state roundtrip and invalid-state rejection; full processor smoke at x1/x2/x4/x8/x16 passed\n";
}
