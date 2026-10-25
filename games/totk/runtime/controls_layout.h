/* One controls.txt for the existing controller/mouse layout and Mac keyboard.
   Pure C so the launcher validates exactly what the editor and runtime read. */
#pragma once
#include "controller_map.h"
#include <stdio.h>
#include <stdlib.h>
enum { CONTROLS_KEYS=128, CONTROLS_TARGETS=24, CONTROLS_GAME_BUTTONS=16, CONTROLS_GAME_NONE=0xff };
/* Options (the Mac control menu's input pages; "name = value" lines, absent = the default):
     swap_sticks, invert_look_y = on|off   controllers only: the two sticks swapped, the look stick's Y inverted
     mouse_drag_look = on|off              hold the right mouse button and drag to look, without mouse mode (key 7)
     mouse_sensitivity = 1..100            mouse look (mouse_look.h); absent: TOTK_MOUSE_SENSITIVITY / 50
     mouse_invert_y = on|off               absent: TOTK_MOUSE_INVERT_Y / off
     game_<button> = <button>|none         the last layer: what the game sees for each of its 16 buttons, after the
                                           keyboard and controller layouts (menus follow too); absent: itself */
typedef struct {
    uint16_t map[CONTROLLER_BUTTONS];
    uint8_t modes[CONTROLLER_BUTTONS], keys[CONTROLS_KEYS]; /* 0 none, 1..16 FullKey, 17..24 stick directions */
    uint8_t game[CONTROLS_GAME_BUTTONS];                    /* FullKey bit i reaches the game as bit game[i]; CONTROLS_GAME_NONE drops it */
    uint8_t swap_sticks, invert_look_y, mouse_drag_look;    /* 0 / 1 */
    uint8_t mouse_sensitivity, mouse_invert_y;              /* 0 = the environment / default; invert 1 off, 2 on */
} ControlsLayout;
static inline uint64_t controls_game_apply(const uint8_t game[CONTROLS_GAME_BUTTONS],uint64_t buttons) {
    uint64_t out=buttons&~0xffffULL;   /* bits 16+ (stick directions) pass unchanged */
    for(unsigned i=0;i<CONTROLS_GAME_BUTTONS;i++)if(buttons>>i&1&&game[i]<CONTROLS_GAME_BUTTONS)out|=1ULL<<game[i];
    return out;
}
/* 1 = an option line (applied to *b), 0 = not one, -1 = a bad value (*error says why). */
static inline int controls_option_parse(const char *row,ControlsLayout *b,const char **error) {
    const char *p=row;while(*p==' '||*p=='\t')p++;
    const char *name=p;while((*p>='a'&&*p<='z')||(*p>='A'&&*p<='Z')||(*p>='0'&&*p<='9')||*p=='_')p++;
    size_t nl=(size_t)(p-name);while(*p==' '||*p=='\t')p++;
    if(!nl||*p!='=')return 0;
    p++;while(*p==' '||*p=='\t')p++;
    const char *value=p;while(*p&&*p!=' '&&*p!='\t'&&*p!='\r'&&*p!='#')p++;
    size_t vl=(size_t)(p-value);while(*p==' '||*p=='\t'||*p=='\r')p++;
#define CONTROLS_IS(s) (nl==strlen(s)&&!strncasecmp(name,s,nl))
#define CONTROLS_VALUE(s) (vl==strlen(s)&&!strncasecmp(value,s,vl))
    int from=nl>5&&!strncasecmp(name,"game_",5)?controller_name_index(controller_switch_names,CONTROLS_GAME_BUTTONS,name+5,nl-5):-1;
    int flag=CONTROLS_IS("swap_sticks")?1:CONTROLS_IS("invert_look_y")?2:CONTROLS_IS("mouse_drag_look")?3:CONTROLS_IS("mouse_invert_y")?4:CONTROLS_IS("mouse_sensitivity")?5:from>=0?6:0;
    if(!flag)return 0;
    if(*p&&*p!='#'){*error="options take one value";return -1;}
    int on=CONTROLS_VALUE("on")?1:CONTROLS_VALUE("off")?0:-1;
    if(flag<=4&&on<0){*error="expected on or off";return -1;}
    if(flag==1)b->swap_sticks=(uint8_t)on;else if(flag==2)b->invert_look_y=(uint8_t)on;else if(flag==3)b->mouse_drag_look=(uint8_t)on;else if(flag==4)b->mouse_invert_y=(uint8_t)(on?2:1);
    else if(flag==5){long v=0;for(size_t i=0;i<vl;i++){if(value[i]<'0'||value[i]>'9'||v>100){v=-1;break;}v=v*10+(value[i]-'0');}
        if(!vl||v<1||v>100){*error="mouse_sensitivity takes 1..100";return -1;}b->mouse_sensitivity=(uint8_t)v;}
    else{int to=CONTROLS_VALUE("none")?CONTROLS_GAME_NONE:controller_name_index(controller_switch_names,CONTROLS_GAME_BUTTONS,value,vl);
        if(to<0){*error="unknown game button";return -1;}b->game[from]=(uint8_t)to;}
#undef CONTROLS_IS
#undef CONTROLS_VALUE
    return 1;
}
static const char *const controls_axis_names[]={"move_left","move_right","move_up","move_down","look_left","look_right","look_up","look_down"};
static inline const char *controls_target_name(unsigned target) {
    return !target ? "none" : target<=16 ? controller_switch_names[target-1] : target<=24 ? controls_axis_names[target-17] : NULL;
}
static inline int controls_key_reserved(unsigned k) {
    return k==18||k==19||k==20||k==21||k==22||k==23||k==26||k==54||k==55; /* 1..7, Command */
}
static inline void controls_layout_default(ControlsLayout *b) {
    controller_map_default(b->map,b->modes); memset(b->keys,0,sizeof b->keys);
    static const unsigned key[CONTROLS_TARGETS]={38,40,34,32,6,8,12,14,58,56,36,48,3,17,4,5,0,2,13,1,123,124,126,125};
    for(unsigned i=0;i<CONTROLS_TARGETS;i++)b->keys[key[i]]=(uint8_t)(i+1);
    b->keys[49]=2;b->keys[53]=12;
    for(unsigned i=0;i<CONTROLS_GAME_BUTTONS;i++)b->game[i]=(uint8_t)i;
    b->swap_sticks=b->invert_look_y=b->mouse_drag_look=b->mouse_sensitivity=b->mouse_invert_y=0;
}
static inline int controls_layout_valid(const ControlsLayout *b) {
    for(unsigned i=0;i<CONTROLS_KEYS;i++)if(b->keys[i]>CONTROLS_TARGETS||(b->keys[i]&&controls_key_reserved(i)))return 0;
    for(unsigned i=0;i<CONTROLS_GAME_BUTTONS;i++)if(b->game[i]>=CONTROLS_GAME_BUTTONS&&b->game[i]!=CONTROLS_GAME_NONE)return 0;
    if(b->swap_sticks>1||b->invert_look_y>1||b->mouse_drag_look>1||b->mouse_sensitivity>100||b->mouse_invert_y>2)return 0;
    for(unsigned i=0;i<CONTROLLER_BUTTONS;i++){
        unsigned m=b->map[i],kind=controller_mode_kind(b->modes[i]);
        if((m&&(m&(m-1)))||kind>CONTROLLER_AIM||(!m&&b->modes[i]))return 0;
        if(kind==CONTROLLER_AIM){unsigned fire=controller_mode_fire(b->modes[i]);if(fire>=CONTROLLER_BUTTONS||fire==i)return 0;}
        else if(controller_mode_fire(b->modes[i]))return 0;
    }
    return 1;
}
/* Transactional: on error neither controller nor keyboard entries change. */
static inline int controls_layout_parse(const char *text,ControlsLayout *b,const char **error) {
    ControlsLayout next=*b;int line=0;
    for(const char *p=text;p&&*p;){
        line++;const char *end=strchr(p,'\n');size_t n=end?(size_t)(end-p):strlen(p);
        char row[16384];if(n>=sizeof row){*error="line too long";return line;}
        memcpy(row,p,n);row[n]=0;char *start=row;while(*start==' '||*start=='\t')start++;
        if(!strncasecmp(start,"key_",4)){
            char *number=start+4,*tail;unsigned long key=strtoul(number,&tail,10);
            if(number==tail||*number<'0'||*number>'9'||key>=CONTROLS_KEYS){*error="expected key_0 through key_127";return line;}
            while(*tail==' '||*tail=='\t')tail++;
            if(*tail++!='='){*error="expected key_<Mac keycode> = target";return line;}
            while(*tail==' '||*tail=='\t')tail++;
            char *stop=tail;while(*stop&&*stop!=' '&&*stop!='\t'&&*stop!='\r'&&*stop!='#')stop++;
            int target=-1;for(unsigned i=0;i<=CONTROLS_TARGETS;i++)if(strlen(controls_target_name(i))==(size_t)(stop-tail)&&!strncasecmp(controls_target_name(i),tail,(size_t)(stop-tail)))target=(int)i;
            if(target<0){*error="unknown keyboard target";return line;}
            while(*stop==' '||*stop=='\t'||*stop=='\r')stop++;
            if(*stop&&*stop!='#'){*error="keyboard bindings take no mode";return line;}
            if(target&&controls_key_reserved((unsigned)key)){*error="key reserved for application shortcuts";return line;}
            next.keys[key]=(uint8_t)target;
        }else{int option=controls_option_parse(start,&next,error);if(option<0)return line;
            if(!option&&controller_map_parse(row,next.map,next.modes,error))return line;}
        p=end?end+1:NULL;
    }
    *b=next;return 0;
}
/* Canonical file, accepted by the same parser. Includes explicit cleared keys. */
static inline int controls_layout_format(const ControlsLayout *b,char *out,size_t cap) {
    if(!controls_layout_valid(b)||!cap)return 0;
    size_t used=0;
    for(unsigned i=0;i<CONTROLLER_BUTTONS;i++){
        unsigned target=0;while(target<16&&b->map[i]!=(1u<<target))target++;
        static const char *const mode[]={"hold","tap","toggle","aim"};unsigned kind=controller_mode_kind(b->modes[i]);
        int n=snprintf(out+used,cap-used,"%s = %s %s%s%s\n",controller_physical_names[i],target<16?controller_switch_names[target]:"none",mode[kind],kind==CONTROLLER_AIM?" ":"",kind==CONTROLLER_AIM?controller_physical_names[controller_mode_fire(b->modes[i])]:"");
        if(n<0||(size_t)n>=cap-used)return 0;used+=(size_t)n;
    }
    for(unsigned i=0;i<CONTROLS_KEYS;i++){
        int n=snprintf(out+used,cap-used,"key_%u = %s\n",i,controls_target_name(b->keys[i]));
        if(n<0||(size_t)n>=cap-used)return 0;used+=(size_t)n;
    }
    /* Options only where they differ from the default, so a file without them reads as before. */
    for(unsigned i=0;i<CONTROLS_GAME_BUTTONS;i++)if(b->game[i]!=i){
        int n=snprintf(out+used,cap-used,"game_%s = %s\n",controller_switch_names[i],b->game[i]<CONTROLS_GAME_BUTTONS?controller_switch_names[b->game[i]]:"none");
        if(n<0||(size_t)n>=cap-used)return 0;used+=(size_t)n;
    }
    const struct{const char *name;int set,on;} options[]={{"swap_sticks",b->swap_sticks,b->swap_sticks},{"invert_look_y",b->invert_look_y,b->invert_look_y},
        {"mouse_drag_look",b->mouse_drag_look,b->mouse_drag_look},{"mouse_invert_y",b->mouse_invert_y!=0,b->mouse_invert_y==2}};
    for(unsigned i=0;i<sizeof options/sizeof *options;i++)if(options[i].set){
        int n=snprintf(out+used,cap-used,"%s = %s\n",options[i].name,options[i].on?"on":"off");
        if(n<0||(size_t)n>=cap-used)return 0;used+=(size_t)n;
    }
    if(b->mouse_sensitivity){int n=snprintf(out+used,cap-used,"mouse_sensitivity = %u\n",b->mouse_sensitivity);if(n<0||(size_t)n>=cap-used)return 0;used+=(size_t)n;}
    return (int)used;
}
