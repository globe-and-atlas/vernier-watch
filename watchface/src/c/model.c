#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
static const int16_t SINE[91]={0,286,572,857,1143,1428,1713,1997,2280,2563,2845,3126,3406,3686,3964,4240,4516,4790,5063,5334,5604,5872,6138,6402,6664,6924,7182,7438,7692,7943,8192,8438,8682,8923,9162,9397,9630,9860,10087,10311,10531,10749,10963,11174,11381,11585,11786,11982,12176,12365,12551,12733,12911,13085,13255,13421,13583,13741,13894,14044,14189,14330,14466,14598,14726,14849,14968,15082,15191,15296,15396,15491,15582,15668,15749,15826,15897,15964,16026,16083,16135,16182,16225,16262,16294,16322,16344,16362,16374,16382,16384};
int t_sin(int angle){
 angle%=3600;if(angle<0)angle+=3600;
 int sign=1;if(angle>=1800){sign=-1;angle-=1800;}if(angle>900)angle=1800-angle;
 int degree=angle/10,part=angle%10;
 return sign*(SINE[degree]+(degree<90?(SINE[degree+1]-SINE[degree])*part/10:0));
}
static int rounded(int value){return value>=0?(value+8192)/16384:-((-value+8192)/16384);}
TPoint t_point(TPoint p,int radius,int angle){TPoint q={p.x+rounded(radius*t_sin(angle)),p.y-rounded(radius*t_sin(angle+900))};return q;}
bool t_24(int mode,bool system24){return mode==2 || (mode==0 && system24);}
int t_hour(int hour,int mode,bool system24){return t_24(mode,system24)?hour:(hour%12?hour%12:12);}
void t_date(int year,int month,int day,int mode,char *out,int n){
 if(mode==1)snprintf(out,n,"%04d-%02d-%02d",year,month,day);
 else if(mode==2)snprintf(out,n,"%02d/%02d/%04d",month,day,year);
 else if(mode==3)snprintf(out,n,"%02d/%02d/%04d",day,month,year);
 else if(n>0)out[0]=0;
}
void t_layout(int w,int h,int hour,int minute,int second,int mode,bool system24,TLayout *l){
 (void)h;memset(l,0,sizeof(*l));bool r=w==260;l->center=(TPoint){w/2,r?133:120};l->main_r=r?96:78;l->hour_r=r?63:51;l->aux_r=r?109:90;
 l->rotation=-(minute*60+second);l->hour_count=t_24(mode,system24)?24:12;l->hour_value=t_hour(hour,mode,system24);l->alignment=(second+3)/6;
 for(int n=0;n<60;n++){int length=n%5?3:7;int a=n*60+l->rotation;l->main_tick[n][0]=t_point(l->center,l->main_r-length,a);l->main_tick[n][1]=t_point(l->center,l->main_r,a);}
 for(int n=0;n<12;n++)l->main_label[n]=t_point(l->center,r?85:67,n*300+l->rotation);
 for(int n=0;n<l->hour_count;n++){int a=n*3600/l->hour_count;l->hour_tick[n][0]=t_point(l->center,l->hour_r-4,a);l->hour_tick[n][1]=t_point(l->center,l->hour_r,a);l->hour_label[n]=t_point(l->center,r?51:42,a);}
 for(int n=0;n<=10;n++){l->aux[n][0]=t_point(l->center,l->aux_r-5,n*54);l->aux[n][1]=t_point(l->center,l->aux_r,n*54);}
 l->aux_label[0]=t_point(l->center,r?120:101,270);l->aux_label[1]=t_point(l->center,r?120:101,540);
}
static bool inside(int w,int h,int x,int y){if(x<0||x>=w||y<0||y>=h)return false;if(w==260){int dx=2*x-259,dy=2*y-259;if(dx*dx+dy*dy>258*258)return false;}return true;}
static bool rect(int w,int h,TPoint c,int x,int y){for(int a=-1;a<=1;a+=2)for(int b=-1;b<=1;b+=2)if(!inside(w,h,c.x+a*x,c.y+b*y))return false;return true;}
bool t_bounds(int w,int h,const TLayout *l){
 for(int n=0;n<60;n++)for(int j=0;j<2;j++)if(!inside(w,h,l->main_tick[n][j].x,l->main_tick[n][j].y))return false;
 for(int n=0;n<12;n++)if(!rect(w,h,l->main_label[n],6,4))return false;
 for(int n=0;n<l->hour_count;n++)if(!rect(w,h,l->hour_label[n],5,4))return false;
 for(int n=0;n<2;n++)if(!rect(w,h,l->aux_label[n],6,4))return false;
 return true;
}
