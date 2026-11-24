#pragma once
#include <array>
#include <algorithm>
#include <vector>
#include <complex>
#include <cmath>
namespace liberty {
struct ProBand{float threshold=-18,ratio=4,attack=10,release=100;bool solo=false,mute=false,bypass=false;};
struct ProSettings{bool enabled=false,listen=false;int count=3,fft=1024;float link=1,hp=20,lp=20000,bellHz=1000,bellGain=0;std::array<float,5> cross{{120,350,1000,3500,8000}};std::array<ProBand,6> bands{};};
inline int proCount(const ProSettings& p){return std::clamp(p.count,2,6);}
inline int proFFT(int n){return n<=512?512:n<=1024?1024:n<=2048?2048:4096;}
inline float proDb(float x){return 20*std::log10(std::max(x,1e-9f));}
inline float proGain(float x){return std::pow(10.f,x*.05f);}
inline float proReduction(float x,const ProBand& b,float knee){float over=proDb(x)-b.threshold;float slope=1-1/std::max(1.f,b.ratio);if(knee<=0)return std::max(0.f,over)*slope;if(over<=-knee*.5f)return 0;if(over>=knee*.5f)return over*slope;float z=over+knee*.5f;return slope*z*z/(2*knee);}
inline std::array<float,5> proCross(const ProSettings& p,float sr){auto a=p.cross;std::sort(a.begin(),a.begin()+proCount(p)-1);for(auto& x:a)x=std::clamp(x,20.f,std::max(21.f,sr*.45f));return a;}
inline bool audibleBand(const ProSettings& p,int i){bool solo=false;for(int j=0;j<proCount(p);++j)solo|=p.bands[j].solo;return !p.bands[i].mute&&(!solo||p.bands[i].solo);}
struct ProEQ{
 float hpState[2]{},lpState[2]{},z1[2]{},z2[2]{},lastHP=-1,lastLP=-1,lastF=-1,lastG=-99,lastSR=-1,hpc=0,lpc=1,b0=1,b1=0,b2=0,a1=0,a2=0;
 void reset(){for(int i=0;i<2;++i)hpState[i]=lpState[i]=z1[i]=z2[i]=0;lastSR=-1;}
 void prepare(const ProSettings& p,float sr){if(lastHP==p.hp&&lastLP==p.lp&&lastF==p.bellHz&&lastG==p.bellGain&&lastSR==sr)return;
  lastHP=p.hp;lastLP=p.lp;lastF=p.bellHz;lastG=p.bellGain;lastSR=sr;auto hz=[&](float f){return std::clamp(f,10.f,sr*.45f);};hpc=1-std::exp(-6.283185307f*hz(p.hp)/sr);lpc=1-std::exp(-6.283185307f*hz(p.lp)/sr);
  float A=std::pow(10.f,p.bellGain/40.f),w=6.283185307f*hz(p.bellHz)/sr,alpha=std::sin(w)/2.f,cosw=std::cos(w),den=1+alpha/A;b0=(1+alpha*A)/den;b1=-2*cosw/den;b2=(1-alpha*A)/den;a1=b1;a2=(1-alpha/A)/den;
 }
 float process(float x,int ch){hpState[ch]+=hpc*(x-hpState[ch]);if(lastHP>20)x-=hpState[ch];lpState[ch]+=lpc*(x-lpState[ch]);if(lastLP<20000)x=lpState[ch];if(lastG==0)return x;float y=b0*x+z1[ch];z1[ch]=b1*x-a1*y+z2[ch];z2[ch]=b2*x-a2*y;return y;}
};
class ProMulti{
 float lp[4][5][2]{},env[2][6]{},crossC[5]{},at[6]{},rel[6]{},lastA[6]{},lastR[6]{},lastX[5]{},lastSR=-1,gr=0,det=0;std::array<float,6> meters{};std::array<float,5> cachedCross{},rawCross{};int lastCount=-1;
public:
 void reset(){for(auto& ch:lp)for(auto& b:ch)for(auto& z:b)z=0;for(auto& ch:env)for(auto& z:ch)z=0;meters.fill(0);gr=det=0;lastSR=-1;lastCount=-1;}
 float reduction()const{return gr;}float detector()const{return det;}float meter(int i)const{return i>=0&&i<6?meters[i]:0;}
 void process(float& l,float& r,float dl,float dr,const ProSettings& p,float sr,float knee,float makeup,float attackScale,float releaseScale){
  int n=proCount(p);bool change=lastSR!=sr;if(change||lastCount!=n||rawCross!=p.cross){cachedCross=proCross(p,sr);rawCross=p.cross;lastCount=n;}const auto& cross=cachedCross;
  for(int i=0;i<n-1;++i)if(change||lastX[i]!=cross[i]){lastX[i]=cross[i];crossC[i]=1-std::exp(-6.283185307f*cross[i]/sr);}
  for(int i=0;i<n;++i){float a=p.bands[i].attack*attackScale,b=p.bands[i].release*releaseScale;if(change||lastA[i]!=a||lastR[i]!=b){lastA[i]=a;lastR[i]=b;at[i]=std::exp(-1000/(std::max(.05f,a)*sr));rel[i]=std::exp(-1000/(std::max(.05f,b)*sr));}}
  lastSR=sr;float x[4]={l,r,dl,dr},out[4][6]{};for(int c=0;c<4;++c){float prev=0;for(int i=0;i<n-1;++i){auto& z=lp[c][i];z[0]+=crossC[i]*(x[c]-z[0]);z[1]+=crossC[i]*(z[0]-z[1]);out[c][i]=z[1]-prev;prev=z[1];}out[c][n-1]=x[c]-prev;}
  l=r=gr=det=0;meters.fill(0);float link=std::clamp(p.link,0.f,1.f);
  for(int i=0;i<n;++i){float a=std::abs(out[2][i]),b=std::abs(out[3][i]),peak=std::max(a,b);det=std::max(det,peak);float levels[]={a+(peak-a)*link,b+(peak-b)*link};
   for(int c=0;c<2;++c){float t=proReduction(levels[c],p.bands[i],knee),k=t>env[c][i]?at[i]:rel[i];env[c][i]=k*env[c][i]+(1-k)*t;}
   float ga=p.bands[i].bypass?0:env[0][i],gb=p.bands[i].bypass?0:env[1][i];if(!audibleBand(p,i))continue;
   meters[i]=std::max(ga,gb);gr=std::max(gr,meters[i]);l+=out[0][i]*proGain(makeup-ga);r+=out[1][i]*proGain(makeup-gb);
  }
 }
};
class ProSpectral{
 using C=std::complex<float>;static constexpr int MAX=4096;
 std::vector<float> inL,inR,inDL,inDR,outL,outR,window,envL,envR;
 std::vector<C> a,b,c,d,twiddle;int n=512,pos=0,hop=0;float gr=0,det=0;std::array<float,16> meters{};std::array<float,6> bandMeters{};
 void fft(std::vector<C>& x,bool inv){for(int i=1,j=0;i<n;++i){int bit=n>>1;for(;j&bit;bit>>=1)j^=bit;j^=bit;if(i<j)std::swap(x[i],x[j]);}for(int len=2;len<=n;len<<=1){int stride=MAX/len;for(int i=0;i<n;i+=len)for(int j=0;j<len/2;++j){C w=twiddle[j*stride];if(inv)w=std::conj(w);auto u=x[i+j],v=x[i+j+len/2]*w;x[i+j]=u+v;x[i+j+len/2]=u-v;}}if(inv)for(int i=0;i<n;++i)x[i]/=float(n);}
public:
 void prepare(){for(auto* v:{&inL,&inR,&inDL,&inDR,&outL,&outR,&window,&envL,&envR})v->assign(MAX,0);for(auto* v:{&a,&b,&c,&d,&twiddle})v->assign(MAX,C{});for(int i=0;i<MAX;++i){float angle=-6.283185307f*i/MAX;twiddle[i]={std::cos(angle),std::sin(angle)};}setSize(512);reset();}
 void reset(){for(auto* v:{&inL,&inR,&inDL,&inDR,&outL,&outR,&envL,&envR})std::fill(v->begin(),v->end(),0.f);pos=hop=0;gr=det=0;meters.fill(0);bandMeters.fill(0);}
 void setSize(int size){n=proFFT(size);if(window.empty())return;for(int i=0;i<n;++i)window[i]=std::sin(3.141592654f*(i+.5f)/n);reset();}
 int size()const{return n;}float reduction()const{return gr;}float detector()const{return det;}float meter(int i)const{return i>=0&&i<16?meters[i]:0;}float bandMeter(int i)const{return i>=0&&i<6?bandMeters[i]:0;}
 void process(float& l,float& r,float dl,float dr,const ProSettings& p,float sr,float knee,float makeup,float attackScale,float releaseScale,bool separate){
  if(inL.empty())return;int desired=proFFT(p.fft);if(desired!=n)setSize(desired);float ol=outL[pos],orr=outR[pos];outL[pos]=outR[pos]=0;inL[pos]=l;inR[pos]=r;inDL[pos]=dl;inDR[pos]=dr;pos=(pos+1)%n;
  if(++hop==n/2){hop=0;auto cross=proCross(p,sr);int bands=proCount(p);
   for(int i=0;i<n;++i){int j=(pos+i)%n;float w=window[i];a[i]=inL[j]*w;b[i]=inR[j]*w;if(separate){c[i]=inDL[j]*w;d[i]=inDR[j]*w;}}
   fft(a,false);fft(b,false);if(separate){fft(c,false);fft(d,false);}float at[6]{},rel[6]{};
   for(int i=0;i<bands;++i){at[i]=std::exp(-1000.f*(n/2)/(std::max(.05f,p.bands[i].attack*attackScale)*sr));rel[i]=std::exp(-1000.f*(n/2)/(std::max(.05f,p.bands[i].release*releaseScale)*sr));}
   meters.fill(0);bandMeters.fill(0);gr=det=0;int band=0;float link=std::clamp(p.link,0.f,1.f);
   for(int k=0;k<=n/2;++k){float hz=k*sr/n;while(band<bands-1&&hz>=cross[band])++band;float norm=(k==0||k==n/2?1.f:2.f)/(n*.63661977f);
    float ml=std::abs(separate?c[k]:a[k])*norm,mr=std::abs(separate?d[k]:b[k])*norm,peak=std::max(ml,mr);det=std::max(det,peak);
    float tl=proReduction(ml+(peak-ml)*link,p.bands[band],knee),tr=proReduction(mr+(peak-mr)*link,p.bands[band],knee);
    float cl=tl>envL[k]?at[band]:rel[band],cr=tr>envR[k]?at[band]:rel[band];envL[k]=cl*envL[k]+(1-cl)*tl;envR[k]=cr*envR[k]+(1-cr)*tr;
    float gl=p.bands[band].bypass?0:envL[k],gg=p.bands[band].bypass?0:envR[k];bool audible=audibleBand(p,band);
    if(audible){float g=std::max(gl,gg);int group=std::min(15,k*32/n);meters[group]=std::max(meters[group],g);bandMeters[band]=std::max(bandMeters[band],g);gr=std::max(gr,g);}
    float gainL=audible?proGain(makeup-gl):0,gainR=audible?proGain(makeup-gg):0;a[k]*=gainL;b[k]*=gainR;if(k>0&&k<n/2){a[n-k]*=gainL;b[n-k]*=gainR;}}
   fft(a,true);fft(b,true);for(int i=0;i<n;++i){int j=(pos+i)%n;outL[j]+=a[i].real()*window[i];outR[j]+=b[i].real()*window[i];}
  }l=ol;r=orr;
 }
};
}
