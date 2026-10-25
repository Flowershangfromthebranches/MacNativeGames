/* The Mac control menu ("Game Controls", ⌘, or 8): video, frame rate, shortcuts and the input pages in one sheet.
   macOS only: included by frontend.m, which the iOS target does not build, so nothing here is referenced from shared code.
   Every choice is "not set" until the player applies a change to it; a setting that was never applied is never touched,
   so a player who does not use the menu runs exactly as without it (the launcher's environment, keys 1-7).
   Storage: video, frame rate and shortcuts in the NSUserDefaults suite org.macnativegames.<game>.display (".tests" for
   agent sessions); bindings and input options in controls.txt (controls_layout.h, the launcher validates the same file),
   player actions in player-actions.txt beside it (action_bindings.h).
   Live paths only, no new pacing: vsync and the frame limit are the graphics flags "vsync" / "uncapped_hz" (the existing
   display link and uncapped clock), frame interpolation is "mfx_interp" / "mfx_auto", the resolution is
   runtime_render_configure (latched at the present boundary like key 3), the shortcuts run the keys' own handlers. */
#pragma once
#include "control_policy.h"
void dm_gfx_set_flag(const char *name, int v);   /* gfx/nvn/nvn_mtl.m */
int dm_gfx_vsync(void);
uint64_t dm_gfx_scene_size(void);
int nvn_uncapped_hz(void);                       /* gfx/nvn/hle_nvn.c */
int nvn_fi_auto_on(void);
uint64_t mfxup_done(int context);                /* gfx/nvn/totk_mfx_up.m */
void taa_toggle(void);
void runtime_input_key_synthetic(unsigned code, int down);   /* input.c: runs the fixed keys' own handlers */
#if !defined(GAME_BOTW) && !defined(TOTK_GAME_V112) && !defined(TOTK_GAME_V121)
#define CONTROLS_RESOLUTION 1   /* 1.0.0 and 1.4.3 have the output and render patches the resolution drives */
#endif

enum { TOTK_WINDOWED, TOTK_FULLSCREEN, TOTK_BORDERLESS };
enum { CONTROLS_SOURCE_KEYS = CONTROLS_KEYS, CONTROLS_SOURCE_MOUSE = CONTROLS_KEYS + 1 };   /* slot sources: key code + 1, or mouse button n + 129 */
enum { CONTROLLER_PRESET_SWITCH, CONTROLLER_PRESET_XBOX, CONTROLLER_PRESET_PLAYSTATION, CONTROLLER_PRESET_GENERIC, CONTROLLER_PRESET_CUSTOM };
static NSUserDefaults *displayDefaults;
static BOOL controlsLoaded, controlsDragLook, displayTransition, displayOurs, displayBorderless, renderRestart;
static int displayMode = TOTK_WINDOWED;
static NSRect displayWindowFrame;
static _Atomic uint64_t controlKeys[CONTROL_SHORTCUTS];
static void front_sync_drawable(void);

static NSString *hotkey_title(uint64_t key) {
 if(!key)return @(TOTK_UI("未分配","Unassigned"));unsigned c=(unsigned)(key&255)-1,m=(unsigned)(key>>8);
 static const char *const names[128]={[0]="A",[1]="S",[2]="D",[3]="F",[4]="H",[5]="G",[6]="Z",[7]="X",[8]="C",[9]="V",[11]="B",
  [12]="Q",[13]="W",[14]="E",[15]="R",[16]="Y",[17]="T",[18]="1",[19]="2",[20]="3",[21]="4",[22]="6",[23]="5",[24]="=",[25]="9",[26]="7",[27]="-",[28]="8",[29]="0",
  [30]="]",[31]="O",[32]="U",[33]="[",[34]="I",[35]="P",[36]="Return",[37]="L",[38]="J",[39]="'",[40]="K",[41]=";",[42]="\\",[43]=",",[44]="/",[45]="N",[46]="M",[47]=".",
  [48]="Tab",[49]="Space",[50]="`",[51]="Delete",[53]="Esc",[54]="Right Command",[55]="Left Command",[56]="Left Shift",[57]="Caps Lock",[58]="Left Option",[59]="Left Control",
  [60]="Right Shift",[61]="Right Option",[62]="Right Control",[63]="Fn",[65]="Keypad .",[67]="Keypad *",[69]="Keypad +",[71]="Keypad Clear",[75]="Keypad /",[76]="Keypad Enter",
  [78]="Keypad -",[81]="Keypad =",[82]="Keypad 0",[83]="Keypad 1",[84]="Keypad 2",[85]="Keypad 3",[86]="Keypad 4",[87]="Keypad 5",[88]="Keypad 6",[89]="Keypad 7",[91]="Keypad 8",
  [92]="Keypad 9",[96]="F5",[97]="F6",[98]="F7",[99]="F3",[100]="F8",[101]="F9",[103]="F11",[109]="F10",[111]="F12",[115]="Home",[116]="Page Up",[117]="Forward Delete",
  [118]="F4",[119]="End",[120]="F2",[121]="Page Down",[122]="F1",[123]="←",[124]="→",[125]="↓",[126]="↑"};
 NSString *name=c<128&&names[c]?@(names[c]):[NSString stringWithFormat:@"Key %u",c];
 if(c==56||c==57||c==58||c==59||c==60||c==61||c==62)name=@[@(TOTK_UI("左 Shift","Left Shift")),@"Caps Lock",@(TOTK_UI("左 Option","Left Option")),@(TOTK_UI("左 Control","Left Control")),@(TOTK_UI("右 Shift","Right Shift")),@(TOTK_UI("右 Option","Right Option")),@(TOTK_UI("右 Control","Right Control"))][c-56];
 return [NSString stringWithFormat:@"%@%@%@%@%@",m&CONTROL_MOD_CONTROL?@"⌃":@"",m&CONTROL_MOD_OPTION?@"⌥":@"",m&CONTROL_MOD_SHIFT?@"⇧":@"",m&CONTROL_MOD_COMMAND?@"⌘":@"",name];
}
static unsigned hotkey_mods(NSEventModifierFlags f){return (f&NSEventModifierFlagCommand?CONTROL_MOD_COMMAND:0)|(f&NSEventModifierFlagOption?CONTROL_MOD_OPTION:0)|(f&NSEventModifierFlagControl?CONTROL_MOD_CONTROL:0)|(f&NSEventModifierFlagShift?CONTROL_MOD_SHIFT:0);}
/* A modifier key's own press (flagsChanged): the device bits tell left from right, as frontend.m's flagsChanged does. */
static BOOL controls_modifier_down(unsigned code,NSEventModifierFlags flags){
 static const struct{unsigned key;uint64_t mask;} m[]={{56,2},{60,4},{58,32},{61,64},{59,1},{62,8192},{57,NSEventModifierFlagCapsLock},{63,NSEventModifierFlagFunction}};
 for(unsigned i=0;i<sizeof m/sizeof *m;i++)if(code==m[i].key)return (flags&m[i].mask)!=0;
 return NO;
}
static NSString *controls_source_title(unsigned source){
 if(!source)return @(TOTK_UI("未分配","Unassigned"));
 if(source>=CONTROLS_SOURCE_MOUSE){NSArray *mouse=@[@(TOTK_UI("鼠标左键","Mouse left")),@(TOTK_UI("鼠标右键","Mouse right")),@(TOTK_UI("鼠标中键","Mouse middle")),@(TOTK_UI("鼠标 4","Mouse 4")),@(TOTK_UI("鼠标 5","Mouse 5"))];return mouse[source-CONTROLS_SOURCE_MOUSE];}
 return hotkey_title(source);
}

