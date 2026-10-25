/* Exercise the real AppKit menu and window-close handler without game data. */
#include "../games/totk/runtime/frontend.m"
#include <time.h>

static int mode;
static uint64_t start;
static uint64_t clock_ns(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (uint64_t)t.tv_sec*1000000000ull+t.tv_nsec;}
uint64_t runtime_now_ns(void){return (clock_ns()-start)*(mode==2?100:1);}
int runtime_fs_save_busy(void){return mode==2||(mode==1&&runtime_now_ns()<200000000ull);}
_Noreturn void runtime_fail(const char *message){fprintf(stderr,"FAIL: %s\n",message);exit(1);}
void runtime_input_key(unsigned code,int down){(void)code;(void)down;}
void runtime_input_clear(void){}
static int suspended;void runtime_input_suspend(int on){suspended=on;}
void runtime_gamepad_start(void){}
int runtime_gamepad_type(int *detected){if(detected)*detected=-1;return 0;}
int runtime_mouse_enabled(void){return 0;}
void runtime_mouse_set_hook(void (*hook)(void)){(void)hook;}
void runtime_mouse_move(double dx,double dy){(void)dx;(void)dy;}
void runtime_mouse_button(int button,int down){(void)button;(void)down;}
void runtime_mouse_release(void){}
void runtime_oe_push_message(uint32_t message){(void)message;}
void hle_bind_system_event(void *event,int autoclear){(void)event;(void)autoclear;}
void hle_signal_system_event(void *event){(void)event;}
uint64_t dm_nvn_frame_count(void){return 0;}
int fps_overlay_toggle_hints(void){return 0;}
int hud_compact(void){return 0;}
int hud_snapshot(int line,char *out,size_t n,int *highlight){(void)line;(void)out;(void)n;(void)highlight;return 0;}
int fps_overlay_full_hud(void){return 1;}
int dm_gfx_fps_now(int *generated){if(generated)*generated=0;return -1;}
int mfx_enabled(void){return 0;}
int mfxup_output_enabled(void){return 0;}
uint64_t runtime_output_size_current(void){return 1920|(uint64_t)1080<<32;}
void runtime_aspect_view(double w,double h){(void)w;(void)h;}
int runtime_aspect_step(void){return -1;}
float runtime_aspect_now(void){return 0;}
int taa_enabled(void){return 0;}
int taa_effective(void){return 0;}
void runtime_input_key_synthetic(unsigned code,int down){(void)code;(void)down;}
void runtime_input_host_shortcuts(int on){(void)on;}
void runtime_game_buttons_set(const uint8_t game[CONTROLS_GAME_BUTTONS]){(void)game;}
void runtime_mouse_drag(int down){(void)down;}
int runtime_render_configure(unsigned w,unsigned h,int quality){return totk_control_request(w,h,quality)?2:0;}
void dm_gfx_set_flag(const char *name,int v){(void)name;(void)v;}
int dm_gfx_vsync(void){return 1;}
uint64_t dm_gfx_scene_size(void){return 0;}
int nvn_uncapped_hz(void){return 120;}
int nvn_fi_auto_on(void){return 0;}
uint64_t mfxup_done(int context){(void)context;return 0;}
void taa_toggle(void){}
void runtime_keyboard_layout_set(const uint8_t keys[CONTROLS_KEYS]){(void)keys;}
void runtime_controller_layout_set(const ControlsLayout *layout){(void)layout;}


#include "../games/totk/runtime/action_runtime.h"
#include <assert.h>

static void spin(void){[NSRunLoop.currentRunLoop runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];}
int main(int argc,char **argv){@autoreleasepool{
 assert(argc==2);NSString *folder=@(argv[1]);NSString *controls=[folder stringByAppendingPathComponent:@"controls.txt"];setenv("TOTK_CONTROLS",controls.fileSystemRepresentation,1);
 NSString *suite=[NSString stringWithFormat:@"org.macnativegames.probe.%d",getpid()];displayDefaults=[[NSUserDefaults alloc] initWithSuiteName:suite];controlsLoaded=YES;
 RuntimeActionBindings b;runtime_action_bindings_get(&b);assert(!runtime_action_bindings_enabled());
 ActionLive live={0};runtime_action_bindings_attach(&live);assert(runtime_action_bindings_available());
 [NSApplication sharedApplication];[NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];[NSApp finishLaunching];
 window=[[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,900,700) styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO];window.releasedWhenClosed=NO;window.contentView=[TOTKInputView new];front_install_menu();
 TOTKGameControls *c=gameControls;[c showSettings:nil];assert(controlsOpen&&c.actionTargets.count==16&&suspended&&c.actionTargets[0].enabled);
 c->actions.target[0]=ACTION_JUMP;[c applySettings:nil];spin();
 assert(!controlsOpen&&!suspended);assert(atomic_load(&live.target[0])==ACTION_JUMP);
 NSString *path=[folder stringByAppendingPathComponent:@"player-actions.txt"];NSString *saved=[NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:NULL];assert([saved hasPrefix:@"4,"]);
 assert(![[NSFileManager defaultManager] fileExistsAtPath:controls]);   /* unchanged bindings: controls.txt is not created */
 [c showSettings:nil];[c resetActions:nil];[c cancelSettings:nil];spin();
 assert([[NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:NULL] isEqualToString:saved]);
 [c showSettings:nil];c->actions.target[1]=ACTION_JUMP;assert([@"external" writeToFile:path atomically:YES encoding:NSUTF8StringEncoding error:NULL]);[c applySettings:nil];assert(controlsOpen&&c.error.stringValue.length);[c cancelSettings:nil];spin();
 assert([saved writeToFile:path atomically:YES encoding:NSUTF8StringEncoding error:NULL]);
 [c showSettings:nil];runtime_action_bindings_attach(NULL);c->actions.target[1]=ACTION_JUMP;[c applySettings:nil];assert(controlsOpen);assert([[NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:NULL] isEqualToString:saved]);[c cancelSettings:nil];
 [displayDefaults removePersistentDomainForName:suite];
 puts("PASS: control menu actions page: 16 sources, live publication, save, cancel/reset, conflict and failed-install refusal");return 0;
}}
