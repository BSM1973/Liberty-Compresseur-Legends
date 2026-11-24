#pragma once
#include <array>
#include <algorithm>
#include <cmath>
namespace liberty {
enum class Circuit{FET,OPTO,VCA,TUBE,DIODE,PWM,DIGITAL,BAND,SPECTRAL};
struct CharacterProfile{Circuit circuit;float attack,release,knee,drive,even;};
inline constexpr std::array<CharacterProfile,60> characterProfiles{{
 {Circuit::FET,0.72f,0.85f,1.1f,1.15f,0.05f},
 {Circuit::OPTO,1.14f,1.4f,1.22f,0.75f,0.16f},
 {Circuit::VCA,1.02f,1.15f,1.15f,0.42f,0.03f},
 {Circuit::VCA,0.8f,0.83f,0.84f,0.7f,0.02f},
 {Circuit::TUBE,1.25f,1.35f,1.25f,1.05f,0.26f},
 {Circuit::DIODE,0.9f,1.16f,0.94f,1.22f,0.16f},
 {Circuit::OPTO,0.94f,1.1f,1.12f,0.38f,0.07f},
 {Circuit::DIGITAL,1.0f,1.0f,0.95f,0.0f,0.0f},
 {Circuit::FET,0.52f,0.68f,0.75f,1.25f,0.02f},
 {Circuit::FET,0.86f,1.12f,1.2f,1.4f,0.1f},
 {Circuit::OPTO,1.32f,1.48f,1.28f,0.65f,0.18f},
 {Circuit::OPTO,0.78f,0.92f,1.0f,0.85f,0.09f},
 {Circuit::VCA,1.1f,1.18f,1.18f,0.25f,0.01f},
 {Circuit::VCA,0.66f,0.73f,0.7f,0.66f,0.025f},
 {Circuit::TUBE,1.28f,1.46f,1.35f,0.8f,0.22f},
 {Circuit::TUBE,0.92f,1.22f,1.18f,1.5f,0.3f},
 {Circuit::DIODE,0.72f,0.88f,0.8f,1.55f,0.19f},
 {Circuit::DIODE,1.12f,1.28f,1.27f,0.91f,0.12f},
 {Circuit::DIGITAL,0.64f,0.85f,0.8f,0.0f,0.0f},
 {Circuit::DIGITAL,1.18f,1.16f,1.3f,0.0f,0.0f},
 {Circuit::FET,0.55f,0.78f,0.8f,1.6f,0.035f},
 {Circuit::FET,1.12f,1.24f,1.35f,1.05f,0.12f},
 {Circuit::OPTO,1.4f,1.6f,1.42f,0.68f,0.2f},
 {Circuit::OPTO,0.65f,0.82f,0.85f,0.95f,0.08f},
 {Circuit::VCA,0.84f,1.22f,1.15f,0.45f,0.025f},
 {Circuit::VCA,0.96f,1.08f,1.08f,0.32f,0.015f},
 {Circuit::TUBE,1.24f,1.32f,1.35f,0.72f,0.24f},
 {Circuit::TUBE,0.97f,1.26f,1.18f,1.35f,0.29f},
 {Circuit::DIODE,0.65f,0.78f,0.76f,1.65f,0.18f},
 {Circuit::DIODE,1.08f,1.23f,1.28f,0.78f,0.09f},
 {Circuit::DIGITAL,0.9f,1.36f,1.15f,0.0f,0.0f},
 {Circuit::DIGITAL,0.66f,0.83f,0.83f,0.0f,0.0f},
 {Circuit::DIGITAL,0.94f,1.12f,1.14f,0.0f,0.0f},
 {Circuit::DIGITAL,0.67f,0.74f,0.88f,0.0f,0.0f},
 {Circuit::BAND,1.08f,1.14f,1.15f,0.0f,0.0f},
 {Circuit::BAND,1.24f,1.26f,1.26f,0.0f,0.0f},
 {Circuit::BAND,0.72f,0.82f,0.82f,0.0f,0.0f},
 {Circuit::BAND,1.38f,1.4f,1.34f,0.0f,0.0f},
 {Circuit::DIGITAL,0.77f,1.24f,1.1f,0.0f,0.0f},
 {Circuit::DIGITAL,0.55f,0.72f,0.73f,0.0f,0.0f},
 {Circuit::FET,0.9f,1.27f,1.25f,1.65f,0.11f},
 {Circuit::FET,0.76f,0.95f,1.03f,0.92f,0.045f},
 {Circuit::OPTO,1.34f,1.36f,1.35f,0.35f,0.11f},
 {Circuit::OPTO,1.2f,1.46f,1.23f,1.35f,0.23f},
 {Circuit::VCA,0.62f,0.76f,0.76f,0.6f,0.015f},
 {Circuit::VCA,1.02f,1.25f,1.18f,0.85f,0.04f},
 {Circuit::TUBE,1.35f,1.46f,1.46f,1.15f,0.34f},
 {Circuit::TUBE,0.86f,1.1f,1.05f,1.1f,0.21f},
 {Circuit::DIODE,0.8f,1.15f,1.02f,1.8f,0.22f},
 {Circuit::DIODE,1.35f,1.44f,1.42f,0.78f,0.13f},
 {Circuit::PWM,0.67f,0.81f,0.86f,0.85f,0.015f},
 {Circuit::PWM,0.96f,1.15f,1.17f,0.52f,0.01f},
 {Circuit::BAND,0.97f,1.14f,1.18f,0.0f,0.0f},
 {Circuit::BAND,1.31f,1.37f,1.35f,0.0f,0.0f},
 {Circuit::BAND,0.79f,0.9f,0.94f,0.0f,0.0f},
 {Circuit::BAND,1.42f,1.48f,1.38f,0.0f,0.0f},
 {Circuit::SPECTRAL,1.1f,1.2f,1.18f,0.0f,0.0f},
 {Circuit::SPECTRAL,0.75f,0.9f,0.88f,0.0f,0.0f},
 {Circuit::SPECTRAL,1.32f,1.38f,1.32f,0.0f,0.0f},
 {Circuit::SPECTRAL,0.6f,0.74f,0.8f,0.0f,0.0f}
}};
inline const CharacterProfile& characterProfile(int model){return characterProfiles[size_t(std::clamp(model,0,59))];}
inline bool analogCircuit(Circuit c){return c==Circuit::FET||c==Circuit::OPTO||c==Circuit::VCA||c==Circuit::TUBE||c==Circuit::DIODE||c==Circuit::PWM;}
inline float blend(float a,float b,float amount){return a+(b-a)*amount;}
// Bounded waveshapers: odd saturation + controlled even-order asymmetry.
// Unity derivative near zero. PWM is a duty-inspired averaged transfer, not a carrier simulation.
inline float characterShape(float x,const CharacterProfile& c,float amount,float color){
 float mix=std::clamp(amount,0.f,1.f)*std::clamp(color,0.f,1.f);
 if(mix<=0||!analogCircuit(c.circuit))return x;
 const float d=mix*c.drive;
 const float k=1.f+d*(c.circuit==Circuit::VCA?.9f:c.circuit==Circuit::TUBE?2.1f:2.8f);
 const float z=k*x;
 float s=0;
 switch(c.circuit){
 case Circuit::FET:s=std::tanh(z);break;
 case Circuit::OPTO:s=z/std::sqrt(1.f+z*z);break;
 case Circuit::VCA:s=z/(1.f+.25f*std::abs(z));break;
 case Circuit::TUBE:s=std::tanh(z*.80f)/.80f;break;
 case Circuit::DIODE:s=std::atan(z);break;
 case Circuit::PWM:s=z/std::sqrt(1.f+.45f*z*z);break;
 default:return x;
 }
 // Even component has zero derivative at zero and stays bounded.
 const float even=(z*z)/(1.f+z*z);
 return blend(x,(s+d*c.even*even)/k,mix);
}
struct CharacterDC{
 float lastIn=0,lastOut=0;
 void reset(){lastIn=lastOut=0;}
 float process(float x,float pole){float y=x-lastIn+pole*lastOut;lastIn=x;lastOut=y;return y;}
};
}
