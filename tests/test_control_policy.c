/* The Mac control menu's pure choices (games/totk/runtime/control_policy.h). */
#include "../games/totk/runtime/control_policy.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
 /* Sizes and presets: 16:9 only, 16 x 9 steps, never below 640 x 360. */
 assert(totk_control_size(1920,1080)&&totk_control_size(640,360)&&!totk_control_size(1920,1200)&&!totk_control_size(3840,2160));
 assert(totk_control_render(1920,1080,0)==(1920|(uint64_t)1080<<32));
 assert(totk_control_render(1920,1080,1)==(1280|(uint64_t)720<<32));
 assert(totk_control_render(1920,1080,3)==(960|(uint64_t)540<<32));
 assert(totk_control_render(1280,720,3)==(640|(uint64_t)360<<32));
 { uint64_t b=totk_control_render(1920,1080,2);assert((b&4095)%16==0&&((b>>32)&4095)%9==0&&(b&4095)*9==((b>>32)&4095)*16); }
 assert(!totk_control_render(1920,1080,4)&&!totk_control_render(1920,1080,-1));
 for(unsigned i=0;i<CONTROL_SIZES;i++)for(int q=0;q<CONTROL_QUALITIES;q++){
  uint64_t r=totk_control_request(control_sizes[i][0],control_sizes[i][1],q),s=totk_control_render(control_sizes[i][0],control_sizes[i][1],q);
  assert((r&4095)==control_sizes[i][0]&&((r>>12)&4095)==control_sizes[i][1]&&((r>>24)&4095)==(s&4095)&&((r>>36)&4095)==((s>>32)&4095)&&(int)(r>>48)==q);}
 assert(!totk_control_request(1000,1000,1));
 /* The limit: 0 is the uncapped clock's own maximum. */
 assert(control_cap_valid(0)&&control_cap_valid(15)&&control_cap_valid(240)&&!control_cap_valid(14)&&!control_cap_valid(241)&&!control_cap_valid(-1));
 assert(control_cap_hz(0)==240&&control_cap_hz(60)==60);
 /* Interpolation tiers map onto the existing modes and back. */
 assert(control_fi_mode(CONTROL_FI_OFF)==MFX_OFF&&control_fi_mode(CONTROL_FI_AUTO)==MFX_HALF&&control_fi_mode(CONTROL_FI_DISPLAY)==MFX_HALF&&control_fi_mode(CONTROL_FI_30)==MFX_30&&control_fi_mode(CONTROL_FI_UNCAPPED)==MFX_MAX&&control_fi_mode(9)==MFX_OFF);
 for(int t=0;t<CONTROL_FI_TIERS;t++)assert(control_fi_tier(control_fi_mode(t),t==CONTROL_FI_AUTO)==t);
 assert(control_fi_paces(CONTROL_FI_AUTO)&&control_fi_paces(CONTROL_FI_DISPLAY)&&control_fi_paces(CONTROL_FI_30)&&!control_fi_paces(CONTROL_FI_OFF)&&!control_fi_paces(CONTROL_FI_UNCAPPED));
 /* Shortcuts: the defaults are the game's keys (1-7, 3 picture, 5 TAA) and 8 for the menu; the preset cycle starts unassigned. */
 static const unsigned keys[]={28,18,19,20,21,23,22,26};
 for(unsigned i=0;i<8;i++)assert(control_shortcut_default(i)==control_hotkey(keys[i],0));
 assert(!control_shortcut_default(CONTROL_SHORTCUT_PRESET)&&!control_shortcut_default(CONTROL_SHORTCUTS));
 assert(control_hotkey(0,0)==1&&control_hotkey(127,15)==(128|15u<<8)&&!control_hotkey(128,0)&&!control_hotkey(1,16));
 assert(control_hotkey_reserved(control_hotkey(12,CONTROL_MOD_COMMAND))&&control_hotkey_reserved(control_hotkey(43,CONTROL_MOD_COMMAND))&&!control_hotkey_reserved(control_hotkey(12,0)));
 uint8_t targets[128]={0};targets[38]=1;uint64_t set[CONTROL_SHORTCUTS];for(unsigned i=0;i<CONTROL_SHORTCUTS;i++)set[i]=control_shortcut_default(i);
 assert(!control_shortcuts_check(set,targets));
 set[CONTROL_SHORTCUT_PRESET]=control_hotkey(38,0);assert(control_shortcuts_check(set,targets)==CONTROL_SHORTCUT_PRESET+1);   /* J drives A */
 set[CONTROL_SHORTCUT_PRESET]=control_hotkey(38,CONTROL_MOD_SHIFT);assert(!control_shortcuts_check(set,targets));             /* Shift-J does not */
 set[CONTROL_SHORTCUT_PRESET]=set[CONTROL_SHORTCUT_HUD];assert(control_shortcuts_check(set,targets)==CONTROL_SHORTCUT_PRESET+1);
 set[CONTROL_SHORTCUT_PRESET]=control_hotkey(43,CONTROL_MOD_COMMAND);assert(control_shortcuts_check(set,targets)==CONTROL_SHORTCUT_PRESET+1);
 set[CONTROL_SHORTCUT_PRESET]=0;set[CONTROL_SHORTCUT_HUD]=0;assert(!control_shortcuts_check(set,targets));
 puts("PASS: control menu sizes, presets, limit, interpolation tiers and shortcut checks");return 0;
}
