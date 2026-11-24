#pragma once
#include <array>
#include <algorithm>
#include <cmath>
namespace liberty {
// 4x interpolating FIR, 32 taps per phase, unity DC gain. Delayed measurement only.
// Independent implementation; not certified against the full BS.1770/EBU suite.
struct TruePeakFIR {
 static constexpr int T=32;
 std::array<std::array<float,T>,4> coeff{};
 std::array<float,T> history{};int pos=0;
 void prepare(){constexpr double pi=3.14159265358979323846;for(int phase=0;phase<4;++phase){double sum=0;for(int k=0;k<T;++k){double x=k-(T/2-1)+phase*.25;double sinc=std::abs(x)<1e-12?1:std::sin(pi*x)/(pi*x);double w=.42+.5*std::cos(pi*x/(T/2))+.08*std::cos(2*pi*x/(T/2));double c=std::abs(x)<T/2?sinc*w:0;coeff[phase][k]=float(c);sum+=c;}for(auto& c:coeff[phase])c=float(c/sum);}reset();}
 void reset(){history.fill(0);pos=0;}
 float process(float x){history[pos]=x;float peak=std::abs(x);for(int p=1;p<4;++p){float sum=0;for(int k=0;k<T;++k)sum+=coeff[p][k]*history[(pos-k+T)%T];peak=std::max(peak,std::abs(sum));}pos=(pos+1)%T;return peak;}
};
}
