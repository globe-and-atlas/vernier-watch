#include <pebble.h>
#include "config.h"
#include "model.h"
typedef struct {int palette,bg,text,hour,minute,time,date,labels,digital;} Settings;
static Settings s={0,0,0xFFFFFF,0x55FFFF,0xFFAA55,0,0,1,1};
static const uint32_t PALETTES[6][4]={
 {0x000000,0xFFFFFF,0xFFAA55,0xFFFFAA},
 {0xFFFFAA,0x000000,0x0000AA,0xAA0000},
 {0x000000,0xFFFFFF,0xAAFFAA,0xFFFF55},
 {0x000055,0xFFFFFF,0xFFFFFF,0x55FFFF},
 {0x000000,0xFFFFFF,0xFF55AA,0xAAAAFF},
 {0x000000,0xFFFFFF,0xFFFFFF,0xAAAAAA}
};
static Window *window;static Layer *face;static int width,height;static struct tm current;
#if VER_FIXTURE
static int fh=VER_HOUR,fm=VER_MINUTE,fs=VER_SECOND;static bool proof=false;
#endif
static uint32_t color(int i){if(s.palette<6)return PALETTES[s.palette][i];return i==0?s.bg:i==1?s.text:i==2?s.hour:s.minute;}
/* Original small bitmap glyphs: predictable native bounds on both displays. */
static const uint8_t NUMBERS[10][7]={
 {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},
 {30,1,1,14,1,1,30},{2,6,10,18,31,2,2},{31,16,16,30,1,1,30},
 {14,16,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},{14,17,17,15,1,1,14}
};
static uint8_t bits(char c,int row){
 if(c>='0'&&c<='9')return NUMBERS[c-'0'][row];
 static const char CHARS[]="ABCDEHILMNOPRSTUVW/-:";
 static const uint8_t OTHER[][7]={{14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{15,16,16,16,16,16,15},{30,17,17,17,17,17,30},{31,16,16,30,16,16,31},{17,17,17,31,17,17,17},{31,4,4,4,4,4,31},{16,16,16,16,16,16,31},{17,27,21,21,17,17,17},{17,25,25,21,19,19,17},{14,17,17,17,17,17,14},{30,17,17,30,16,16,16},{30,17,17,30,20,18,17},{15,16,16,14,1,1,30},{31,4,4,4,4,4,4},{17,17,17,17,17,17,14},{17,17,17,17,17,10,4},{17,17,17,21,21,21,10},{1,2,2,4,8,8,16},{0,0,0,31,0,0,0},{0,1,1,0,1,1,0}};
 const char *at=strchr(CHARS,c);return at?OTHER[at-CHARS][row]:0;
}
static int char_width(char c){return c==':'?1:c==' '?2:5;}
static int text_width(const char *text){int n=0;while(*text)n+=char_width(*text++)+1;return n?n-1:0;}
static void text(GContext *ctx,const char *value,int x,int y,int w,int h,uint32_t ink){
 int source=text_width(value),offset=0;
 if(!source)return;
 graphics_context_set_fill_color(ctx,GColorFromHEX(ink));
 for(int i=0;value[i];i++){
  int columns=char_width(value[i]);
  for(int row=0;row<7;row++)for(int col=0;col<columns;col++)if(bits(value[i],row)&(1<<(columns-1-col))){
   int left=x+(offset+col)*w/source,right=x+(offset+col+1)*w/source;
   int top=y+row*h/7,bottom=y+(row+1)*h/7;
   graphics_fill_rect(ctx,GRect(left,top,right-left,bottom-top),0,GCornerNone);
  }
  offset+=columns+1;
 }
}
static void line(GContext *ctx,TPoint a,TPoint b,int thickness,uint32_t ink){
 graphics_context_set_stroke_color(ctx,GColorFromHEX(ink));graphics_context_set_stroke_width(ctx,thickness);graphics_draw_line(ctx,GPoint(a.x,a.y),GPoint(b.x,b.y));
}
static void ring(GContext *ctx,TPoint p,int r,int thickness,uint32_t ink){
 graphics_context_set_stroke_color(ctx,GColorFromHEX(ink));graphics_context_set_stroke_width(ctx,thickness);graphics_draw_circle(ctx,GPoint(p.x,p.y),r);
}
static void centered(GContext *ctx,const char *value,TPoint p,int w,int h,uint32_t ink){text(ctx,value,p.x-w/2,p.y-h/2,w,h,ink);}
static uint32_t dim(uint32_t ink){uint32_t result=0;for(int shift=0;shift<24;shift+=8){int v=(((color(0)>>shift)&255)*2+((ink>>shift)&255))/3;result|=(uint32_t)(((v+42)/85)*85)<<shift;}return result;}
static void fill_circle(GContext *ctx,TPoint p,int radius,uint32_t ink){graphics_context_set_fill_color(ctx,GColorFromHEX(ink));graphics_fill_circle(ctx,GPoint(p.x,p.y),radius);}

static void index_tip(GContext *ctx,int x,int y,uint32_t ink){for(int row=0;row<5;row++)line(ctx,(TPoint){x-4+row,y+row},(TPoint){x+4-row,y+row},1,ink);}
static void draw(Layer *layer,GContext *ctx){
 graphics_context_set_antialiased(ctx,false);graphics_context_set_fill_color(ctx,GColorFromHEX(color(0)));graphics_fill_rect(ctx,layer_get_bounds(layer),0,GCornerNone);
 TLayout l;t_layout(width,height,current.tm_hour,current.tm_min,current.tm_sec,s.time,clock_is_24h_style(),&l);
 ring(ctx,l.center,l.main_r,1,dim(color(3)));ring(ctx,l.center,l.hour_r,1,dim(color(2)));
 for(int n=0;n<60;n++)line(ctx,l.main_tick[n][0],l.main_tick[n][1],n%5?1:2,n%5?dim(color(3)):color(3));
 for(int n=0;n<12;n++){char value[3];snprintf(value,sizeof(value),"%02d",n*5);centered(ctx,value,l.main_label[n],11,7,color(1));}
 for(int n=0;n<l.hour_count;n++){
  line(ctx,l.hour_tick[n][0],l.hour_tick[n][1],1,color(2));
  if(l.hour_count==24 && n%2)continue;
  int value=(current.tm_hour+n)%l.hour_count;if(l.hour_count==12 && !value)value=12;char number[3];snprintf(number,sizeof(number),"%d",value);centered(ctx,number,l.hour_label[n],value>9?11:5,7,n?dim(color(1)):color(1));
 }
 index_tip(ctx,l.center.x,l.center.y-l.main_r-8,color(3));index_tip(ctx,l.center.x,l.center.y-l.hour_r-8,color(2));
 if(s.labels){
  for(int n=0;n<=10;n++)line(ctx,l.aux[n][0],l.aux[n][1],n==l.alignment?3:1,n==l.alignment?color(1):color(2));
  centered(ctx,"30",l.aux_label[0],11,7,color(2));centered(ctx,"60",l.aux_label[1],11,7,color(2));
  centered(ctx,"6S SCALE",(TPoint){width/2,height-15},47,7,color(2));
 }
 char date[16];t_date(current.tm_year+1900,current.tm_mon+1,current.tm_mday,s.date,date,sizeof(date));const char *heading=s.date?date:"VERNIER";text(ctx,heading,(width-text_width(heading))/2,width==200?10:7,text_width(heading),7,color(1));
 if(s.digital){bool day=t_24(s.time,clock_is_24h_style());char time_value[12],seconds[8];snprintf(time_value,sizeof(time_value),"%02d:%02d",l.hour_value,current.tm_min);centered(ctx,time_value,(TPoint){l.center.x,l.center.y-4},57,12,color(1));if(!day)centered(ctx,current.tm_hour>=12?"PM":"AM",(TPoint){l.center.x,l.center.y+10},11,7,color(2));snprintf(seconds,sizeof(seconds),"%02d",current.tm_sec);centered(ctx,seconds,(TPoint){l.center.x,l.center.y+24},17,10,color(3));}
#if VER_FIXTURE
 if(proof){graphics_context_set_fill_color(ctx,GColorMagenta);graphics_fill_rect(ctx,GRect(width/2-4,19,2,2),0,GCornerNone);graphics_context_set_fill_color(ctx,t_bounds(width,height,&l)?GColorGreen:GColorRed);graphics_fill_rect(ctx,GRect(width/2+2,19,2,2),0,GCornerNone);}
#endif
}

static void tick(struct tm *now,TimeUnits changed){
 (void)changed;current=*now;
#if VER_FIXTURE
 current.tm_hour=fh;current.tm_min=fm;current.tm_sec=fs;current.tm_year=126;current.tm_mon=10;current.tm_mday=8;
#endif
 layer_mark_dirty(face);
}
static void setting(DictionaryIterator *iter,int message,int key,int maximum,int *value){Tuple *t=dict_find(iter,message);if(!t||(t->type!=TUPLE_INT && t->type!=TUPLE_UINT))return;int v=t->value->int32;if(v<0||v>maximum)return;*value=v;persist_write_int(key,v);}
static void inbox(DictionaryIterator *iter,void *context){
 (void)context;setting(iter,MESSAGE_KEY_Palette,1,6,&s.palette);setting(iter,MESSAGE_KEY_Background,2,0xFFFFFF,&s.bg);setting(iter,MESSAGE_KEY_TextColor,3,0xFFFFFF,&s.text);setting(iter,MESSAGE_KEY_HourColor,4,0xFFFFFF,&s.hour);setting(iter,MESSAGE_KEY_MinuteColor,5,0xFFFFFF,&s.minute);setting(iter,MESSAGE_KEY_TimeFormat,6,2,&s.time);setting(iter,MESSAGE_KEY_DateFormat,7,3,&s.date);setting(iter,MESSAGE_KEY_Labels,8,1,&s.labels);setting(iter,MESSAGE_KEY_DigitalTime,9,1,&s.digital);
#if VER_FIXTURE
 int *fields[]={&fh,&fm,&fs};int max[]={23,59,59};for(int i=0;i<3;i++){Tuple *t=dict_find(iter,11000+i);if(t && t->type==TUPLE_INT && t->value->int32>=0 && t->value->int32<=max[i])*fields[i]=t->value->int32;}Tuple *p=dict_find(iter,11022);if(p && p->type==TUPLE_INT)proof=p->value->int32==1;
#endif
 window_set_background_color(window,GColorFromHEX(color(0)));time_t now=time(NULL);tick(localtime(&now),SECOND_UNIT);
}
static void restore(int key,int max,int *v){if(persist_exists(key)){int n=persist_read_int(key);if(n>=0 && n<=max)*v=n;}}
static void init(void){
 restore(1,6,&s.palette);restore(2,0xFFFFFF,&s.bg);restore(3,0xFFFFFF,&s.text);restore(4,0xFFFFFF,&s.hour);restore(5,0xFFFFFF,&s.minute);restore(6,2,&s.time);restore(7,3,&s.date);restore(8,1,&s.labels);restore(9,1,&s.digital);
 window=window_create();window_set_background_color(window,GColorFromHEX(color(0)));Layer *root=window_get_root_layer(window);GRect b=layer_get_bounds(root);width=b.size.w;height=b.size.h;face=layer_create(b);layer_set_update_proc(face,draw);layer_add_child(root,face);window_stack_push(window,true);
 time_t now=time(NULL);tick(localtime(&now),SECOND_UNIT);tick_timer_service_subscribe(SECOND_UNIT,tick);app_message_register_inbox_received(inbox);app_message_open(256,64);
}
static void deinit(void){tick_timer_service_unsubscribe();app_message_deregister_callbacks();layer_destroy(face);window_destroy(window);}
int main(void){init();app_event_loop();deinit();}
