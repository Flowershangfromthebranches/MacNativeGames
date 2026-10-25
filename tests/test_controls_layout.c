#include "../games/totk/runtime/controls_layout.h"
#include <assert.h>
int main(void){
 ControlsLayout b,original;controls_layout_default(&b);original=b;const char *error=NULL;
 assert(b.keys[38]==1&&b.keys[49]==2&&b.keys[53]==12&&b.keys[0]==17&&b.keys[126]==23);
 assert(!controls_layout_parse("# existing layout\nsouth = x tap\nmouse_right = zr aim mouse5\nkey_38 = none\nkey_7 = a\nkey_0 = look_left\n",&b,&error));
 assert(b.map[0]==4&&b.modes[0]==CONTROLLER_TAP&&controller_mode_fire(b.modes[18])==21);
 assert(!b.keys[38]&&b.keys[7]==1&&b.keys[0]==21);
 char text[16384];assert(controls_layout_format(&b,text,sizeof text)>0);ControlsLayout roundtrip;controls_layout_default(&roundtrip);
 assert(!controls_layout_parse(text,&roundtrip,&error));assert(!memcmp(&b,&roundtrip,sizeof b));
 const char *bad[]={"key_128 = a","key_-1 = a","key_1x = a","key_7 = a tap","key_18 = b","key_38 = invalid","mouse_right = zr aim mouse_right"};
 for(unsigned i=0;i<sizeof bad/sizeof *bad;i++){char row[256];snprintf(row,sizeof row,"south = b\n%s",bad[i]);ControlsLayout keep=b;assert(controls_layout_parse(row,&b,&error)==2);assert(!memcmp(&keep,&b,sizeof b));}
 assert(!controls_layout_parse("KEY_7 = B # comment\nkey_18 = none\n",&b,&error));
 assert(!controls_layout_format(&b,text,8));
 /* Options (the control menu's input pages): absent = the default, written only where they differ. */
 ControlsLayout o;controls_layout_default(&o);char plain[16384];assert(controls_layout_format(&o,plain,sizeof plain)>0&&!strstr(plain,"swap_sticks")&&!strstr(plain,"game_"));
 assert(!controls_layout_parse("swap_sticks = on\ninvert_look_y = on # look\nmouse_drag_look = on\nmouse_invert_y = off\nmouse_sensitivity = 80\ngame_a = b\ngame_minus = none\n",&o,&error));
 assert(o.swap_sticks&&o.invert_look_y&&o.mouse_drag_look&&o.mouse_invert_y==1&&o.mouse_sensitivity==80&&o.game[0]==1&&o.game[11]==CONTROLS_GAME_NONE&&o.game[1]==1);
 assert(controls_layout_format(&o,text,sizeof text)>0&&strstr(text,"game_a = b\n")&&strstr(text,"game_minus = none\n")&&strstr(text,"mouse_invert_y = off\n")&&strstr(text,"mouse_sensitivity = 80\n"));
 ControlsLayout o2;controls_layout_default(&o2);assert(!controls_layout_parse(text,&o2,&error));assert(!memcmp(&o,&o2,sizeof o));
 const char *badOption[]={"swap_sticks = yes","mouse_sensitivity = 0","mouse_sensitivity = 101","game_a = q","game_a = b c","mouse_drag_look ="};
 for(unsigned i=0;i<sizeof badOption/sizeof *badOption;i++){ControlsLayout keep=o;assert(controls_layout_parse(badOption[i],&o,&error)==1);assert(!memcmp(&keep,&o,sizeof o));}
 { uint8_t game[CONTROLS_GAME_BUTTONS];for(unsigned i=0;i<CONTROLS_GAME_BUTTONS;i++)game[i]=(uint8_t)i;game[0]=1;game[1]=CONTROLS_GAME_NONE;
   assert(controls_game_apply(game,1)==2&&controls_game_apply(game,2)==0&&controls_game_apply(game,1ull<<16|8)==(1ull<<16|8)); }
 assert(controls_layout_valid(&original));puts("PASS: shared controls.txt parsing, validation, rollback, round-trip and menu options");
}
