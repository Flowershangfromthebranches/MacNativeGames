/* Exercise the production input dispatcher without a game, window or player data. */
#include "../games/totk/runtime/input.c"
#include <assert.h>
static int active=1,actions,userNotices,legacyOverlay,legacyFI,legacyOutput,legacyAspect;
static unsigned binding=13,bindingMods=1;
int runtime_controls_active(void){return active;}
int runtime_controls_handle_key(unsigned c,unsigned m){if(active&&c==binding&&m==bindingMods){actions++;return 1;}return 0;}
void runtime_survey_cancel_user(void){}
void runtime_autoload_user_input(void){userNotices++;}
uint64_t runtime_now_ns(void){return 1000000000;}
void runtime_gamepad_clear(void){}
void runtime_shortcut_help_toggle(void){legacyOverlay++;}
void dm_gfx_set_flag(const char *s,int v){(void)s;(void)v;legacyFI++;}
int runtime_output_mode_cycle(void){legacyOutput++;return 1;}
int runtime_aspect_toggle(void){legacyAspect++;return 1;}
/* Constructor-installed diagnostic streams are disabled in this probe. Any accidental
   diagnostic call must fail rather than silently behaving like the real game. */
#define UNREACHABLE(name,args) void name args { abort(); }
UNREACHABLE(dm_gfx_anomdump,(void))
UNREACHABLE(dm_gfx_passdump_next,(long n))
UNREACHABLE(dm_gfx_querylog_next,(long n))
UNREACHABLE(dm_gfx_shot_at,(long n,const char *p))
UNREACHABLE(dm_gfx_texdump_next,(const char *p))
UNREACHABLE(dm_gfx_window_shot,(const char *p))
UNREACHABLE(runtime_dump_threads,(void))
UNREACHABLE(runtime_frame_step_pin,(double n))
UNREACHABLE(runtime_front_control_test,(const char *s))
UNREACHABLE(runtime_front_request_close,(void))
UNREACHABLE(runtime_front_text_test,(const char *s,const char *p))
UNREACHABLE(runtime_hle_trace_set,(int n))
UNREACHABLE(runtime_swkbd_selftest,(void))
int runtime_survey_command(const char *s){(void)s;abort();}
int runtime_hid_feedback_flag(const char *s,int n){(void)s;(void)n;abort();}
_Noreturn void runtime_fail(const char *s){fprintf(stderr,"unexpected runtime failure: %s\n",s);abort();}
int main(void){
 runtime_input_key_modifiers(13,1,1);assert(actions==1&&!keys[13]&&!pending&&userNotices==1);
 runtime_input_key_modifiers(13,1,1);assert(actions==1); // held keys never toggle repeatedly
 runtime_input_key_modifiers(13,0,0);assert(!keys[13]);
 runtime_input_key_modifiers(13,1,0);assert(actions==1&&keys[13]); // modifiers must match exactly
 runtime_input_key_modifiers(13,0,0);
 bindingMods=0;
 runtime_input_key_modifiers(13,1,1);assert(actions==1&&keys[13]); // Cmd+W must not fire plain W
 runtime_input_clear();
 runtime_input_key_synthetic(13,1);runtime_input_key_synthetic(13,1);assert(actions==2&&!keys[13]);
 runtime_input_key_synthetic(13,0);runtime_input_key_synthetic(13,1);assert(actions==3);
 runtime_input_clear();
 active=0;
 unsigned legacy[]={18,19,20,21};for(unsigned i=0;i<4;i++){
  runtime_input_key_synthetic(legacy[i],1);runtime_input_key_synthetic(legacy[i],1);runtime_input_key_synthetic(legacy[i],0);
 }
 assert(legacyOverlay==1&&legacyFI==1&&legacyOutput==1&&legacyAspect==1);
 puts("PASS: exact modifiers, held keys, consumed controller input, takeover notification, legacy fallbacks");
}
