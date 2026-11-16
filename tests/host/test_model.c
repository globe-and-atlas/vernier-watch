#include "model.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void inside(int w,int h,int x,int y){assert(x>=0&&x<w&&y>=0&&y<h);if(w==260)assert(hypot(x-129.5,y-129.5)<129);}
static void rect(int w,int h,TPoint p,int rx,int ry){for(int x=-rx;x<=rx;x++)for(int y=-ry;y<=ry;y++)inside(w,h,p.x+x,p.y+y);}
static void oracle(TPoint p,TPoint c,int r,double degrees){double a=degrees*acos(-1)/180;assert(abs(p.x-(int)lround(c.x+r*sin(a)))<=1);assert(abs(p.y-(int)lround(c.y-r*cos(a)))<=1);}
int main(void){
 for(int platform=0;platform<2;platform++){
  int w=platform?260:200,h=platform?260:228;
  for(int t=0;t<86400;t++)for(int mode=1;mode<=2;mode++){
   int hh=t/3600,mm=t/60%60,ss=t%60;TLayout l;t_layout(w,h,hh,mm,ss,mode,false,&l);assert(t_bounds(w,h,&l));assert(l.hour_count==(mode==1?12:24));assert(l.hour_value==(mode==2?hh:hh%12?hh%12:12));
   for(int n=0;n<12;n++){oracle(l.main_label[n],l.center,platform?85:67,n*30-mm*6-ss*.1);rect(w,h,l.main_label[n],6,4);}
   for(int n=0;n<l.hour_count;n++)rect(w,h,l.hour_label[n],5,4);
   for(int n=0;n<=10;n++){oracle(l.aux[n][1],l.center,l.aux_r,n*5.4);inside(w,h,l.aux[n][1].x,l.aux[n][1].y);}
   for(int n=0;n<2;n++)rect(w,h,l.aux_label[n],6,4);
   assert(abs(l.alignment*6-ss)<=3);assert(l.alignment>=0&&l.alignment<=10);
   if(ss%6==0){int k=ss/6;TPoint p=l.main_tick[(mm+k)%60][1],q=l.aux[k][1];int ax=p.x-l.center.x,ay=p.y-l.center.y,bx=q.x-l.center.x,by=q.y-l.center.y;assert(abs(ax*by-ay*bx)<2*(l.main_r+l.aux_r));assert(ax*bx+ay*by>0);}
   assert(l.hour_label[0].x==w/2 && l.hour_label[0].y==l.center.y-(platform?51:42));
  }
  printf("PASS Vernier %dx%d:86400seconds,two formats,9:10alignment,bounds,fixed hour index\n",w,h);
 }
 assert(t_hour(0,1,1)==12 && t_hour(0,2,0)==0 && t_hour(13,0,1)==13 && t_hour(13,0,0)==1);
 char d[20];t_date(2026,11,8,0,d,20);assert(!strcmp(d,""));t_date(2026,11,8,1,d,20);assert(!strcmp(d,"2026-11-08"));t_date(2026,11,8,2,d,20);assert(!strcmp(d,"11/08/2026"));t_date(2026,11,8,3,d,20);assert(!strcmp(d,"08/11/2026"));return 0;
}
