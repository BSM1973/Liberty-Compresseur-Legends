#pragma once
class Editor final:public juce::AudioProcessorEditor,private juce::Timer{
 using AP=Processor::APVTS;
 Processor& p;
 juce::LookAndFeel_V4 theme;
 std::array<juce::TextButton,8> slots;
 juce::TextButton language{"FR"};
 juce::ToggleButton active,parallel;
 juce::ComboBox model;
 juce::Slider balance;
 std::array<juce::Slider,7> knobs;
 std::array<juce::Label,7> labels;
 juce::Label balanceLabel;
 std::unique_ptr<AP::ButtonAttachment> activeBinding,parallelBinding;
 std::unique_ptr<AP::ComboBoxAttachment> modelBinding;
 std::unique_ptr<AP::SliderAttachment> balanceBinding;
 std::array<std::unique_ptr<AP::SliderAttachment>,7> bindings;
 int selected=0;bool french=true;
 static juce::String u(const char* s){return juce::String::fromUTF8(s);}
 static juce::Colour color(int i){
  constexpr unsigned colors[]={0xffa9253f,0xffded7c5,0xffaab2bc,0xffaab2bc,0xff976188,0xff53ab96,0xffded7c5,0xffa4a9b4};
  return juce::Colour(colors[juce::jlimit(0,7,i)]);
 }
 void bind(int i){
  selected=i;activeBinding.reset();modelBinding.reset();for(auto& b:bindings)b.reset();
  activeBinding=std::make_unique<AP::ButtonAttachment>(p.state,Processor::id(i,"on"),active);
  modelBinding=std::make_unique<AP::ComboBoxAttachment>(p.state,Processor::id(i,"model"),model);
  const char* keys[]={"threshold","ratio","attack","release","makeup","mix","color"};
  for(int k=0;k<7;++k)bindings[k]=std::make_unique<AP::SliderAttachment>(p.state,Processor::id(i,keys[k]),knobs[k]);
  refresh();
 }
 void translate(){
  const char* fr[]={"Seuil","Ratio","Attaque","Relâchement","Gain","Mélange","Couleur"};
  const char* en[]={"Threshold","Ratio","Attack","Release","Gain","Mix","Color"};
  for(int k=0;k<7;++k)labels[k].setText(u(french?fr[k]:en[k]),juce::dontSendNotification);
  active.setButtonText(u(french?"Activé":"Enabled"));parallel.setButtonText(u(french?"Parallèle A/B":"Parallel A/B"));
  balanceLabel.setText("Balance A/B",juce::dontSendNotification);language.setButtonText(french?"FR":"EN");repaint();
 }
 void refresh(){
  for(int i=0;i<8;++i){
   const int m=static_cast<int>(p.state.getRawParameterValue(Processor::id(i,"model"))->load());
   const bool on=p.state.getRawParameterValue(Processor::id(i,"on"))->load()>.5f;
   slots[i].setButtonText(juce::String(i+1)+"  "+liberty::models[juce::jlimit(0,7,m)].name);
   slots[i].setColour(juce::TextButton::buttonColourId,i==selected?juce::Colour(0xffa9253f):juce::Colour(0xff292e35));
   slots[i].setColour(juce::TextButton::textColourOffId,on?juce::Colours::white:juce::Colour(0xffaab2bc));
  }
  balance.setEnabled(parallel.getToggleState());
  repaint();
 }
 void timerCallback()override{refresh();}
public:
 explicit Editor(Processor& processor):AudioProcessorEditor(&processor),p(processor){
  theme.setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xffd6465c));
  theme.setColour(juce::Slider::rotarySliderOutlineColourId,juce::Colour(0xff4d535c));
  theme.setColour(juce::Slider::thumbColourId,juce::Colour(0xffd8dce0));
  theme.setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff22262c));
  theme.setColour(juce::Slider::textBoxTextColourId,juce::Colours::white);
  theme.setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff2a3038));
  theme.setColour(juce::PopupMenu::backgroundColourId,juce::Colour(0xff252a30));
  theme.setColour(juce::ToggleButton::textColourId,juce::Colours::white);
  setLookAndFeel(&theme);
  for(int i=0;i<8;++i){addAndMakeVisible(slots[i]);slots[i].onClick=[this,i]{bind(i);};}
  for(int i=0;i<8;++i)model.addItem(liberty::models[i].name,i+1);
  for(auto* c:std::array<juce::Component*,6>{&language,&active,&parallel,&model,&balance,&balanceLabel})addAndMakeVisible(c);
  language.onClick=[this]{french=!french;translate();};
  for(int k=0;k<7;++k){
   auto& s=knobs[k];s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,76,22);
   if(k==0||k==4)s.setTextValueSuffix(" dB");if(k==2||k==3)s.setTextValueSuffix(" ms");
   addAndMakeVisible(s);labels[k].setJustificationType(juce::Justification::centred);addAndMakeVisible(labels[k]);
  }
  balance.setSliderStyle(juce::Slider::LinearHorizontal);balance.setTextBoxStyle(juce::Slider::TextBoxRight,false,54,22);
  parallelBinding=std::make_unique<AP::ButtonAttachment>(p.state,"parallel",parallel);
  balanceBinding=std::make_unique<AP::SliderAttachment>(p.state,"balance",balance);
  translate();bind(0);setSize(940,480);startTimerHz(20);
 }
 ~Editor()override{stopTimer();setLookAndFeel(nullptr);}
 void resized()override{
  language.setBounds(872,18,48,28);
  for(int i=0;i<8;++i)slots[i].setBounds(20+(i%4)*226,68+(i/4)*34,218,28);
  model.setBounds(24,151,270,34);active.setBounds(304,151,120,34);
  for(int k=0;k<7;++k){labels[k].setBounds(24+k*128,260,124,24);knobs[k].setBounds(24+k*128,286,124,108);}
  parallel.setBounds(24,426,180,32);balanceLabel.setBounds(225,426,110,32);balance.setBounds(338,426,275,32);
 }
 void paint(juce::Graphics& g)override{
  g.fillAll(juce::Colour(0xff1c1f24));
  g.setColour(juce::Colour(0xffe9e5db));g.setFont(26.f);g.drawText("COMPRESSEUR LEGENDS",20,12,620,30,juce::Justification::centredLeft);
  g.setColour(juce::Colour(0xffaab2bc));g.setFont(12.f);g.drawText("LIBERTY MUSIC SOFTWARE   /   TITANIUM RED",22,40,620,18,juce::Justification::centredLeft);
  g.setColour(juce::Colour(0xff282d34));g.fillRoundedRectangle(12.f,140.f,916.f,268.f,8.f);
  const auto m=juce::jlimit(0,7,model.getSelectedId()-1);
  g.setColour(color(m));g.fillRoundedRectangle(24.f,193.f,892.f,4.f,2.f);
  const float gr=juce::jlimit(0.f,30.f,p.meters[selected].load(std::memory_order_relaxed));
  g.setColour(juce::Colour(0xffe9e5db));g.setFont(13.f);g.drawText(u(french?"Réduction de gain":"Gain reduction"),440,151,180,34,juce::Justification::centredLeft);
  g.setColour(juce::Colour(0xff13171b));g.fillRoundedRectangle(24.f,215.f,768.f,22.f,4.f);
  g.setColour(juce::Colour(0xffd6465c));g.fillRoundedRectangle(24.f,215.f,768.f*gr/30.f,22.f,4.f);
  g.setColour(juce::Colours::white);g.drawText(juce::String(gr,1)+" dB",804,214,110,24,juce::Justification::centred);
  g.setColour(juce::Colour(0xffaab2bc));g.drawText("v0.1.1  /  PROTOTYPE",690,426,228,32,juce::Justification::centredRight);
 }
};
