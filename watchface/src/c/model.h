#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef struct {int16_t x,y;} TPoint;
typedef struct {TPoint center,main_tick[60][2],main_label[12],hour_tick[24][2],hour_label[24],aux[11][2],aux_label[2];int main_r,hour_r,aux_r,rotation,hour_count,hour_value,alignment;} TLayout;
int t_sin(int a);TPoint t_point(TPoint p,int r,int a);bool t_24(int mode,bool system24);int t_hour(int hour,int mode,bool system24);void t_date(int y,int m,int d,int mode,char *out,int n);
void t_layout(int w,int h,int hour,int minute,int second,int mode,bool system24,TLayout *out);
bool t_bounds(int w,int h,const TLayout *l);