/* ---- controls.txt and player-actions.txt ---- */
static NSString *controls_path(void){const char *p=getenv("TOTK_CONTROLS");return p&&*p?@(p):nil;}
static NSString *actions_path(void){NSString *p=controls_path();return p?[[p stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"player-actions.txt"]:nil;}
static NSError *controls_error(NSString *message){return [NSError errorWithDomain:@"MacNativeGames.Controls" code:1 userInfo:@{NSLocalizedDescriptionKey:message}];}
/* Reads controls.txt into *layout (defaults when the file does not exist); *original is the bytes read, for Apply's check. */
static BOOL controls_read(ControlsLayout *layout,NSData **original,NSError **error){
 controls_layout_default(layout);*original=nil;NSString *path=controls_path();
 if(!path){if(error)*error=controls_error(@(TOTK_UI("启动器未设置 controls.txt 路径。","The launcher did not set a controls.txt path.")));return NO;}
 NSError *problem=nil;NSData *data=[NSData dataWithContentsOfFile:path options:0 error:&problem];
 if(!data&&problem.code!=NSFileReadNoSuchFileError){if(error)*error=problem;return NO;}
 NSString *text=data?[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding]:@"";const char *why="invalid UTF-8 or embedded NUL";
 int line=!text||data.length>=16384||(data&&memchr(data.bytes,0,data.length))?1:controls_layout_parse(text.UTF8String,layout,&why);
 if(line){if(error)*error=controls_error([NSString stringWithFormat:@"controls.txt, line %d: %s",line,why]);return NO;}
 *original=data;return YES;
}
/* Writes the canonical layout, keeping the player's comments; refuses when the file changed since it was read. */
static BOOL controls_write(const ControlsLayout *layout,NSData **original,NSError **error){
 char text[16384];if(!controls_layout_format(layout,text,sizeof text)){if(error)*error=controls_error(@"Invalid bindings");return NO;}
 NSString *path=controls_path();NSError *problem=nil;NSData *current=[NSData dataWithContentsOfFile:path options:0 error:&problem];
 if(!current&&problem.code!=NSFileReadNoSuchFileError){if(error)*error=problem;return NO;}
 if((current||*original)&&![current isEqualToData:*original]){if(error)*error=controls_error(@(TOTK_UI("controls.txt 已被其他程序修改。取消后重新打开以读取新内容。","controls.txt changed outside the menu. Cancel and reopen to read it.")));return NO;}
 NSMutableString *output=[NSMutableString string];NSString *old=*original?[[NSString alloc] initWithData:*original encoding:NSUTF8StringEncoding]:@"";
 for(NSString *row in [old componentsSeparatedByString:@"\n"]){NSRange hash=[row rangeOfString:@"#"];if(hash.location!=NSNotFound)[output appendFormat:@"%@\n",[row substringFromIndex:hash.location]];}
 [output appendString:@(text)];NSData *data=[output dataUsingEncoding:NSUTF8StringEncoding];
 if(data.length>=16384){if(error)*error=controls_error(@"controls.txt is too large (maximum 16383 bytes)");return NO;}
 if(![data writeToFile:path options:NSDataWritingAtomic error:error])return NO;
 *original=data;return YES;
}
static BOOL controls_layout_same(const ControlsLayout *a,const ControlsLayout *b){
 static char x[16384],y[16384];int n=controls_layout_format(a,x,sizeof x),m=controls_layout_format(b,y,sizeof y);
 return n&&n==m&&!memcmp(x,y,(size_t)n);
}

/* ---- the saved video / frame-rate / shortcut choices ---- */
static void display_presentation(void){
 if(displayTransition||(window.styleMask&NSWindowStyleMaskFullScreen))return;
 NSApp.presentationOptions=displayBorderless&&NSApp.active?(NSApplicationPresentationAutoHideDock|NSApplicationPresentationAutoHideMenuBar):NSApplicationPresentationDefault;
}
static void display_restore_window(void){
 if(!displayBorderless)return;
 displayBorderless=NO;
 window.styleMask=NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskResizable;
 window.level=NSNormalWindowLevel;display_presentation();
 if(!NSIsEmptyRect(displayWindowFrame))[window setFrame:displayWindowFrame display:YES];
}
static void display_apply_mode(void){
 if(displayTransition||!window)return;
 BOOL full=(window.styleMask&NSWindowStyleMaskFullScreen)!=0;
 if(full){if(displayMode!=TOTK_FULLSCREEN){displayTransition=displayOurs=YES;[window toggleFullScreen:nil];}return;}
 if(displayMode==TOTK_FULLSCREEN){display_restore_window();displayTransition=displayOurs=YES;[window toggleFullScreen:nil];return;}
 if(displayMode==TOTK_BORDERLESS){
  NSScreen *screen=window.screen?:NSScreen.mainScreen;if(!screen)return;
  if(!displayBorderless){displayWindowFrame=window.frame;displayBorderless=YES;window.styleMask=NSWindowStyleMaskBorderless;}
  /* A normal-level window that fills this desktop, not an always-on-top overlay. */
  if(!NSEqualRects(window.frame,screen.frame))[window setFrame:screen.frame display:YES];
  [window makeFirstResponder:window.contentView];
 }else display_restore_window();
 display_presentation();front_sync_drawable();
}
/* The window delegate: a full-screen change finished. One the player made (the green button, ⌃⌘F) becomes the mode in
   force for this run without being saved. */
static void display_transition_done(void){
 BOOL full=(window.styleMask&NSWindowStyleMaskFullScreen)!=0;
 if(!displayOurs)displayMode=full?TOTK_FULLSCREEN:displayBorderless?TOTK_BORDERLESS:TOTK_WINDOWED;
 displayTransition=displayOurs=NO;display_apply_mode();
}
static int display_live_mode(void){return window&&(window.styleMask&NSWindowStyleMaskFullScreen)?TOTK_FULLSCREEN:displayBorderless?TOTK_BORDERLESS:TOTK_WINDOWED;}
static BOOL controls_saved(NSString *key){return [displayDefaults objectForKey:key]!=nil;}
static int controls_saved_int(NSString *key,int lo,int hi,int fallback){id v=[displayDefaults objectForKey:key];if(![v isKindOfClass:NSNumber.class])return fallback;long n=[v longValue];return n>=lo&&n<=hi?(int)n:fallback;}
static void controls_fi_apply(int tier){
 if(tier==CONTROL_FI_AUTO)dm_gfx_set_flag("mfx_auto",1);
 else dm_gfx_set_flag("mfx_interp",control_fi_mode(tier));   /* also ends auto */
}
#ifdef CONTROLS_RESOLUTION
static int controls_render_apply(void){
 int r=controls_saved_int(@"resolution",0,CONTROL_SIZES-1,-1),q=controls_saved_int(@"upscalePreset",0,CONTROL_QUALITIES-1,1);
 if(r<0)return 0;
 int how=runtime_render_configure(control_sizes[r][0],control_sizes[r][1],q);renderRestart=how==2;return how;
}
#endif
static void display_load(void){
 if(controlsLoaded)return;
 const char *owner=getenv("TOTK_SLOT_OWNER");
 displayDefaults=[[NSUserDefaults alloc] initWithSuiteName:[NSString stringWithFormat:@"org.macnativegames.%s.display%s",GAME_LOG,owner&&*owner&&strcmp(owner,"user")?".tests":""]];
 ControlsLayout layout;NSData *original;if(!controls_read(&layout,&original,NULL))controls_layout_default(&layout);
 controlsDragLook=layout.mouse_drag_look;
 NSArray *saved=[displayDefaults arrayForKey:@"controlShortcuts"];uint64_t keys[CONTROL_SHORTCUTS];
 for(unsigned i=0;i<CONTROL_SHORTCUTS;i++){id v=saved.count==CONTROL_SHORTCUTS?saved[i]:nil;keys[i]=[v isKindOfClass:NSNumber.class]?[v unsignedLongLongValue]:control_shortcut_default(i);
  if(!control_hotkey_valid(keys[i])||control_hotkey_reserved(keys[i]))keys[i]=control_shortcut_default(i);}
 /* A controls.txt edited elsewhere can take a shortcut's key; the gameplay binding wins (⌘, still opens the menu). */
 for(unsigned i=0;i<CONTROL_SHORTCUTS;i++){if(control_hotkey_conflicts(keys[i],layout.keys))keys[i]=0;for(unsigned j=0;j<i;j++)if(keys[i]&&keys[i]==keys[j])keys[i]=0;atomic_store(&controlKeys[i],keys[i]);}
 controlsLoaded=YES;
}
/* Before the game's code is patched (loader.c): the saved resolution decides whether 1.0.0's render patch is installed. */
void runtime_controls_prepare(void){@autoreleasepool{
 display_load();
#ifdef CONTROLS_RESOLUTION
 controls_render_apply();
#endif
 if(controls_saved(@"vsync"))dm_gfx_set_flag("vsync",[displayDefaults boolForKey:@"vsync"]);
 if(controls_saved(@"nativeFrameCap")){int cap=controls_saved_int(@"nativeFrameCap",0,CONTROL_CAP_MAX,-1);if(control_cap_valid(cap))dm_gfx_set_flag("uncapped_hz",control_cap_hz(cap));}
 if(controls_saved(@"frameGeneration")){int fi=controls_saved_int(@"frameGeneration",0,CONTROL_FI_TIERS-1,-1);if(fi>=0)controls_fi_apply(fi);}
 if(controls_saved(@"taa")&&[displayDefaults boolForKey:@"taa"]!=!!taa_enabled())taa_toggle();
}}
/* After the window is shown (runtime_front_run): the saved window mode. */
static void controls_window_ready(void){
 display_load();runtime_input_host_shortcuts(1);
 if(controls_saved(@"mode")){displayMode=controls_saved_int(@"mode",0,2,TOTK_WINDOWED);dispatch_async(dispatch_get_main_queue(),^{display_apply_mode();});}
}

/* ---- the sheet ---- */
static NSView *controls_tab(NSTabView *tabs,NSString *name){NSTabViewItem *item=[[NSTabViewItem alloc] initWithIdentifier:name];item.label=name;item.view=[[NSView alloc] initWithFrame:NSMakeRect(0,0,600,390)];[tabs addTabViewItem:item];return item.view;}
static void controls_label(NSView *v,NSString *text,CGFloat y){NSTextField *l=[NSTextField labelWithString:text];l.frame=NSMakeRect(20,y,190,25);[v addSubview:l];}
static NSTextField *controls_hint(NSView *v,NSString *text,CGFloat y,CGFloat height){NSTextField *l=[NSTextField wrappingLabelWithString:text];l.frame=NSMakeRect(20,y,550,height);l.font=[NSFont systemFontOfSize:13];l.textColor=NSColor.secondaryLabelColor;[v addSubview:l];return l;}
static NSPopUpButton *controls_popup(NSView *v,NSArray *items,CGFloat y){NSPopUpButton *b=[[NSPopUpButton alloc] initWithFrame:NSMakeRect(218,y,350,28) pullsDown:NO];[b addItemsWithTitles:items];[v addSubview:b];return b;}
static NSView *controls_scroll(NSView *parent,CGFloat height,NSRect frame){NSScrollView *scroll=[[NSScrollView alloc] initWithFrame:frame];scroll.hasVerticalScroller=YES;scroll.borderType=NSBezelBorder;NSView *content=[[NSView alloc] initWithFrame:NSMakeRect(0,0,550,height)];scroll.documentView=content;[parent addSubview:scroll];[content scrollPoint:NSMakePoint(0,height)];return content;}
static NSArray<NSString *> *controls_target_titles(void){return @[@"A",@"B",@"X",@"Y",@"L3",@"R3",@"L",@"R",@"ZL",@"ZR",@"+",@"−",@(TOTK_UI("十字键 ←","D-pad ←")),@(TOTK_UI("十字键 ↑","D-pad ↑")),@(TOTK_UI("十字键 →","D-pad →")),@(TOTK_UI("十字键 ↓","D-pad ↓")),
 @(TOTK_UI("移动 ←","Move ←")),@(TOTK_UI("移动 →","Move →")),@(TOTK_UI("移动 ↑","Move ↑")),@(TOTK_UI("移动 ↓","Move ↓")),@(TOTK_UI("视角 ←","Look ←")),@(TOTK_UI("视角 →","Look →")),@(TOTK_UI("视角 ↑","Look ↑")),@(TOTK_UI("视角 ↓","Look ↓"))];}
static NSArray<NSString *> *controls_physical_titles(NSInteger preset){
 BOOL zh=totk_ui_zh();
 NSArray *faces=preset==CONTROLLER_PRESET_SWITCH?(zh?@[@"下 / B",@"右 / A",@"左 / Y",@"上 / X"]:@[@"South / B",@"East / A",@"West / Y",@"North / X"]):
  preset==CONTROLLER_PRESET_XBOX?(zh?@[@"下 / A",@"右 / B",@"左 / X",@"上 / Y"]:@[@"South / A",@"East / B",@"West / X",@"North / Y"]):
  preset==CONTROLLER_PRESET_PLAYSTATION?(zh?@[@"下 / ×",@"右 / ○",@"左 / □",@"上 / △"]:@[@"South / ×",@"East / ○",@"West / □",@"North / △"]):
  (zh?@[@"下 / B · A · ×",@"右 / A · B · ○",@"左 / Y · X · □",@"上 / X · Y · △"]:@[@"South / B · A · ×",@"East / A · B · ○",@"West / Y · X · □",@"North / X · Y · △"]);
 NSMutableArray *names=[faces mutableCopy];
 [names addObjectsFromArray:preset==CONTROLLER_PRESET_SWITCH?@[@"L3",@"R3",@"L",@"R",@"ZL",@"ZR",@"+",@"−"]:preset==CONTROLLER_PRESET_XBOX?@[@"L3",@"R3",@"LB",@"RB",@"LT",@"RT",@"Menu",@"View"]:
  preset==CONTROLLER_PRESET_PLAYSTATION?@[@"L3",@"R3",@"L1",@"R1",@"L2",@"R2",@"Options",@"Share / Create"]:@[@"L3",@"R3",@"L / LB / L1",@"R / RB / R1",@"ZL / LT / L2",@"ZR / RT / R2",@"+ / Menu / Options",@"− / View / Create"]];
 [names addObjectsFromArray:@[@"←",@"↑",@"→",@"↓",@(TOTK_UI("触控板按下","Touchpad click")),@(TOTK_UI("鼠标左键（鼠标模式）","Mouse left (mouse mode)")),@(TOTK_UI("鼠标右键（鼠标模式）","Mouse right (mouse mode)")),@(TOTK_UI("鼠标中键（鼠标模式）","Mouse middle (mouse mode)")),@(TOTK_UI("鼠标 4（鼠标模式）","Mouse 4 (mouse mode)")),@(TOTK_UI("鼠标 5（鼠标模式）","Mouse 5 (mouse mode)"))]];
 return names;
}
static NSString *controls_preset_hint(NSInteger preset){
 switch(preset){
 case CONTROLLER_PRESET_SWITCH:return @(TOTK_UI("Switch：A 确认、B 返回。场景动作请在“动作”页单独设置。","Switch: A confirms, B returns. Configure gameplay separately in Actions."));
 case CONTROLLER_PRESET_XBOX:return @(TOTK_UI("Xbox：A 确认、B 返回。场景动作请在“动作”页单独设置。","Xbox: A confirms, B returns. Configure gameplay separately in Actions."));
 case CONTROLLER_PRESET_PLAYSTATION:return @(TOTK_UI("PlayStation：× 确认、○ 返回。场景动作请在“动作”页单独设置。","PlayStation: × confirms, ○ returns. Configure gameplay separately in Actions."));
 default:return @(TOTK_UI("这里修改菜单也使用的按键映射；场景动作请到“动作”页设置。预设会重置模式与摇杆选项。","These bindings also affect menus; use Actions for gameplay-only changes. Presets reset modes and sticks."));
 }
}
/* Presets set the 17 controller buttons by position and clear the stick options; the mouse's buttons stay as they are. */
static void controls_preset_apply(ControlsLayout *b,unsigned preset){
 uint16_t map[CONTROLLER_BUTTONS];uint8_t modes[CONTROLLER_BUTTONS];controller_map_default(map,modes);
 if(preset==CONTROLLER_PRESET_XBOX){map[0]=1;map[1]=2;map[2]=4;map[3]=8;}
 else if(preset==CONTROLLER_PRESET_PLAYSTATION){map[0]=1;map[1]=2;}   /* Cross confirms, Circle returns; Square attacks, Triangle jumps */
 memcpy(b->map,map,CONTROLLER_MOUSE_FIRST*sizeof *map);memcpy(b->modes,modes,CONTROLLER_MOUSE_FIRST);b->swap_sticks=b->invert_look_y=0;
}
static BOOL controls_preset_matches(const ControlsLayout *b,unsigned preset){
 ControlsLayout e=*b;controls_preset_apply(&e,preset);
 return !memcmp(e.map,b->map,sizeof e.map)&&!memcmp(e.modes,b->modes,sizeof e.modes)&&e.swap_sticks==b->swap_sticks&&e.invert_look_y==b->invert_look_y;
}
static NSArray *controls_action_titles(void){
 return @[@(TOTK_UI("未分配","Unassigned")),@(TOTK_UI("交互","Interact")),@(TOTK_UI("取消 / 奔跑","Cancel / run")),@(TOTK_UI("跳跃","Jump")),@(TOTK_UI("攻击","Attack")),@(TOTK_UI("能力","Ability")),@(TOTK_UI("投掷武器","Throw weapon")),@(TOTK_UI("盾 / 瞄准","Shield / aim")),@(TOTK_UI("弓","Bow")),
  @(TOTK_UI("快捷选择：上","Quick select: up")),@(TOTK_UI("快捷选择：下","Quick select: down")),@(TOTK_UI("快捷选择：左","Quick select: left")),@(TOTK_UI("快捷选择：右","Quick select: right")),@(TOTK_UI("下蹲","Crouch")),@(TOTK_UI("望远镜 / 右摇杆","Scope / R-stick")),@(TOTK_UI("暂停 / 背包","Pause / inventory")),@(TOTK_UI("地图","Map"))];
}
static NSArray *controls_action_sources(void){return @[@"A",@"B",@"X",@"Y",@"L",@"R",@"ZL",@"ZR",@(TOTK_UI("上","Up")),@(TOTK_UI("下","Down")),@(TOTK_UI("左","Left")),@(TOTK_UI("右","Right")),@(TOTK_UI("左摇杆按下","L-stick click")),@(TOTK_UI("右摇杆按下","R-stick click")),@"+",@"−"];}
static int controls_env_sensitivity(void){const char *e=getenv("TOTK_MOUSE_SENSITIVITY");int v=e?atoi(e):0;return v>=1&&v<=100?v:50;}
static BOOL controls_env_invert(void){const char *e=getenv("TOTK_MOUSE_INVERT_Y");return e&&*e&&strcmp(e,"0");}

@interface TOTKGameControls:NSWindowController<NSWindowDelegate,NSTabViewDelegate> {
 @public ControlsLayout layout,loaded;RuntimeActionBindings actions,loadedActions;
}
@property(strong) NSData *controlsOriginal,*actionsOriginal;
@property(strong) NSTabView *tabs;
@property(strong) NSPopUpButton *resolution,*quality,*aa,*mode,*generation,*preset;
@property(strong) NSButton *vsync,*dragLook,*mouseInvert,*swapSticks,*padInvert,*actionOriginal,*actionExample;
@property(strong) NSTextField *cap,*status,*error,*controllerHint,*actionHint,*sensitivityValue;
@property(strong) NSSlider *sensitivity;
@property(strong) NSArray<NSButton *> *hotkeyButtons,*slotButtons;
@property(strong) NSArray<NSPopUpButton *> *padTargets,*padModes,*padFires,*actionTargets,*gameTargets;
@property(strong) NSArray<NSTextField *> *padLabels;
@property(weak) NSView *keyboardTab;
@property(strong) NSMutableArray<NSNumber *> *draftKeys;
@property(strong) id monitor;
@property(strong) NSTimer *statusTimer;
@property NSInteger recordingShortcut,recordingSlot,shownResolution,shownQuality,shownAA,shownMode,shownVsync,shownCap,shownFI;
-(void)showSettings:(id)sender;
-(void)cancelSettings:(id)sender;
@end
static TOTKGameControls *gameControls;
@implementation TOTKGameControls
-(instancetype)init{
 NSPanel *p=[[NSPanel alloc] initWithContentRect:NSMakeRect(0,0,660,550) styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskClosable backing:NSBackingStoreBuffered defer:NO];
 if(!(self=[super initWithWindow:p]))return nil;
 p.title=@(TOTK_UI("控制菜单","Game Controls"));p.releasedWhenClosed=NO;p.delegate=self;self.recordingShortcut=self.recordingSlot=-1;
 self.tabs=[[NSTabView alloc] initWithFrame:NSMakeRect(20,100,620,425)];self.tabs.delegate=self;[p.contentView addSubview:self.tabs];
 [self addVideoTab];[self addFrameTab];[self addShortcutTab];[self addKeyboardTab];[self addControllerTab];[self addActionTab];[self addGameButtonTab];
 self.status=controls_hint(p.contentView,@"",65,28);self.status.frame=NSMakeRect(26,65,608,30);
 self.error=controls_hint(p.contentView,@"",38,24);self.error.textColor=NSColor.systemRedColor;self.error.frame=NSMakeRect(26,38,605,24);self.error.font=[NSFont systemFontOfSize:11];
 NSButton *cancel=[NSButton buttonWithTitle:@(TOTK_UI("取消","Cancel")) target:self action:@selector(cancelSettings:)];cancel.frame=NSMakeRect(444,16,88,32);cancel.keyEquivalent=@"\e";[p.contentView addSubview:cancel];
 NSButton *apply=[NSButton buttonWithTitle:@(TOTK_UI("应用","Apply")) target:self action:@selector(applySettings:)];apply.frame=NSMakeRect(541,16,88,32);apply.keyEquivalent=@"\r";[p.contentView addSubview:apply];
 return self;
}
-(void)addVideoTab{
 NSView *v=controls_tab(self.tabs,@(TOTK_UI("画面","Video")));
 controls_label(v,@(TOTK_UI("目标分辨率","Target resolution")),326);
 self.resolution=controls_popup(v,@[@(TOTK_UI("默认（3 键切换画面模式）","Default (key 3 picture modes)")),@"1280 × 720",@"1600 × 900",@"1920 × 1080",@"2560 × 1440"],322);
 self.resolution.target=self;self.resolution.action=@selector(resolutionChanged:);
 controls_label(v,@(TOTK_UI("MetalFX 空间超分","MetalFX spatial upscaling")),273);
 self.quality=controls_popup(v,@[@(TOTK_UI("关闭 · 100% 渲染","Off · 100% rendering")),@(TOTK_UI("质量 · 约 67% 渲染","Quality · about 67%")),@(TOTK_UI("平衡 · 约 59% 渲染","Balanced · about 59%")),@(TOTK_UI("性能 · 50% 渲染","Performance · 50%"))],269);
 controls_label(v,@(TOTK_UI("抗锯齿","Antialiasing")),220);
 self.aa=controls_popup(v,@[@(TOTK_UI("FXAA（游戏原有）","FXAA (game's original)")),@(TOTK_UI("MetalFX 时域抗锯齿（1.4.3）","MetalFX temporal antialiasing (1.4.3)"))],216);
 self.aa.autoenablesItems=NO;self.aa.toolTip=@(TOTK_UI("1.4.3 使用 MetalFX 时域抗锯齿；按 5 也可切换。","MetalFX temporal antialiasing on 1.4.3; key 5 also toggles it."));
#if !defined(TOTK_GAME_V143)
 [self.aa itemAtIndex:1].enabled=NO;
#endif
 controls_label(v,@(TOTK_UI("窗口模式","Window mode")),167);
 self.mode=controls_popup(v,@[@(TOTK_UI("窗口","Windowed")),@(TOTK_UI("全屏（独立空间）","Full screen (separate Space)")),@(TOTK_UI("无边框窗口","Borderless window"))],163);
#ifdef CONTROLS_RESOLUTION
 controls_hint(v,@(TOTK_UI("分辨率与超分档位共同决定 3D 绘制尺寸。窗口大小只改变显示大小。\n例如：1080p + 质量档，3D 以 1280×720 绘制，再由 MetalFX 重建到 1080p。\n“默认”保留 3 键的画面模式；按 3 键也会回到默认。切换档位可能有短暂停顿。","Resolution and upscale preset determine the actual 3D rendering size. Window size only affects presentation.\n1080p + Quality renders at 1280×720 before MetalFX reconstructs 1080p.\nDefault keeps key 3's picture modes; key 3 also returns to it. Changing presets may briefly stall.")),37,100);
#else
 self.resolution.enabled=self.quality.enabled=NO;
 controls_hint(v,@(TOTK_UI("这个版本没有分辨率补丁：目标分辨率与超分档位不可用。","This version has no resolution patch: target resolution and upscaling are unavailable.")),37,100);
#endif
}
-(void)resolutionChanged:(id)sender{(void)sender;
#ifdef CONTROLS_RESOLUTION
 self.quality.enabled=self.resolution.indexOfSelectedItem>0;
#endif
}
-(void)addFrameTab{
 NSView *v=controls_tab(self.tabs,@(TOTK_UI("帧率","FPS")));
 self.vsync=[NSButton checkboxWithTitle:@(TOTK_UI("垂直同步","Vertical sync")) target:nil action:nil];self.vsync.frame=NSMakeRect(20,316,540,28);[v addSubview:self.vsync];
 controls_label(v,@(TOTK_UI("真实帧率上限","Real frame rate limit")),273);
 self.cap=[[NSTextField alloc] initWithFrame:NSMakeRect(218,271,100,27)];[v addSubview:self.cap];
 controls_hint(v,@(TOTK_UI("0 = 不限；可填 15–240 FPS","0 = unlimited; 15–240 FPS")),236,26);
 controls_label(v,@(TOTK_UI("MetalFX 补帧","MetalFX frame generation")),178);
 self.generation=controls_popup(v,@[@(TOTK_UI("关闭","Off")),@(TOTK_UI("自动（按预算在刷新率补帧与 30+30 间切换）","Auto (by the measured budget, the display rate or 30+30)")),
  @(TOTK_UI("补帧至显示刷新率（120 Hz 屏为 60+60）","Up to the display refresh (60+60 at 120 Hz)")),@(TOTK_UI("30+30（游戏 30 帧，补 30 帧）","30+30 (the game at 30, 30 generated)")),
  @(TOTK_UI("不限帧率（能跑多少补多少；不强制垂直同步）","Uncapped (as many as it runs; vsync not forced)"))],174);
 self.generation.target=self;self.generation.action=@selector(generationChanged:);
 controls_hint(v,@(TOTK_UI("补帧关闭时，垂直同步与帧率上限照常生效。\n「补帧至显示刷新率」和「30+30」按显示节拍上屏：垂直同步强制开启（上方置灰），真实帧节奏由档位决定。\n「不限帧率」能跑多少补多少：垂直同步由你决定——开时插值帧逐拍可见，关时可能被新帧盖住。\n自动档先补到刷新率，真实帧连续落后 2 秒降到 30+30，回到预算 5 秒后再试。","With generation off, vsync and the limit work as usual.\n\"Up to the display refresh\" and \"30+30\" pace on the display link: vsync is forced (greyed above) and the tier sets the real-frame cadence.\n\"Uncapped\" generates per real frame at whatever rate it runs; vsync is yours — with it on every generated frame lands on a refresh, with it off a newer frame may cover it.\nAuto starts at the display rate, falls to 30+30 after 2 s over budget and retries after 5 s back inside it.")),22,140);
}
-(void)generationChanged:(id)sender{(void)sender;
 BOOL paced=control_fi_paces((int)self.generation.indexOfSelectedItem),uncapped=nvn_uncapped_hz()!=0;
 self.vsync.enabled=!paced;self.cap.enabled=!paced&&uncapped;
 self.vsync.state=paced||self.shownVsync?NSControlStateValueOn:NSControlStateValueOff;
 self.vsync.toolTip=paced?@(TOTK_UI("此补帧档按显示节拍上屏，垂直同步强制开启；「不限帧率」档不强制。","This tier paces on the display link, so vsync is forced; the uncapped tier does not force it.")):nil;
 self.cap.toolTip=paced?@(TOTK_UI("此补帧档决定真实帧节奏（60+60 或 30+30）；关闭补帧或选「不限帧率」后此上限恢复生效。","This tier sets the real-frame cadence (60+60 or 30+30); the limit applies again with generation off or the uncapped tier.")):
  uncapped?nil:@(TOTK_UI("本次以固定节拍运行（TOTK_UNCAPPED=0），没有可调的上限。","This run uses fixed pacing (TOTK_UNCAPPED=0): there is no limit to set."));
}
-(void)addShortcutTab{
 NSView *v=controls_tab(self.tabs,@(TOTK_UI("快捷键","Shortcuts")));NSMutableArray *buttons=[NSMutableArray new];
 NSArray *titles=@[@(TOTK_UI("打开控制菜单","Open controls")),@(TOTK_UI("显示 / 隐藏提示","Show / hide overlays")),@(TOTK_UI("切换补帧","Cycle frame generation")),@(TOTK_UI("切换画面模式（2K）","Cycle picture mode (2K)")),
  @(TOTK_UI("切换画面比例（支持版本）","Cycle aspect ratio (supported versions)")),@(TOTK_UI("开关时域抗锯齿","Toggle temporal antialiasing")),@(TOTK_UI("切换手柄类型","Cycle controller type")),@(TOTK_UI("开关鼠标模式","Toggle mouse mode")),@(TOTK_UI("切换超分档位","Cycle upscale preset"))];
 for(int i=0;i<CONTROL_SHORTCUTS;i++){CGFloat y=354-i*33;controls_label(v,titles[i],y);NSButton *b=[NSButton buttonWithTitle:@"" target:self action:@selector(recordShortcut:)];b.frame=NSMakeRect(218,y-3,350,30);b.tag=i;[v addSubview:b];[buttons addObject:b];}
 self.hotkeyButtons=buttons;
 controls_hint(v,@(TOTK_UI("点击右侧按钮，再按要绑定的按键或组合键。Esc 取消录入；Delete 清除。\n⌘, 始终可以打开菜单。快捷键改动在点击“应用”后生效。","Click a button and press a key or chord. Esc cancels recording; Delete clears it.\nCommand+, always opens the menu. Shortcut changes take effect on Apply.")),8,52);
}
-(void)addKeyboardTab{
 NSView *tab=controls_tab(self.tabs,@(TOTK_UI("键鼠","Keyboard")));self.keyboardTab=tab;
 controls_hint(tab,@(TOTK_UI("每个动作可设两个按键。点按钮后按键或鼠标；Esc 取消录入，× 清除。鼠标按键在鼠标模式（7 键）下生效。","Two bindings per action. Click, then press a key or mouse button; Esc cancels; × clears. Mouse buttons work in mouse mode (key 7).")),352,35);
 NSArray *titles=controls_target_titles();NSView *rows=controls_scroll(tab,CONTROLS_TARGETS*32+8,NSMakeRect(12,110,576,240));NSMutableArray *buttons=[NSMutableArray new];
 for(int i=0;i<CONTROLS_TARGETS;i++){CGFloat y=(CONTROLS_TARGETS-1-i)*32+4;NSTextField *label=[NSTextField labelWithString:titles[i]];label.frame=NSMakeRect(8,y+4,120,22);[rows addSubview:label];
  for(int j=0;j<2;j++){NSButton *b=[NSButton buttonWithTitle:@"" target:self action:@selector(recordSlot:)];b.tag=i*2+j;b.frame=NSMakeRect(132+j*202,y,160,28);[rows addSubview:b];[buttons addObject:b];
   NSButton *clear=[NSButton buttonWithTitle:@"×" target:self action:@selector(clearSlot:)];clear.tag=b.tag;clear.frame=NSMakeRect(294+j*202,y,32,28);clear.toolTip=@(TOTK_UI("清除此绑定","Clear binding"));[rows addSubview:clear];}}
 self.slotButtons=buttons;
 self.dragLook=[NSButton checkboxWithTitle:@(TOTK_UI("按住鼠标右键拖动视角","Hold right mouse button and drag to look")) target:self action:@selector(keyboardOptionsChanged:)];self.dragLook.frame=NSMakeRect(20,74,540,25);[tab addSubview:self.dragLook];
 self.mouseInvert=[NSButton checkboxWithTitle:@(TOTK_UI("反转鼠标 Y","Invert mouse Y")) target:self action:@selector(keyboardOptionsChanged:)];self.mouseInvert.frame=NSMakeRect(20,44,180,25);[tab addSubview:self.mouseInvert];
 NSTextField *s=[NSTextField labelWithString:@(TOTK_UI("灵敏度","Sensitivity"))];s.frame=NSMakeRect(254,44,85,25);[tab addSubview:s];
 self.sensitivity=[NSSlider sliderWithValue:50 minValue:1 maxValue:100 target:self action:@selector(keyboardOptionsChanged:)];self.sensitivity.frame=NSMakeRect(340,46,190,22);self.sensitivity.toolTip=@(TOTK_UI("鼠标视角灵敏度：1–100（默认 50）","Mouse look sensitivity: 1–100 (default 50)"));[tab addSubview:self.sensitivity];
 self.sensitivityValue=[NSTextField labelWithString:@""];self.sensitivityValue.frame=NSMakeRect(536,44,40,25);[tab addSubview:self.sensitivityValue];
 NSButton *reset=[NSButton buttonWithTitle:@(TOTK_UI("恢复键鼠默认","Reset keyboard/mouse")) target:self action:@selector(resetKeyboard:)];reset.frame=NSMakeRect(20,9,230,28);[tab addSubview:reset];
}
-(void)addControllerTab{
 NSView *tab=controls_tab(self.tabs,@(TOTK_UI("手柄","Controller")));
 NSTextField *pl=[NSTextField labelWithString:@(TOTK_UI("手柄预设","Controller preset"))];pl.frame=NSMakeRect(20,351,165,25);[tab addSubview:pl];
 self.preset=[[NSPopUpButton alloc] initWithFrame:NSMakeRect(188,347,354,30) pullsDown:NO];[self.preset addItemsWithTitles:@[@"Switch",@"Xbox",@"PlayStation",@(TOTK_UI("通用（按位置）","Generic (by position)")),@(TOTK_UI("自定义","Custom"))]];
 self.preset.toolTip=@(TOTK_UI("预设会重置按键、触发模式和摇杆选项；点击应用后生效。","Presets reset buttons, modes and stick options; click Apply to activate."));self.preset.target=self;self.preset.action=@selector(presetChanged:);[tab addSubview:self.preset];
 self.controllerHint=controls_hint(tab,@"",301,42);
 NSView *rows=controls_scroll(tab,CONTROLLER_BUTTONS*34+8,NSMakeRect(12,92,576,180));
 NSArray *headings=@[@(TOTK_UI("实体按钮","Physical button")),@(TOTK_UI("游戏按键","Switch button")),@(TOTK_UI("触发方式","Mode")),@(TOTK_UI("瞄准开火键","Aim fire button"))];
 const CGFloat columns[]={20,200,310,414};for(int i=0;i<4;i++){NSTextField *h=[NSTextField labelWithString:headings[i]];h.frame=NSMakeRect(columns[i],276,i==0?175:130,20);h.font=[NSFont systemFontOfSize:12];h.textColor=NSColor.secondaryLabelColor;[tab addSubview:h];}
 NSArray *names=controls_physical_titles(CONTROLLER_PRESET_CUSTOM);
 NSMutableArray *targets=[NSMutableArray arrayWithObject:@(TOTK_UI("未分配","Unassigned"))];[targets addObjectsFromArray:[controls_target_titles() subarrayWithRange:NSMakeRange(0,16)]];
 NSMutableArray *t=[NSMutableArray new],*m=[NSMutableArray new],*f=[NSMutableArray new],*labels=[NSMutableArray new];
 for(int i=0;i<CONTROLLER_BUTTONS;i++){CGFloat y=(CONTROLLER_BUTTONS-1-i)*34+4;NSTextField *l=[NSTextField labelWithString:names[i]];l.frame=NSMakeRect(8,y+3,180,25);[rows addSubview:l];[labels addObject:l];
  NSPopUpButton *target=[[NSPopUpButton alloc] initWithFrame:NSMakeRect(188,y,106,28) pullsDown:NO];[target addItemsWithTitles:targets];[t addObject:target];
  NSPopUpButton *mode=[[NSPopUpButton alloc] initWithFrame:NSMakeRect(298,y,100,28) pullsDown:NO];[mode addItemsWithTitles:@[@(TOTK_UI("按住","Hold")),@(TOTK_UI("短按","Tap")),@(TOTK_UI("切换","Toggle")),@(TOTK_UI("瞄准","Aim"))]];
  mode.toolTip=@(TOTK_UI("按住：按下期间；短按：每次按下 400 毫秒；切换：按一下开、再按关（摇杆回中时清除）；瞄准：按住瞄准，按开火键射出。","Hold: while down; Tap: 400 ms per press; Toggle: on and off (cleared when the stick centres); Aim: hold to aim, the fire button shoots."));[m addObject:mode];
  NSPopUpButton *fire=[[NSPopUpButton alloc] initWithFrame:NSMakeRect(402,y,140,28) pullsDown:NO];[fire addItemsWithTitles:names];fire.toolTip=@(TOTK_UI("瞄准模式的开火实体按钮","Physical fire button for aim mode"));[f addObject:fire];
  for(NSPopUpButton *b in @[target,mode,fire]){b.tag=i;b.target=self;b.action=@selector(padChanged:);[rows addSubview:b];}}
 self.padTargets=t;self.padModes=m;self.padFires=f;self.padLabels=labels;
 self.swapSticks=[NSButton checkboxWithTitle:@(TOTK_UI("交换左右摇杆","Swap left/right sticks")) target:self action:@selector(controllerOptionsChanged:)];self.swapSticks.frame=NSMakeRect(20,57,270,25);[tab addSubview:self.swapSticks];
 self.padInvert=[NSButton checkboxWithTitle:@(TOTK_UI("反转视角摇杆 Y","Invert look-stick Y")) target:self action:@selector(controllerOptionsChanged:)];self.padInvert.frame=NSMakeRect(300,57,270,25);[tab addSubview:self.padInvert];
 NSButton *reset=[NSButton buttonWithTitle:@(TOTK_UI("恢复手柄默认","Reset controller")) target:self action:@selector(resetController:)];reset.frame=NSMakeRect(20,22,220,28);[tab addSubview:reset];
}
-(void)addActionTab{
 NSView *tab=controls_tab(self.tabs,@(TOTK_UI("动作","Actions")));
 self.actionHint=controls_hint(tab,@"",318,58);
 NSView *rows=controls_scroll(tab,ACTION_BINDING_COUNT*30+8,NSMakeRect(12,103,576,210));NSArray *sources=controls_action_sources(),*titles=controls_action_titles();NSMutableArray *targets=[NSMutableArray new];
 for(unsigned i=0;i<ACTION_BINDING_COUNT;i++){CGFloat y=(ACTION_BINDING_COUNT-1-i)*30+4;NSTextField *l=[NSTextField labelWithString:sources[i]];l.frame=NSMakeRect(8,y+3,140,22);[rows addSubview:l];
  NSPopUpButton *b=[[NSPopUpButton alloc] initWithFrame:NSMakeRect(160,y,360,26) pullsDown:NO];[b addItemsWithTitles:titles];b.tag=i;b.target=self;b.action=@selector(actionChanged:);[rows addSubview:b];[targets addObject:b];}
 self.actionTargets=targets;
 self.actionOriginal=[NSButton buttonWithTitle:@(TOTK_UI("恢复原版动作","Original actions")) target:self action:@selector(resetActions:)];self.actionOriginal.frame=NSMakeRect(20,62,180,30);[tab addSubview:self.actionOriginal];
 self.actionExample=[NSButton buttonWithTitle:@(TOTK_UI("A 跳跃 · X 攻击 · Y 交互","A jump · X attack · Y interact")) target:self action:@selector(exampleActions:)];self.actionExample.frame=NSMakeRect(218,62,350,30);[tab addSubview:self.actionExample];
 controls_hint(tab,@(TOTK_UI("键鼠与手柄共用这组动作；点击应用后生效。\n场景内的动作提示图标暂时仍显示原版按键。","Keyboard/mouse and controllers share these actions; click Apply to activate.\nIn-world prompts still show the original button icons.")),7,49);
}
-(void)addGameButtonTab{
 NSView *tab=controls_tab(self.tabs,@(TOTK_UI("全局","All Buttons")));
 controls_hint(tab,@(TOTK_UI("这层映射在键盘与手柄之后，改的是游戏最终看到的 16 个按键；菜单、对话、地图和场景动作都会跟着变。Plus/Minus 也在这里。Home/截图键由系统处理。只想改场景动作时，请保持此页默认并使用“动作”页。","This mapping runs after the keyboard and controller layouts and changes the 16 buttons the game finally sees, so menus, dialogue, maps and gameplay all follow. Plus/Minus are included; Home/Capture stay system-level. Leave this page at defaults and use Actions for gameplay-only changes.")),318,58);
 NSView *rows=controls_scroll(tab,CONTROLS_GAME_BUTTONS*30+8,NSMakeRect(12,103,576,210));NSArray *names=[controls_target_titles() subarrayWithRange:NSMakeRange(0,CONTROLS_GAME_BUTTONS)];
 NSMutableArray *titles=[NSMutableArray arrayWithObject:@(TOTK_UI("未分配","Unassigned"))];[titles addObjectsFromArray:names];NSMutableArray *targets=[NSMutableArray new];
 for(unsigned i=0;i<CONTROLS_GAME_BUTTONS;i++){CGFloat y=(CONTROLS_GAME_BUTTONS-1-i)*30+4;NSTextField *l=[NSTextField labelWithString:names[i]];l.frame=NSMakeRect(8,y+3,140,22);[rows addSubview:l];
  NSPopUpButton *b=[[NSPopUpButton alloc] initWithFrame:NSMakeRect(160,y,360,26) pullsDown:NO];[b addItemsWithTitles:titles];b.tag=i;b.target=self;b.action=@selector(gameButtonChanged:);[rows addSubview:b];[targets addObject:b];}
 self.gameTargets=targets;
 NSButton *reset=[NSButton buttonWithTitle:@(TOTK_UI("恢复原始按键","Original buttons")) target:self action:@selector(resetGameButtons:)];reset.frame=NSMakeRect(20,62,220,30);[tab addSubview:reset];
 controls_hint(tab,@(TOTK_UI("“未分配”会屏蔽该键。两个来源可以指向同一个游戏按键。\n此页改变后，“动作”页的来源名称指的是游戏最终看到的按键。","Unassigned drops the button. Two sources may select the same game button.\nAfter a change, Actions source names mean the buttons the game finally sees.")),7,49);
}

/* ---- keyboard slots: the first two sources of each target, from layout.keys and the mouse's controller buttons ---- */
-(void)sources:(unsigned)target into:(unsigned[2])out{
 out[0]=out[1]=0;unsigned n=0;
 for(unsigned k=0;k<CONTROLS_KEYS&&n<2;k++)if(layout.keys[k]==target)out[n++]=k+1;
 for(unsigned b=0;b<CONTROLLER_MOUSE_BUTTONS&&n<2&&target<=CONTROLLER_SWITCH_BUTTONS;b++)if(layout.map[CONTROLLER_MOUSE_FIRST+b]==(1u<<(target-1)))out[n++]=CONTROLS_SOURCE_MOUSE+b;
}
-(void)unbindSource:(unsigned)source{
 if(!source)return;
 if(source<CONTROLS_SOURCE_MOUSE)layout.keys[source-1]=0;
 else{unsigned p=CONTROLLER_MOUSE_FIRST+source-CONTROLS_SOURCE_MOUSE;layout.map[p]=0;layout.modes[p]=CONTROLLER_HOLD;}
}
-(void)refreshKeyboard{
 for(unsigned i=0;i<CONTROLS_TARGETS;i++){unsigned s[2];[self sources:i+1 into:s];for(unsigned j=0;j<2;j++)self.slotButtons[i*2+j].title=controls_source_title(s[j]);}
 if(self.recordingSlot>=0)self.slotButtons[self.recordingSlot].title=@(TOTK_UI("按键或鼠标…","Key or mouse…"));
 self.dragLook.state=layout.mouse_drag_look;
 self.mouseInvert.state=layout.mouse_invert_y?layout.mouse_invert_y==2:controls_env_invert();
 self.sensitivity.integerValue=layout.mouse_sensitivity?layout.mouse_sensitivity:controls_env_sensitivity();self.sensitivityValue.integerValue=self.sensitivity.integerValue;
}
-(void)keyboardOptionsChanged:(id)sender{
 layout.mouse_drag_look=self.dragLook.state==NSControlStateValueOn;
 if(sender==self.mouseInvert){BOOL on=self.mouseInvert.state==NSControlStateValueOn;layout.mouse_invert_y=on==controls_env_invert()&&!loaded.mouse_invert_y?0:on?2:1;}
 if(sender==self.sensitivity){int v=(int)self.sensitivity.integerValue;layout.mouse_sensitivity=v==controls_env_sensitivity()&&!loaded.mouse_sensitivity?0:(uint8_t)v;self.sensitivityValue.integerValue=v;}
}
-(void)captureSource:(unsigned)source{
 NSInteger slot=self.recordingSlot;if(slot<0)return;unsigned target=(unsigned)slot/2+1,s[2];[self sources:target into:s];
 if(source<CONTROLS_SOURCE_MOUSE&&controls_key_reserved(source-1)){self.error.stringValue=@(TOTK_UI("1–7 和 Command 键保留给快捷键。","Keys 1–7 and Command are reserved for shortcuts."));[self stopRecording];return;}
 if(source>=CONTROLS_SOURCE_MOUSE&&target>CONTROLLER_SWITCH_BUTTONS){self.error.stringValue=@(TOTK_UI("鼠标按键只能绑定游戏按键，不能绑定移动或视角方向。","Mouse buttons bind game buttons, not movement or look directions."));[self stopRecording];return;}
 NSString *moved=nil;
 if(source<CONTROLS_SOURCE_MOUSE){unsigned was=layout.keys[source-1];if(was&&was!=target)moved=controls_target_titles()[was-1];}
 else{uint16_t was=layout.map[CONTROLLER_MOUSE_FIRST+source-CONTROLS_SOURCE_MOUSE];if(was&&was!=(1u<<(target-1))){unsigned t=0;while(!(was>>t&1))t++;moved=controls_target_titles()[t];}}
 if(s[slot%2]!=source){[self unbindSource:s[slot%2]];[self unbindSource:source];
  if(source<CONTROLS_SOURCE_MOUSE)layout.keys[source-1]=(uint8_t)target;
  else{unsigned p=CONTROLLER_MOUSE_FIRST+source-CONTROLS_SOURCE_MOUSE;layout.map[p]=(uint16_t)(1u<<(target-1));layout.modes[p]=CONTROLLER_HOLD;}}
 self.error.stringValue=moved?[NSString stringWithFormat:@(TOTK_UI("%@ 原来绑定的是「%@」，已移到这里。","%@ was bound to \"%@\"; moved here.")),controls_source_title(source),moved]:@"";
 [self stopRecording];
}
-(void)recordSlot:(NSButton *)button{
 [self stopRecording];self.error.stringValue=@"";self.recordingSlot=button.tag;button.title=@(TOTK_UI("按键或鼠标…","Key or mouse…"));__weak TOTKGameControls *weak=self;
 self.monitor=[NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskKeyDown|NSEventMaskFlagsChanged|NSEventMaskLeftMouseDown|NSEventMaskRightMouseDown|NSEventMaskOtherMouseDown handler:^NSEvent *(NSEvent *e){TOTKGameControls *s=weak;return s?[s captureSlot:e]:e;}];
}
-(NSEvent *)captureSlot:(NSEvent *)e{
 if(self.recordingSlot<0||e.window!=self.window)return e;
 if(e.type==NSEventTypeKeyDown){if(e.isARepeat)return nil;if(e.keyCode==53){[self stopRecording];return nil;}
  if(e.modifierFlags&(NSEventModifierFlagCommand|NSEventModifierFlagControl)){self.error.stringValue=@(TOTK_UI("游戏绑定使用单键；组合键请在“快捷键”页设置。","Use a single game key; configure chords in Shortcuts."));return nil;}
  if(e.keyCode<CONTROLS_KEYS)[self captureSource:e.keyCode+1];return nil;}
 if(e.type==NSEventTypeFlagsChanged){if(controls_modifier_down(e.keyCode,e.modifierFlags))[self captureSource:e.keyCode+1];return nil;}
 /* Tabs, Apply / Cancel and the other controls stay usable while recording: only the slot itself or blank page area captures a click. */
 NSPoint point=[self.keyboardTab convertPoint:e.locationInWindow fromView:nil];
 if(!NSPointInRect(point,self.keyboardTab.bounds)){[self stopRecording];return e;}
 NSView *hit=[self.window.contentView hitTest:[self.window.contentView convertPoint:e.locationInWindow fromView:nil]];
 for(NSView *v=hit;v&&v!=self.keyboardTab;v=v.superview)if([v isKindOfClass:NSControl.class]&&v!=self.slotButtons[self.recordingSlot]){[self stopRecording];return e;}
 if(e.buttonNumber>=0&&e.buttonNumber<CONTROLLER_MOUSE_BUTTONS){[self captureSource:CONTROLS_SOURCE_MOUSE+(unsigned)e.buttonNumber];return nil;}
 return e;
}
-(void)clearSlot:(NSButton *)button{[self stopRecording];unsigned s[2];[self sources:(unsigned)button.tag/2+1 into:s];[self unbindSource:s[button.tag%2]];[self refreshKeyboard];[self refreshController];}
-(void)resetKeyboard:(id)sender{(void)sender;[self stopRecording];ControlsLayout d;controls_layout_default(&d);memcpy(layout.keys,d.keys,sizeof d.keys);
 for(unsigned p=CONTROLLER_MOUSE_FIRST;p<CONTROLLER_BUTTONS;p++){layout.map[p]=d.map[p];layout.modes[p]=d.modes[p];}
 layout.mouse_drag_look=layout.mouse_sensitivity=layout.mouse_invert_y=0;[self refreshKeyboard];[self refreshController];self.error.stringValue=@"";}

/* ---- controller ---- */
-(void)refreshPadLabels{NSInteger preset=self.preset.indexOfSelectedItem;NSArray *names=controls_physical_titles(preset);
 for(NSUInteger i=0;i<self.padLabels.count;i++)self.padLabels[i].stringValue=names[i];
 for(NSPopUpButton *fire in self.padFires)for(NSUInteger j=0;j<names.count;j++)[fire itemAtIndex:j].title=names[j];
 self.controllerHint.stringValue=controls_preset_hint(preset);}
-(void)refreshController{[self refreshPadLabels];
 for(int i=0;i<CONTROLLER_BUTTONS;i++){unsigned t=0;while(t<16&&layout.map[i]!=(1u<<t))t++;
  [self.padTargets[i] selectItemAtIndex:t<16?t+1:0];[self.padModes[i] selectItemAtIndex:controller_mode_kind(layout.modes[i])];[self.padFires[i] selectItemAtIndex:controller_mode_fire(layout.modes[i])];
  self.padModes[i].enabled=layout.map[i]!=0;self.padFires[i].enabled=controller_mode_kind(layout.modes[i])==CONTROLLER_AIM;}
 self.swapSticks.state=layout.swap_sticks;self.padInvert.state=layout.invert_look_y;}
-(void)presetChanged:(id)sender{(void)sender;unsigned preset=(unsigned)self.preset.indexOfSelectedItem;if(preset<CONTROLLER_PRESET_CUSTOM)controls_preset_apply(&layout,preset);[self refreshController];self.error.stringValue=@"";}
-(void)padChanged:(NSPopUpButton *)sender{
 [self.preset selectItemAtIndex:CONTROLLER_PRESET_CUSTOM];
 unsigned i=(unsigned)sender.tag,t=(unsigned)self.padTargets[i].indexOfSelectedItem,k=(unsigned)self.padModes[i].indexOfSelectedItem,fire=(unsigned)self.padFires[i].indexOfSelectedItem;
 layout.map[i]=t?(uint16_t)(1u<<(t-1)):0;if(!t)k=CONTROLLER_HOLD;
 if(k==CONTROLLER_AIM&&fire==i)fire=(i+1)%CONTROLLER_BUTTONS;
 layout.modes[i]=(uint8_t)(k|(k==CONTROLLER_AIM?fire<<3:0));[self refreshController];[self refreshKeyboard];
}
-(void)controllerOptionsChanged:(id)sender{(void)sender;[self.preset selectItemAtIndex:CONTROLLER_PRESET_CUSTOM];[self refreshPadLabels];layout.swap_sticks=self.swapSticks.state==NSControlStateValueOn;layout.invert_look_y=self.padInvert.state==NSControlStateValueOn;}
-(void)resetController:(id)sender{(void)sender;[self.preset selectItemAtIndex:CONTROLLER_PRESET_GENERIC];[self presetChanged:nil];}

/* ---- player actions and the game's buttons ---- */
-(void)refreshActions{
 BOOL supported=runtime_action_bindings_supported();
 for(unsigned i=0;i<ACTION_BINDING_COUNT;i++){[self.actionTargets[i] selectItemAtIndex:action_binding_code_index(actions.target[i])];self.actionTargets[i].enabled=supported;}
 self.actionOriginal.enabled=self.actionExample.enabled=supported;
 self.actionHint.stringValue=!supported?@(TOTK_UI("游戏动作绑定支持已验证的 1.0.0 / 1.4.3 Mac。\n当前版本不支持，或游戏指令校验未通过；保持原版动作。","Player action bindings require the verified 1.0.0 or 1.4.3 Mac runtime.\nThis version is unsupported or instruction guards failed; original actions are retained.")):
  runtime_action_bindings_available()?@(TOTK_UI("自定义场景中的全部 16 个按键（含 +/−）；菜单、对话和快捷栏内的原始操作保持不变。\n长按与松开跟随新按键；全局页会同时修改菜单，通常保持默认即可。","Customize all 16 gameplay buttons, including Plus/Minus. Original menu, dialogue and shortcut controls stay unchanged.\nHeld actions and releases follow the new buttons. Leave All Buttons at defaults to retain menu controls.")):
  @(TOTK_UI("自定义场景中的全部 16 个按键（含 +/−）。首次启用：应用后保存，下次启动生效（TOTK_PLAYER_ACTIONS=0 会禁用）。","Customize all 16 gameplay buttons, including Plus/Minus. First use: Apply saves them and the next start installs them (TOTK_PLAYER_ACTIONS=0 disables this)."));
}
-(void)actionChanged:(NSPopUpButton *)sender{actions.target[sender.tag]=action_binding_code_at((unsigned)sender.indexOfSelectedItem);self.error.stringValue=@"";}
-(void)resetActions:(id)sender{(void)sender;action_bindings_default(&actions);[self refreshActions];self.error.stringValue=@"";}
-(void)exampleActions:(id)sender{(void)sender;action_bindings_xbox(&actions);[self refreshActions];self.error.stringValue=@"";}
-(void)refreshGameButtons{for(unsigned i=0;i<CONTROLS_GAME_BUTTONS;i++){uint8_t t=layout.game[i];[self.gameTargets[i] selectItemAtIndex:t==CONTROLS_GAME_NONE?0:t+1];}}
-(void)gameButtonChanged:(NSPopUpButton *)sender{NSInteger index=sender.indexOfSelectedItem;layout.game[sender.tag]=index==0?CONTROLS_GAME_NONE:(uint8_t)(index-1);self.error.stringValue=@"";}
-(void)resetGameButtons:(id)sender{(void)sender;for(unsigned i=0;i<CONTROLS_GAME_BUTTONS;i++)layout.game[i]=(uint8_t)i;[self refreshGameButtons];self.error.stringValue=@"";}
/* Reads player-actions.txt as the #545 editor did: absent = the runtime's current table. */
-(BOOL)loadActions:(NSError **)error{
 runtime_action_bindings_get(&actions);loadedActions=actions;self.actionsOriginal=nil;NSString *path=actions_path();if(!path)return YES;
 NSError *problem=nil;NSData *data=[NSData dataWithContentsOfFile:path options:0 error:&problem];
 if(!data&&problem.code!=NSFileReadNoSuchFileError){if(error)*error=problem;return NO;}
 if(data){NSString *text=[[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] stringByTrimmingCharactersInSet:NSCharacterSet.newlineCharacterSet];RuntimeActionBindings saved;
  if(!text||data.length>=512||memchr(data.bytes,0,data.length)||!action_bindings_parse(text.UTF8String,&saved)){if(error)*error=controls_error(@(TOTK_UI("player-actions.txt 格式错误，文件未改动。","Invalid player-actions.txt; file left unchanged.")));return NO;}
  if(!runtime_action_bindings_available()){actions=loadedActions=saved;}}   /* saved for the next start: show what will be installed */
 self.actionsOriginal=data;return YES;
}
-(BOOL)saveActions:(NSError **)error{
 if(action_bindings_equal(&actions,&loadedActions))return YES;
 if(!action_bindings_valid(&actions)||!runtime_action_bindings_supported()){if(error)*error=controls_error(@(TOTK_UI("当前运行版本不支持这些游戏动作绑定。","This runtime does not support these player action bindings.")));return NO;}
 NSString *path=actions_path();NSError *problem=nil;NSData *current=[NSData dataWithContentsOfFile:path options:0 error:&problem];
 if(!current&&problem.code!=NSFileReadNoSuchFileError){if(error)*error=problem;return NO;}
 if((current||self.actionsOriginal)&&![current isEqualToData:self.actionsOriginal]){if(error)*error=controls_error(@(TOTK_UI("player-actions.txt 已被其他程序修改，请取消后重新打开。","player-actions.txt changed outside the menu. Cancel and reopen.")));return NO;}
 RuntimeActionBindings previous;runtime_action_bindings_get(&previous);
 if(!runtime_action_bindings_set(&actions)){if(error)*error=controls_error(@(TOTK_UI("版本或指令校验不通过，未保存。","Version/instruction checks failed; nothing saved.")));return NO;}
 NSMutableString *text=[NSMutableString string];for(unsigned i=0;i<ACTION_BINDING_COUNT;i++)[text appendFormat:i?@",%u":@"%u",actions.target[i]];[text appendString:@"\n"];
 NSData *data=[text dataUsingEncoding:NSUTF8StringEncoding];
 if(![data writeToFile:path options:NSDataWritingAtomic error:error]){runtime_action_bindings_set(&previous);return NO;}
 self.actionsOriginal=data;loadedActions=actions;return YES;
}

/* ---- shortcuts ---- */
-(void)stopRecording{
 if(self.monitor){[NSEvent removeMonitor:self.monitor];self.monitor=nil;}self.recordingShortcut=self.recordingSlot=-1;
 for(int i=0;i<CONTROL_SHORTCUTS;i++)self.hotkeyButtons[i].title=hotkey_title(self.draftKeys[i].unsignedLongLongValue);
 [self refreshKeyboard];
}
-(void)recordShortcut:(NSButton *)button{
 [self stopRecording];button.title=@(TOTK_UI("请按快捷键（Esc 取消）","Press a shortcut (Esc cancels)"));NSInteger index=button.tag;self.recordingShortcut=index;__weak TOTKGameControls *weak=self;
 self.monitor=[NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskKeyDown handler:^NSEvent *(NSEvent *e){
  TOTKGameControls *s=weak;if(!s||e.window!=s.window)return e;if(e.isARepeat)return nil;
  if(e.keyCode==53){[s stopRecording];return nil;}
  uint64_t key=e.keyCode==51?0:control_hotkey(e.keyCode,hotkey_mods(e.modifierFlags));
  if(control_hotkey_reserved(key)){s.error.stringValue=@(TOTK_UI("⌘Q 和 ⌘, 是保留快捷键。","Command+Q and Command+, are reserved."));[s stopRecording];return nil;}
  s.draftKeys[index]=@(key);s.error.stringValue=@"";[s stopRecording];return nil;
 }];
}

/* ---- show, refresh, apply ---- */
-(void)tabView:(NSTabView *)tabView willSelectTabViewItem:(NSTabViewItem *)item{(void)tabView;(void)item;[self stopRecording];[self refreshController];[self refreshGameButtons];}
-(void)refreshStatus{
 uint64_t scene=dm_gfx_scene_size(),out=runtime_output_size_current();unsigned sw=(uint32_t)scene,sh=(uint32_t)(scene>>32),ow=(uint32_t)out,oh=(uint32_t)(out>>32);
 NSString *s=sw?[NSString stringWithFormat:@(TOTK_UI("实际 3D %u×%u → %u×%u · MetalFX 已处理 %llu 帧","Actual 3D %u×%u → %u×%u · MetalFX encoded %llu frames")),sw,sh,ow,oh,(unsigned long long)mfxup_done(1)]:@(TOTK_UI("等待游戏画面…","Waiting for the game's picture…"));
 if(renderRestart)s=[s stringByAppendingString:@(TOTK_UI(" · 所选分辨率下次启动生效"," · the chosen resolution applies from the next start"))];
 self.status.stringValue=s;
}
-(void)showSettings:(id)sender{
 (void)sender;if(self.window.sheetParent||displayTransition||textEntryOpen||window.attachedSheet)return;
 display_load();NSError *problem=nil;ControlsLayout read;NSData *original=nil;
 if(!controls_read(&read,&original,&problem)||![self loadActions:&problem]){NSAlert *alert=[NSAlert new];alert.messageText=@(TOTK_UI("无法读取按键布局","Cannot read bindings"));alert.informativeText=problem.localizedDescription;[alert beginSheetModalForWindow:window completionHandler:nil];return;}
 layout=loaded=read;self.controlsOriginal=original;
 int r=controls_saved_int(@"resolution",0,CONTROL_SIZES-1,-1);
 self.shownResolution=r+1;self.shownQuality=controls_saved_int(@"upscalePreset",0,CONTROL_QUALITIES-1,1);
#ifndef CONTROLS_RESOLUTION
 self.shownResolution=0;
#endif
 [self.resolution selectItemAtIndex:self.shownResolution];[self.quality selectItemAtIndex:self.shownQuality];[self resolutionChanged:nil];
 self.shownAA=!!taa_enabled();[self.aa selectItemAtIndex:self.shownAA];
 self.shownMode=display_live_mode();[self.mode selectItemAtIndex:self.shownMode];
 self.shownVsync=dm_gfx_vsync();self.vsync.state=self.shownVsync?NSControlStateValueOn:NSControlStateValueOff;
 { int hz=nvn_uncapped_hz(),cap=controls_saved_int(@"nativeFrameCap",0,CONTROL_CAP_MAX,-1);
   self.shownCap=control_cap_valid(cap)?cap:hz>=CONTROL_CAP_MAX?0:hz>0?hz:0;self.cap.integerValue=self.shownCap; }
 self.shownFI=control_fi_tier(mfx_enabled(),nvn_fi_auto_on());[self.generation selectItemAtIndex:self.shownFI];[self generationChanged:nil];
 self.draftKeys=[NSMutableArray new];for(int i=0;i<CONTROL_SHORTCUTS;i++)[self.draftKeys addObject:@(atomic_load(&controlKeys[i]))];
 NSInteger preset=controls_saved_int(@"controllerPreset",0,CONTROLLER_PRESET_CUSTOM,CONTROLLER_PRESET_CUSTOM);
 if(preset==CONTROLLER_PRESET_CUSTOM||!controls_preset_matches(&layout,(unsigned)preset)){   /* Switch and Generic are the same layout: the saved name tells them apart */
  static const unsigned order[]={CONTROLLER_PRESET_GENERIC,CONTROLLER_PRESET_XBOX,CONTROLLER_PRESET_PLAYSTATION};preset=CONTROLLER_PRESET_CUSTOM;
  for(unsigned p=0;p<3;p++)if(controls_preset_matches(&layout,order[p])){preset=order[p];break;}}
 [self.preset selectItemAtIndex:preset];
 [self stopRecording];[self refreshController];[self refreshActions];[self refreshGameButtons];[self refreshStatus];self.error.stringValue=@"";
 self.statusTimer=[NSTimer scheduledTimerWithTimeInterval:0.5 target:self selector:@selector(refreshStatus) userInfo:nil repeats:YES];
 controlsOpen=YES;runtime_input_suspend(1);front_mouse_update();[window beginSheet:self.window completionHandler:nil];
}
-(BOOL)windowShouldClose:(NSWindow *)sender{(void)sender;[self cancelSettings:nil];return NO;}
-(void)cancelSettings:(id)sender{
 (void)sender;[self stopRecording];[self.statusTimer invalidate];self.statusTimer=nil;
 if(self.window.sheetParent)[window endSheet:self.window];[self.window orderOut:nil];
 controlsOpen=NO;runtime_input_suspend(0);front_mouse_update();
}
-(void)applySettings:(id)sender{
 (void)sender;[self stopRecording];
 NSString *text=[self.cap.stringValue stringByTrimmingCharactersInSet:NSCharacterSet.whitespaceAndNewlineCharacterSet];NSScanner *scan=[NSScanner scannerWithString:text];int cap=0;
 if(![scan scanInt:&cap]||!scan.isAtEnd||!control_cap_valid(cap)){self.error.stringValue=@(TOTK_UI("帧率上限应为 0 或 15–240。","Frame limit must be 0 or 15–240."));return;}
 uint64_t keys[CONTROL_SHORTCUTS];for(int i=0;i<CONTROL_SHORTCUTS;i++)keys[i]=self.draftKeys[i].unsignedLongLongValue;
 int bad=control_shortcuts_check(keys,layout.keys);
 if(bad){unsigned i=(unsigned)bad-1;BOOL game=control_hotkey_conflicts(keys[i],layout.keys);
  self.error.stringValue=[NSString stringWithFormat:game?@(TOTK_UI("快捷键 %@ 已被游戏按键绑定使用，请更换其中一个。","Shortcut %@ is also a game binding; change one of them.")):@(TOTK_UI("快捷键 %@ 重复，请更换后应用。","Shortcut %@ is used twice; choose another binding.")),hotkey_title(keys[i])];return;}
 if(!controls_layout_valid(&layout)){self.error.stringValue=@(TOTK_UI("手柄瞄准的开火按钮不能是它自身。","An aim binding needs a different physical fire button."));return;}
 NSError *problem=nil;
 if(!controls_layout_same(&layout,&loaded)){NSData *original=self.controlsOriginal;
  if(!controls_write(&layout,&original,&problem)){self.error.stringValue=problem.localizedDescription;return;}
  self.controlsOriginal=original;loaded=layout;runtime_keyboard_layout_set(layout.keys);runtime_controller_layout_set(&layout);runtime_game_buttons_set(layout.game);controlsDragLook=layout.mouse_drag_look;}
 if(![self saveActions:&problem]){self.error.stringValue=problem.localizedDescription;return;}
 [displayDefaults setInteger:self.preset.indexOfSelectedItem forKey:@"controllerPreset"];
 for(int i=0;i<CONTROL_SHORTCUTS;i++)atomic_store(&controlKeys[i],keys[i]);
 { BOOL same=YES;for(int i=0;i<CONTROL_SHORTCUTS;i++)same&=keys[i]==control_shortcut_default((unsigned)i);
   if(same)[displayDefaults removeObjectForKey:@"controlShortcuts"];
   else{NSMutableArray *a=[NSMutableArray new];for(int i=0;i<CONTROL_SHORTCUTS;i++)[a addObject:@(keys[i])];[displayDefaults setObject:a forKey:@"controlShortcuts"];} }
 /* Only what the player changed is saved and applied; the rest keeps whatever is in force. */
#ifdef CONTROLS_RESOLUTION
 NSInteger res=self.resolution.indexOfSelectedItem,q=self.quality.indexOfSelectedItem;
 if(res!=self.shownResolution||(res&&q!=self.shownQuality)){
  if(!res){[displayDefaults removeObjectForKey:@"resolution"];[displayDefaults removeObjectForKey:@"upscalePreset"];renderRestart=YES;}
  else{[displayDefaults setInteger:res-1 forKey:@"resolution"];[displayDefaults setInteger:q forKey:@"upscalePreset"];controls_render_apply();}
 }
#endif
 NSInteger aa=self.aa.indexOfSelectedItem;if(aa!=self.shownAA){[displayDefaults setBool:aa==1 forKey:@"taa"];if((aa==1)!=!!taa_enabled())taa_toggle();}
 NSInteger mode=self.mode.indexOfSelectedItem;if(mode!=self.shownMode){[displayDefaults setInteger:mode forKey:@"mode"];displayMode=(int)mode;}
 NSInteger fi=self.generation.indexOfSelectedItem;if(fi!=self.shownFI){[displayDefaults setInteger:fi forKey:@"frameGeneration"];controls_fi_apply((int)fi);}
 if(self.vsync.enabled){BOOL vs=self.vsync.state==NSControlStateValueOn;if(vs!=self.shownVsync){[displayDefaults setBool:vs forKey:@"vsync"];dm_gfx_set_flag("vsync",vs);}}
 if(self.cap.enabled&&cap!=self.shownCap){[displayDefaults setInteger:cap forKey:@"nativeFrameCap"];dm_gfx_set_flag("uncapped_hz",control_cap_hz(cap));}
 [self cancelSettings:nil];if(mode!=self.shownMode)display_apply_mode();
}
-(void)dealloc{if(_monitor)[NSEvent removeMonitor:_monitor];}
@end

/* ---- shortcut dispatch (TOTKInputView keyDown): the keys' own handlers, through the synthetic key path ---- */
static void controls_action(unsigned action){
 static const unsigned key[CONTROL_SHORTCUTS]={0,18,19,20,21,23,22,26,0};   /* input.c's fixed keys for actions 1-7 */
 if(action==CONTROL_SHORTCUT_MENU){if(gameControls.window.sheetParent)[gameControls cancelSettings:nil];else [gameControls showSettings:nil];return;}
 if(action==CONTROL_SHORTCUT_PRESET){
#ifdef CONTROLS_RESOLUTION
  if(controls_saved(@"resolution")){int q=(controls_saved_int(@"upscalePreset",0,CONTROL_QUALITIES-1,1)+1)%CONTROL_QUALITIES;[displayDefaults setInteger:q forKey:@"upscalePreset"];controls_render_apply();fprintf(stderr,"[totk-controls] upscale preset -> %d\n",q);}
  else
#endif
  fprintf(stderr,"[totk-controls] the upscale preset needs a target resolution (control menu, Video)\n");
  return;
 }
 if(action==CONTROL_SHORTCUT_PICTURE){[displayDefaults removeObjectForKey:@"resolution"];[displayDefaults removeObjectForKey:@"upscalePreset"];renderRestart=NO;}   /* key 3 takes the picture back */
 if(action==CONTROL_SHORTCUT_INTERP)[displayDefaults removeObjectForKey:@"frameGeneration"];   /* the key's cycle replaces the menu's tier */
 if(action==CONTROL_SHORTCUT_TAA)[displayDefaults removeObjectForKey:@"taa"];
 runtime_input_key_synthetic(key[action],1);runtime_input_key_synthetic(key[action],0);
}
static int runtime_controls_handle_key(unsigned code,NSEventModifierFlags flags,BOOL repeat){
 if(!controlsLoaded)return 0;uint64_t key=control_hotkey(code,hotkey_mods(flags));
 for(unsigned i=0;i<CONTROL_SHORTCUTS;i++)if(key&&atomic_load(&controlKeys[i])==key){if(!repeat)controls_action(i);return 1;}
 return 0;
}
