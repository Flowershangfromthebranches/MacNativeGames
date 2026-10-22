/* macOS binding editor for #42's control sheet. Drafts never reach the runtime
   until Apply. The event monitor is scoped to this sheet and removed on every exit. */
#pragma once
#include "controller_presets.h"
static NSArray<NSString *> *input_action_titles(void){return @[@"A",@"B",@"X",@"Y",@"L3",@"R3",@"L",@"R",@"ZL",@"ZR",@"+",@"−",@(TOTK_UI("十字键 ←","D-pad ←")),@(TOTK_UI("十字键 ↑","D-pad ↑")),@(TOTK_UI("十字键 →","D-pad →")),@(TOTK_UI("十字键 ↓","D-pad ↓")),@(TOTK_UI("移动 ←","Move ←")),@(TOTK_UI("移动 →","Move →")),@(TOTK_UI("移动 ↑","Move ↑")),@(TOTK_UI("移动 ↓","Move ↓")),@(TOTK_UI("视角 ←","Look ←")),@(TOTK_UI("视角 →","Look →")),@(TOTK_UI("视角 ↑","Look ↑")),@(TOTK_UI("视角 ↓","Look ↓"))];}
static NSString *input_source_title(unsigned source){
 if(!source)return @(TOTK_UI("未分配","Unassigned"));
 if(source>INPUT_KEYS){NSArray *mouse=@[@(TOTK_UI("鼠标左键","Mouse left")),@(TOTK_UI("鼠标右键","Mouse right")),@(TOTK_UI("鼠标中键","Mouse middle"))];unsigned n=source-INPUT_KEYS-1;return n<mouse.count?mouse[n]:[NSString stringWithFormat:@(TOTK_UI("鼠标 %u","Mouse %u")),n+1];}
 NSDictionary *special=@{@37:@"Return",@54:@"Esc",@124:@"←",@125:@"→",@127:@"↑",@126:@"↓",@57:@(TOTK_UI("左 Shift","Left Shift")),@59:@(TOTK_UI("左 Option","Left Option")),@61:@(TOTK_UI("右 Shift","Right Shift")),@62:@(TOTK_UI("右 Option","Right Option")),@60:@(TOTK_UI("左 Control","Left Control")),@63:@(TOTK_UI("右 Control","Right Control")),@56:@(TOTK_UI("左 Command","Left Command")),@55:@(TOTK_UI("右 Command","Right Command"))};
 if(special[@(source)])return special[@(source)];
 return hotkey_title(source);
}
static BOOL input_number(id value,long lo,long hi){if(![value isKindOfClass:NSNumber.class])return NO;double v=[value doubleValue];return isfinite(v)&&v>=lo&&v<=hi&&floor(v)==v;}
static NSArray *input_keyboard_values(const RuntimeKeyboardBindings *b){NSMutableArray *a=[NSMutableArray new];for(int i=0;i<INPUT_ACTIONS;i++)for(int j=0;j<2;j++)[a addObject:@(b->source[i][j])];[a addObjectsFromArray:@[@(b->mouse_look),@(b->mouse_sensitivity),@(b->mouse_invert_y)]];return a;}
static BOOL input_keyboard_decode(id values,RuntimeKeyboardBindings *b){
 if(![values isKindOfClass:NSArray.class]||[values count]!=INPUT_ACTIONS*2+3)return NO;
 RuntimeKeyboardBindings next={0};for(int i=0;i<INPUT_ACTIONS*2;i++){if(!input_number(values[i],0,INPUT_SOURCES))return NO;next.source[i/2][i%2]=[values[i] unsignedIntValue];}
 if(!input_number(values[48],0,1)||!input_number(values[49],10,500)||!input_number(values[50],0,1))return NO;
 next.mouse_look=[values[48] intValue];next.mouse_sensitivity=[values[49] intValue];next.mouse_invert_y=[values[50] intValue];if(!input_bindings_valid(&next))return NO;*b=next;return YES;
}
static NSArray *input_controller_values(const RuntimeControllerBindings *b){NSMutableArray *a=[NSMutableArray new];for(int i=0;i<CONTROLLER_BUTTONS;i++)[a addObject:@(b->map[i])];for(int i=0;i<CONTROLLER_BUTTONS;i++)[a addObject:@(b->modes[i])];[a addObjectsFromArray:@[@(b->swap_sticks),@(b->invert_look_y)]];return a;}
static BOOL input_controller_decode(id values,RuntimeControllerBindings *b){
 if(![values isKindOfClass:NSArray.class]||[values count]!=CONTROLLER_BUTTONS*2+2)return NO;RuntimeControllerBindings next={0};
 for(int i=0;i<CONTROLLER_BUTTONS;i++){if(!input_number(values[i],0,65535)||!input_number(values[CONTROLLER_BUTTONS+i],0,255))return NO;next.map[i]=[values[i] unsignedIntValue];next.modes[i]=[values[CONTROLLER_BUTTONS+i] unsignedIntValue];}
 if(!input_number(values[34],0,1)||!input_number(values[35],0,1))return NO;next.swap_sticks=[values[34] intValue];next.invert_look_y=[values[35] intValue];if(!controller_bindings_valid(&next))return NO;*b=next;return YES;
}
static NSArray<NSString *> *input_controller_names(NSInteger preset){
 NSArray *faces=preset==CONTROLLER_PRESET_SWITCH?@[@"下 / B",@"右 / A",@"左 / Y",@"上 / X"]:
 preset==CONTROLLER_PRESET_XBOX?@[@"下 / A",@"右 / B",@"左 / X",@"上 / Y"]:
 preset==CONTROLLER_PRESET_PLAYSTATION?@[@"下 / ×",@"右 / ○",@"左 / □",@"上 / △"]:
 @[@"下 / B · A · ×",@"右 / A · B · ○",@"左 / Y · X · □",@"上 / X · Y · △"];
 if(!totk_ui_zh()){faces=preset==CONTROLLER_PRESET_SWITCH?@[@"South / B",@"East / A",@"West / Y",@"North / X"]:
 preset==CONTROLLER_PRESET_XBOX?@[@"South / A",@"East / B",@"West / X",@"North / Y"]:
 preset==CONTROLLER_PRESET_PLAYSTATION?@[@"South / ×",@"East / ○",@"West / □",@"North / △"]:
 @[@"South / B · A · ×",@"East / A · B · ○",@"West / Y · X · □",@"North / X · Y · △"];}
 NSMutableArray *names=[faces mutableCopy];
 NSArray *rest=preset==CONTROLLER_PRESET_SWITCH?@[@"L3",@"R3",@"L",@"R",@"ZL",@"ZR",@"+",@"−"]:
 preset==CONTROLLER_PRESET_XBOX?@[@"L3",@"R3",@"LB",@"RB",@"LT",@"RT",@"Menu",@"View"]:
 preset==CONTROLLER_PRESET_PLAYSTATION?@[@"L3",@"R3",@"L1",@"R1",@"L2",@"R2",@"Options",@"Share / Create"]:
 @[@"L3",@"R3",@"L / LB / L1",@"R / RB / R1",@"ZL / LT / L2",@"ZR / RT / R2",@"+ / Menu / Options",@"− / View / Create"];
 [names addObjectsFromArray:rest];[names addObjectsFromArray:@[@"←",@"↑",@"→",@"↓",@(TOTK_UI("触控板按下","Touchpad click"))]];return names;
}
static NSString *input_controller_hint(NSInteger preset){
 switch(preset){
 case CONTROLLER_PRESET_SWITCH:return @(TOTK_UI("Switch：A 确认、B 返回。场景动作请在“游戏动作”页单独设置。","Switch: A confirms, B returns. Configure gameplay separately in Player Actions."));
 case CONTROLLER_PRESET_XBOX:return @(TOTK_UI("Xbox：A 确认、B 返回。场景动作请在“游戏动作”页单独设置。","Xbox: A confirms, B returns. Configure gameplay separately in Player Actions."));
 case CONTROLLER_PRESET_PLAYSTATION:return @(TOTK_UI("PlayStation：× 确认、○ 返回。场景动作请在“游戏动作”页单独设置。","PlayStation: × confirms, ○ returns. Configure gameplay separately in Player Actions."));
 default:return @(TOTK_UI("这里修改菜单也使用的按键映射；场景动作请到“游戏动作”页设置。预设会重置模式与摇杆选项。","These bindings also affect menus; use Player Actions for gameplay-only changes. Presets reset modes and sticks."));
 }
}
static NSArray *input_player_action_titles(void){
 return @[@(TOTK_UI("未分配","Unassigned")),@(TOTK_UI("交互","Interact")),@(TOTK_UI("取消 / 奔跑","Cancel / run")),@(TOTK_UI("跳跃","Jump")),@(TOTK_UI("攻击","Attack")),@(TOTK_UI("能力","Ability")),@(TOTK_UI("投掷武器","Throw weapon")),@(TOTK_UI("盾 / 瞄准","Shield / aim")),@(TOTK_UI("弓","Bow")),@(TOTK_UI("快捷选择：上","Quick select: up")),@(TOTK_UI("快捷选择：下","Quick select: down")),@(TOTK_UI("快捷选择：左","Quick select: left")),@(TOTK_UI("快捷选择：右","Quick select: right")),@(TOTK_UI("下蹲","Crouch")),@(TOTK_UI("望远镜 / 右摇杆","Scope / R-stick")),@(TOTK_UI("暂停 / 背包","Pause / inventory")),@(TOTK_UI("地图","Map"))];
}
static NSArray *input_player_action_source_titles(void){
 return @[@"A",@"B",@"X",@"Y",@"L",@"R",@"ZL",@"ZR",@(TOTK_UI("上","Up")),@(TOTK_UI("下","Down")),@(TOTK_UI("左","Left")),@(TOTK_UI("右","Right")),@(TOTK_UI("左摇杆按下","L-stick click")),@(TOTK_UI("右摇杆按下","R-stick click")),@"+",@"−"];
}
static NSArray *input_player_action_values(const RuntimeActionBindings *b){
 NSMutableArray *a=[NSMutableArray arrayWithCapacity:ACTION_BINDING_COUNT];
 for(unsigned i=0;i<ACTION_BINDING_COUNT;i++)[a addObject:@(b->target[i])];
 return a;
}
static BOOL input_player_action_decode(id values,RuntimeActionBindings *b){
 if(![values isKindOfClass:NSArray.class]||[values count]!=ACTION_BINDING_COUNT)return NO;RuntimeActionBindings next;
 for(unsigned i=0;i<ACTION_BINDING_COUNT;i++){if(!input_number(values[i],0,0xffff))return NO;unsigned v=[values[i] unsignedIntValue];if(!action_binding_code_valid((uint16_t)v))return NO;next.target[i]=(uint16_t)v;}
 if(!action_bindings_valid(&next))return NO;*b=next;return YES;
}
static NSArray *input_button_values(const RuntimeButtonBindings *b){NSMutableArray *a=[NSMutableArray arrayWithCapacity:BUTTON_BINDING_COUNT];for(unsigned i=0;i<BUTTON_BINDING_COUNT;i++)[a addObject:@(b->target[i])];return a;}
static BOOL input_button_decode(id values,RuntimeButtonBindings *b){
 if(![values isKindOfClass:NSArray.class]||[values count]!=BUTTON_BINDING_COUNT)return NO;RuntimeButtonBindings next;
 for(unsigned i=0;i<BUTTON_BINDING_COUNT;i++){if(!input_number(values[i],0,BUTTON_BINDING_NONE))return NO;unsigned v=[values[i] unsignedIntValue];if(v>=BUTTON_BINDING_COUNT&&v!=BUTTON_BINDING_NONE)return NO;next.target[i]=(uint8_t)v;}
 if(!button_bindings_valid(&next))return NO;*b=next;return YES;
}
static void display_input_load(void){
 RuntimeKeyboardBindings k;RuntimeControllerBindings c;RuntimeActionBindings a;
 id saved=[displayDefaults objectForKey:@"playerActionBindingsV3"];
 if(input_player_action_decode(saved,&a)){if(!runtime_action_bindings_set(&a))fprintf(stderr,"[totk] saved player actions unsupported by this runtime; original actions retained\n");}
 else if(!saved){
  id previous=[displayDefaults objectForKey:@"playerActionBindingsV2"];
  if([previous isKindOfClass:NSArray.class]&&[previous count]==14){
   NSMutableArray *expanded=[previous mutableCopy];[expanded addObjectsFromArray:@[@(ACTION_PAUSE),@(ACTION_MAP)]];
   if(input_player_action_decode(expanded,&a))runtime_action_bindings_set(&a);
  }else if(!previous){
  id old=[displayDefaults objectForKey:@"playerActionBindingsV1"];
  if([old isKindOfClass:NSArray.class]&&[old count]==3){
   static const unsigned map[3]={0,2,3};action_bindings_default(&a);BOOL valid=YES;
   for(unsigned i=0;i<3;i++){
    if(!input_number(old[i],0,8)){valid=NO;break;}unsigned v=[old[i] unsignedIntValue];
    if(v!=0&&v!=ACTION_INTERACT&&v!=ACTION_JUMP&&v!=ACTION_ATTACK){valid=NO;break;}
    a.target[map[i]]=(uint16_t)v;
   }
   if(valid&&!runtime_action_bindings_set(&a))fprintf(stderr,"[totk] saved player actions unsupported by this runtime; original actions retained\n");
  }
 }
 }
 RuntimeButtonBindings buttons;
 if(input_button_decode([displayDefaults objectForKey:@"allButtonBindingsV1"],&buttons))runtime_button_bindings_set(&buttons);
 if(input_keyboard_decode([displayDefaults objectForKey:@"keyboardMouseBindingsV1"],&k))runtime_input_bindings_set(&k);
 if(input_controller_decode([displayDefaults objectForKey:@"controllerBindingsV1"],&c))runtime_gamepad_bindings_set(&c);
}
@interface TOTKInputSettings:NSObject
@property RuntimeKeyboardBindings keyboard;
@property RuntimeControllerBindings controller;
@property RuntimeActionBindings playerActions;
@property RuntimeButtonBindings gameButtons;
@property(strong) NSArray<NSPopUpButton *> *actionTargets,*buttonTargets;
@property(strong) NSTextField *actionHint;
@property(strong) NSButton *actionOriginal,*actionExample;
@property(weak) NSWindow *hostWindow;
@property(weak) NSView *keyboardTab;
@property(weak) NSTextField *error;
@property(strong) NSArray<NSButton *> *keyButtons;
@property(strong) NSArray<NSPopUpButton *> *targets,*modes,*fires;
@property(strong) NSPopUpButton *preset;
@property(strong) NSArray<NSTextField *> *controllerLabels;
@property(strong) NSTextField *controllerHint;
@property(strong) NSButton *mouseLook,*mouseInvert,*swapSticks,*padInvert;
@property(strong) NSSlider *sensitivity;
@property(strong) id monitor;
@property NSInteger recording;
-(void)addKeyboardTab:(NSTabView *)tabs;
-(void)addControllerTab:(NSTabView *)tabs;
-(void)addActionTab:(NSTabView *)tabs;
-(void)addAllButtonsTab:(NSTabView *)tabs;
-(void)refreshAllButtons;
-(void)beginDraft;
-(void)stopRecording;
-(NSEvent *)captureEvent:(NSEvent *)event;
-(BOOL)validate:(NSArray<NSNumber *> *)shortcuts;
-(BOOL)apply;
@end
static NSView *input_scroll(NSView *parent,CGFloat height,CGFloat top){NSScrollView *scroll=[[NSScrollView alloc] initWithFrame:NSMakeRect(12,top,576,350-top)];scroll.hasVerticalScroller=YES;scroll.borderType=NSBezelBorder;NSView *content=[[NSView alloc] initWithFrame:NSMakeRect(0,0,550,height)];scroll.documentView=content;[parent addSubview:scroll];[content scrollPoint:NSMakePoint(0,height)];return content;}
@implementation TOTKInputSettings
-(void)keyboardOptionsChanged:(id)sender{(void)sender;_keyboard.mouse_look=self.mouseLook.state==NSControlStateValueOn;_keyboard.mouse_invert_y=self.mouseInvert.state==NSControlStateValueOn;_keyboard.mouse_sensitivity=(int)self.sensitivity.integerValue;}
-(void)controllerOptionsChanged:(id)sender{(void)sender;[self.preset selectItemAtIndex:CONTROLLER_PRESET_CUSTOM];[self refreshControllerLabels];_controller.swap_sticks=self.swapSticks.state==NSControlStateValueOn;_controller.invert_look_y=self.padInvert.state==NSControlStateValueOn;}
-(void)refreshKeyboard{for(int i=0;i<INPUT_ACTIONS*2;i++)self.keyButtons[i].title=input_source_title(_keyboard.source[i/2][i%2]);self.mouseLook.state=_keyboard.mouse_look;self.mouseInvert.state=_keyboard.mouse_invert_y;self.sensitivity.integerValue=_keyboard.mouse_sensitivity;}
-(void)refreshControllerLabels{
 NSInteger preset=self.preset.indexOfSelectedItem;NSArray *names=input_controller_names(preset);
 for(NSUInteger i=0;i<self.controllerLabels.count;i++)self.controllerLabels[i].stringValue=names[i];
 for(NSPopUpButton *fire in self.fires)for(NSUInteger j=0;j<names.count;j++)[fire itemAtIndex:j].title=names[j];
 self.controllerHint.stringValue=input_controller_hint(preset);
}
-(void)presetChanged:(id)sender{(void)sender;unsigned preset=(unsigned)self.preset.indexOfSelectedItem;
 if(preset<CONTROLLER_PRESET_CUSTOM)controller_preset_apply(&_controller,preset);
 [self refreshController];self.error.stringValue=@"";
}
-(void)refreshController{[self refreshControllerLabels];for(int i=0;i<CONTROLLER_BUTTONS;i++){
 unsigned map=_controller.map[i];int target=0;while(map&&!(map&1)){target++;map>>=1;}
 [self.targets[i] selectItemAtIndex:_controller.map[i]?target+1:0];[self.modes[i] selectItemAtIndex:controller_mode_kind(_controller.modes[i])];[self.fires[i] selectItemAtIndex:controller_mode_fire(_controller.modes[i])];self.fires[i].enabled=controller_mode_kind(_controller.modes[i])==CONTROLLER_AIM;
 }self.swapSticks.state=_controller.swap_sticks;self.padInvert.state=_controller.invert_look_y;}
-(void)stopRecording{if(self.monitor){[NSEvent removeMonitor:self.monitor];self.monitor=nil;}self.recording=-1;[self refreshKeyboard];}
-(void)captureSource:(unsigned)source{if(self.recording<0)return;_keyboard.source[self.recording/2][self.recording%2]=(uint16_t)source;[self stopRecording];}
-(void)record:(NSButton *)button{
 [self stopRecording];self.error.stringValue=@"";self.recording=button.tag;button.title=@(TOTK_UI("按键或鼠标…","Key or mouse…"));__weak TOTKInputSettings *weak=self;
 self.monitor=[NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskKeyDown|NSEventMaskFlagsChanged|NSEventMaskLeftMouseDown|NSEventMaskRightMouseDown|NSEventMaskOtherMouseDown handler:^NSEvent *(NSEvent *e){TOTKInputSettings *s=weak;return s?[s captureEvent:e]:e;}];
}
-(NSEvent *)captureEvent:(NSEvent *)e{
 if(self.recording<0||e.window!=self.hostWindow)return e;
 if(e.type==NSEventTypeKeyDown){if(e.isARepeat)return nil;if(e.keyCode==53){[self stopRecording];return nil;}
  if(e.modifierFlags&(NSEventModifierFlagCommand|NSEventModifierFlagControl)){self.error.stringValue=@(TOTK_UI("游戏绑定使用单键；组合键请在“快捷键”页设置。","Use a single game key; configure chords in Shortcuts."));return nil;}
  [self captureSource:e.keyCode+1];return nil;}
 if(e.type==NSEventTypeFlagsChanged){if(front_modifier_down(e.keyCode,e.modifierFlags))[self captureSource:e.keyCode+1];return nil;}
 // Keep tab navigation, Apply/Cancel, clear buttons and other controls usable
 // while recording. Only the selected binding or blank page area captures a click.
 NSPoint point=[self.keyboardTab convertPoint:e.locationInWindow fromView:nil];
 if(!NSPointInRect(point,self.keyboardTab.bounds)){[self stopRecording];return e;}
 NSView *hit=[self.hostWindow.contentView hitTest:[self.hostWindow.contentView convertPoint:e.locationInWindow fromView:nil]];
 for(NSView *v=hit;v&&v!=self.keyboardTab;v=v.superview)if([v isKindOfClass:NSControl.class]&&v!=self.keyButtons[self.recording]){[self stopRecording];return e;}
 if(e.buttonNumber>=0&&e.buttonNumber<INPUT_MOUSE_BUTTONS){[self captureSource:INPUT_KEYS+1+(unsigned)e.buttonNumber];return nil;}return e;
}

-(void)clear:(NSButton *)button{[self stopRecording];_keyboard.source[button.tag/2][button.tag%2]=0;[self refreshKeyboard];}
-(void)resetKeyboard:(id)sender{(void)sender;[self stopRecording];input_bindings_default(&_keyboard);[self refreshKeyboard];self.error.stringValue=@"";}
-(void)resetController:(id)sender{(void)sender;[self.preset selectItemAtIndex:CONTROLLER_PRESET_GENERIC];[self presetChanged:nil];}
-(void)controllerChanged:(NSPopUpButton *)sender{[self.preset selectItemAtIndex:CONTROLLER_PRESET_CUSTOM];[self refreshControllerLabels];int i=(int)sender.tag;NSInteger t=self.targets[i].indexOfSelectedItem,kind=self.modes[i].indexOfSelectedItem;_controller.map[i]=t?1u<<(t-1):0;if(!t){kind=CONTROLLER_HOLD;[self.modes[i] selectItemAtIndex:kind];}
 _controller.modes[i]=(uint8_t)(kind|(kind==CONTROLLER_AIM?self.fires[i].indexOfSelectedItem<<3:0));self.fires[i].enabled=kind==CONTROLLER_AIM;
}
-(void)addKeyboardTab:(NSTabView *)tabs{
 NSView *tab=controls_tab(tabs,@(TOTK_UI("键鼠","Keyboard")));self.keyboardTab=tab;
 controls_hint(tab,@(TOTK_UI("每个动作可设两个按键。点按钮后按键或鼠标；Esc 取消录入，× 清除。","Two bindings per action. Click, then press a key or mouse button; Esc cancels; × clears.")),352,35);
 NSView *rows=input_scroll(tab,INPUT_ACTIONS*32+8,110);NSMutableArray *buttons=[NSMutableArray new];NSArray *titles=input_action_titles();
 for(int i=0;i<INPUT_ACTIONS;i++){CGFloat y=(INPUT_ACTIONS-1-i)*32+4;NSTextField *label=[NSTextField labelWithString:titles[i]];label.frame=NSMakeRect(8,y+4,120,22);[rows addSubview:label];
  for(int j=0;j<2;j++){NSButton *b=[NSButton buttonWithTitle:@"" target:self action:@selector(record:)];b.tag=i*2+j;b.frame=NSMakeRect(132+j*202,y,160,28);[rows addSubview:b];[buttons addObject:b];NSButton *clear=[NSButton buttonWithTitle:@"×" target:self action:@selector(clear:)];clear.tag=b.tag;clear.frame=NSMakeRect(294+j*202,y,32,28);clear.toolTip=@(TOTK_UI("清除此绑定","Clear binding"));[rows addSubview:clear];}}
 self.keyButtons=buttons;
 self.mouseLook=[NSButton checkboxWithTitle:@(TOTK_UI("按住鼠标右键拖动视角","Hold right mouse button and drag to look")) target:self action:@selector(keyboardOptionsChanged:)];self.mouseLook.frame=NSMakeRect(20,74,540,25);[tab addSubview:self.mouseLook];
 self.mouseInvert=[NSButton checkboxWithTitle:@(TOTK_UI("反转鼠标 Y","Invert mouse Y")) target:self action:@selector(keyboardOptionsChanged:)];self.mouseInvert.frame=NSMakeRect(20,44,180,25);[tab addSubview:self.mouseInvert];
 self.sensitivity=[NSSlider sliderWithValue:100 minValue:10 maxValue:500 target:self action:@selector(keyboardOptionsChanged:)];self.sensitivity.frame=NSMakeRect(340,46,220,22);self.sensitivity.toolTip=@(TOTK_UI("鼠标灵敏度：10%–500%","Mouse sensitivity: 10%–500%"));[tab addSubview:self.sensitivity];
 NSTextField *s=[NSTextField labelWithString:@(TOTK_UI("灵敏度","Sensitivity"))];s.frame=NSMakeRect(254,44,85,25);[tab addSubview:s];
 NSButton *reset=[NSButton buttonWithTitle:@(TOTK_UI("恢复键鼠默认","Reset keyboard/mouse")) target:self action:@selector(resetKeyboard:)];reset.frame=NSMakeRect(20,9,230,28);[tab addSubview:reset];
}
-(void)addControllerTab:(NSTabView *)tabs{
 NSView *tab=controls_tab(tabs,@(TOTK_UI("手柄","Controller")));
 NSTextField *presetLabel=[NSTextField labelWithString:@(TOTK_UI("手柄预设","Controller preset"))];presetLabel.frame=NSMakeRect(20,351,165,25);[tab addSubview:presetLabel];
 self.preset=[[NSPopUpButton alloc] initWithFrame:NSMakeRect(188,347,354,30) pullsDown:NO];[self.preset addItemsWithTitles:@[@"Switch",@"Xbox",@"PlayStation",@(TOTK_UI("通用（按位置）","Generic (by position)")),@(TOTK_UI("自定义","Custom"))]];self.preset.toolTip=@(TOTK_UI("预设会重置按键、触发模式和摇杆选项；点击应用后生效。","Presets reset buttons, modes and stick options; click Apply to activate."));self.preset.target=self;self.preset.action=@selector(presetChanged:);[self.preset selectItemAtIndex:CONTROLLER_PRESET_CUSTOM];[tab addSubview:self.preset];
 self.controllerHint=controls_hint(tab,@"",301,42);
 NSView *rows=input_scroll(tab,CONTROLLER_BUTTONS*34+8,92);rows.enclosingScrollView.frame=NSMakeRect(12,92,576,180);[rows scrollPoint:NSMakePoint(0,rows.frame.size.height)];
 NSMutableArray *targets=[NSMutableArray new],*modes=[NSMutableArray new],*fires=[NSMutableArray new],*labels=[NSMutableArray new];
 NSArray *headings=@[@(TOTK_UI("实体按钮","Physical button")),@(TOTK_UI("游戏按键","Switch button")),@(TOTK_UI("触发方式","Mode")),@(TOTK_UI("瞄准开火键","Aim fire button"))];
 const CGFloat columns[]={20,200,310,414};for(int i=0;i<4;i++){NSTextField *h=[NSTextField labelWithString:headings[i]];h.frame=NSMakeRect(columns[i],276,i==0?175:130,20);h.font=[NSFont systemFontOfSize:12];h.textColor=NSColor.secondaryLabelColor;[tab addSubview:h];}
 NSArray *names=input_controller_names(CONTROLLER_PRESET_CUSTOM);
 NSMutableArray *actions=[NSMutableArray arrayWithObject:@(TOTK_UI("未分配","Unassigned"))];[actions addObjectsFromArray:[input_action_titles() subarrayWithRange:NSMakeRange(0,16)]];
 for(int i=0;i<CONTROLLER_BUTTONS;i++){CGFloat y=(CONTROLLER_BUTTONS-1-i)*34+4;NSTextField *l=[NSTextField labelWithString:names[i]];l.frame=NSMakeRect(8,y+3,180,25);[rows addSubview:l];[labels addObject:l];
  NSPopUpButton *target=[[NSPopUpButton alloc] initWithFrame:NSMakeRect(188,y,106,28) pullsDown:NO];[target addItemsWithTitles:actions];[targets addObject:target];
  NSPopUpButton *mode=[[NSPopUpButton alloc] initWithFrame:NSMakeRect(298,y,100,28) pullsDown:NO];[mode addItemsWithTitles:@[@(TOTK_UI("按住","Hold")),@(TOTK_UI("短按","Tap")),@(TOTK_UI("切换","Toggle")),@(TOTK_UI("瞄准","Aim"))]];mode.toolTip=@(TOTK_UI("hold：按住；tap：短按；toggle：切换至移动结束；aim：按住瞄准，开火键释放。","hold: held; tap: pulse; toggle: until movement ends; aim: hold to aim, fire releases."));[modes addObject:mode];
  NSPopUpButton *fire=[[NSPopUpButton alloc] initWithFrame:NSMakeRect(402,y,140,28) pullsDown:NO];[fire addItemsWithTitles:names];fire.toolTip=@(TOTK_UI("aim 模式的开火实体按钮","Physical fire button for aim mode"));[fires addObject:fire];
  for(NSPopUpButton *p in @[target,mode,fire]){p.tag=i;p.target=self;p.action=@selector(controllerChanged:);[rows addSubview:p];}}
 self.targets=targets;self.modes=modes;self.fires=fires;self.controllerLabels=labels;
 self.swapSticks=[NSButton checkboxWithTitle:@(TOTK_UI("交换左右摇杆","Swap left/right sticks")) target:self action:@selector(controllerOptionsChanged:)];self.swapSticks.frame=NSMakeRect(20,57,270,25);[tab addSubview:self.swapSticks];
 self.padInvert=[NSButton checkboxWithTitle:@(TOTK_UI("反转视角摇杆 Y","Invert look-stick Y")) target:self action:@selector(controllerOptionsChanged:)];self.padInvert.frame=NSMakeRect(300,57,270,25);[tab addSubview:self.padInvert];
 NSButton *reset=[NSButton buttonWithTitle:@(TOTK_UI("恢复手柄默认","Reset controller")) target:self action:@selector(resetController:)];reset.frame=NSMakeRect(20,22,220,28);[tab addSubview:reset];
}
-(void)refreshPlayerActions{
 BOOL available=runtime_action_bindings_available();
 for(unsigned i=0;i<ACTION_BINDING_COUNT;i++){NSInteger selected=action_binding_code_index(_playerActions.target[i]);[self.actionTargets[i] selectItemAtIndex:selected];self.actionTargets[i].enabled=available;}
 self.actionOriginal.enabled=self.actionExample.enabled=available;
 self.actionHint.stringValue=available?@(TOTK_UI("自定义场景中的全部 16 个按键（含 +/−）；菜单、对话和快捷栏内的原始操作保持不变。\n长按与松开跟随新按键；全局页会同时修改菜单，通常保持默认即可。","Customize all 16 gameplay buttons, including Plus/Minus. Original menu, dialogue and shortcut controls stay unchanged.\nHeld actions and releases follow the new buttons. Leave All Buttons at defaults to retain menu controls.")):@(TOTK_UI("游戏动作绑定仅支持已验证的 1.0.0 Mac。\n当前版本不支持，或游戏指令校验未通过；保持原版动作。","Player action bindings require the verified 1.0.0 Mac runtime.\nThis version is unsupported or instruction guards failed; original actions are retained."));
}
-(void)playerActionChanged:(NSPopUpButton *)sender{NSInteger index=sender.indexOfSelectedItem;if(sender.tag>=0&&sender.tag<(NSInteger)ACTION_BINDING_COUNT&&index>=0&&index<(NSInteger)action_binding_code_count())_playerActions.target[sender.tag]=action_binding_code_at((unsigned)index);self.error.stringValue=@"";}
-(void)resetPlayerActions:(id)sender{(void)sender;action_bindings_default(&_playerActions);[self refreshPlayerActions];self.error.stringValue=@"";}
-(void)examplePlayerActions:(id)sender{(void)sender;action_bindings_xbox(&_playerActions);[self refreshPlayerActions];self.error.stringValue=@"";}
-(void)addActionTab:(NSTabView *)tabs{
 NSView *tab=controls_tab(tabs,@(TOTK_UI("动作","Actions")));
 self.actionHint=controls_hint(tab,@"",318,58);
 NSView *rows=input_scroll(tab,ACTION_BINDING_COUNT*30+8,103);rows.enclosingScrollView.frame=NSMakeRect(12,103,576,210);
 NSArray *sources=input_player_action_source_titles(),*actions=input_player_action_titles();NSMutableArray *targets=[NSMutableArray new];
 for(unsigned i=0;i<ACTION_BINDING_COUNT;i++){CGFloat y=(ACTION_BINDING_COUNT-1-i)*30+4;NSTextField *label=[NSTextField labelWithString:sources[i]];label.frame=NSMakeRect(8,y+3,140,22);[rows addSubview:label];
  NSPopUpButton *target=[[NSPopUpButton alloc] initWithFrame:NSMakeRect(160,y,360,26) pullsDown:NO];[target addItemsWithTitles:actions];target.tag=i;target.target=self;target.action=@selector(playerActionChanged:);[rows addSubview:target];[targets addObject:target];}
 self.actionTargets=targets;
 self.actionOriginal=[NSButton buttonWithTitle:@(TOTK_UI("恢复原版动作","Original actions")) target:self action:@selector(resetPlayerActions:)];self.actionOriginal.frame=NSMakeRect(20,62,180,30);[tab addSubview:self.actionOriginal];
 self.actionExample=[NSButton buttonWithTitle:@(TOTK_UI("A 跳跃 · X 攻击 · Y 交互","A jump · X attack · Y interact")) target:self action:@selector(examplePlayerActions:)];self.actionExample.frame=NSMakeRect(218,62,350,30);[tab addSubview:self.actionExample];
 controls_hint(tab,@(TOTK_UI("键鼠与手柄共用这组动作；点击应用后生效。\n场景内的动作提示图标暂时仍显示原版按键。","Keyboard/mouse and controllers share these actions; click Apply to activate.\nIn-world prompts still show the original button icons.")),7,49);
}
-(void)refreshAllButtons{
 for(unsigned i=0;i<BUTTON_BINDING_COUNT;i++){uint8_t t=_gameButtons.target[i];[self.buttonTargets[i] selectItemAtIndex:t==BUTTON_BINDING_NONE?0:t+1];}
}
-(void)buttonChanged:(NSPopUpButton *)sender{
 NSInteger index=sender.indexOfSelectedItem;
 if(sender.tag>=0&&sender.tag<(NSInteger)BUTTON_BINDING_COUNT&&index>=0&&index<=BUTTON_BINDING_COUNT)_gameButtons.target[sender.tag]=index==0?BUTTON_BINDING_NONE:(uint8_t)(index-1);
 self.error.stringValue=@"";
}
-(void)resetAllButtons:(id)sender{(void)sender;button_bindings_default(&_gameButtons);[self refreshAllButtons];self.error.stringValue=@"";}
-(void)addAllButtonsTab:(NSTabView *)tabs{
 NSView *tab=controls_tab(tabs,@(TOTK_UI("全局","All Buttons")));
 controls_hint(tab,@(TOTK_UI("这层映射在键盘与手柄之后，改的是游戏最终看到的 16 个按键；菜单、对话、地图和场景动作都会跟着变。Plus/Minus 也在这里。Home/截图键由系统处理。只想改场景动作时，请保持此页默认并使用“动作”页。","This mapping runs after the keyboard and controller layouts and changes the 16 buttons the game finally sees, so menus, dialogue, maps and gameplay all follow. Plus/Minus are included; Home/Capture stay system-level. Leave this page at defaults and use Actions for gameplay-only changes.")),318,58);
 NSView *rows=input_scroll(tab,BUTTON_BINDING_COUNT*30+8,103);rows.enclosingScrollView.frame=NSMakeRect(12,103,576,210);
 NSArray *names=[input_action_titles() subarrayWithRange:NSMakeRange(0,BUTTON_BINDING_COUNT)];
 NSMutableArray *actions=[NSMutableArray arrayWithObject:@(TOTK_UI("未分配","Unassigned"))];[actions addObjectsFromArray:names];
 NSMutableArray *targets=[NSMutableArray new];
 for(unsigned i=0;i<BUTTON_BINDING_COUNT;i++){
  CGFloat y=(BUTTON_BINDING_COUNT-1-i)*30+4;NSTextField *label=[NSTextField labelWithString:names[i]];label.frame=NSMakeRect(8,y+3,140,22);[rows addSubview:label];
  NSPopUpButton *target=[[NSPopUpButton alloc] initWithFrame:NSMakeRect(160,y,360,26) pullsDown:NO];[target addItemsWithTitles:actions];target.tag=i;target.target=self;target.action=@selector(buttonChanged:);[rows addSubview:target];[targets addObject:target];}
 self.buttonTargets=targets;
 NSButton *reset=[NSButton buttonWithTitle:@(TOTK_UI("恢复原始按键","Original buttons")) target:self action:@selector(resetAllButtons:)];reset.frame=NSMakeRect(20,62,220,30);[tab addSubview:reset];
 controls_hint(tab,@(TOTK_UI("“未分配”会屏蔽该键。两个来源可以指向同一个游戏按键。\n此页改变后，“动作”页的来源名称指的是游戏最终看到的按键。","Unassigned drops the button. Two sources may select the same game button.\nAfter a change, Actions source names mean the buttons the game finally sees.")),7,49);
}
-(void)beginDraft{[self stopRecording];runtime_input_bindings_get(&_keyboard);runtime_gamepad_bindings_get(&_controller);
 id saved=[displayDefaults objectForKey:@"controllerPresetV1"];NSInteger preset=input_number(saved,0,CONTROLLER_PRESET_CUSTOM)?[saved integerValue]:CONTROLLER_PRESET_CUSTOM;
 if(preset<CONTROLLER_PRESET_CUSTOM&&!controller_preset_matches(&_controller,(unsigned)preset))preset=CONTROLLER_PRESET_CUSTOM;
 [self.preset selectItemAtIndex:preset];runtime_action_bindings_get(&_playerActions);runtime_button_bindings_get(&_gameButtons);[self refreshPlayerActions];[self refreshAllButtons];[self refreshKeyboard];[self refreshController];self.error.stringValue=@"";}
-(BOOL)validate:(NSArray<NSNumber *> *)shortcuts{
 _keyboard.mouse_look=self.mouseLook.state==NSControlStateValueOn;_keyboard.mouse_invert_y=self.mouseInvert.state==NSControlStateValueOn;_keyboard.mouse_sensitivity=(int)self.sensitivity.integerValue;
 _controller.swap_sticks=self.swapSticks.state==NSControlStateValueOn;_controller.invert_look_y=self.padInvert.state==NSControlStateValueOn;
 if(!button_bindings_valid(&_gameButtons)){self.error.stringValue=@(TOTK_UI("全局按键包含无效目标。","All Buttons contains an invalid target."));return NO;}
 RuntimeActionBindings current;runtime_action_bindings_get(&current);
 if(!action_bindings_valid(&_playerActions)||(!runtime_action_bindings_available()&&!action_bindings_equal(&_playerActions,&current))){self.error.stringValue=@(TOTK_UI("当前运行版本不支持这些游戏动作绑定。","This runtime does not support these player action bindings."));return NO;}
 if(!input_bindings_valid(&_keyboard)){self.error.stringValue=@(TOTK_UI("键鼠绑定重复，请先清除原绑定。","Duplicate keyboard/mouse binding; clear the previous binding first."));return NO;}
 for(int i=0;i<INPUT_ACTIONS;i++)for(int j=0;j<2;j++){uint64_t s=_keyboard.source[i][j];if(!s||s>INPUT_KEYS)continue;
  if(s==24){self.error.stringValue=@(TOTK_UI("5 为时域抗锯齿保留。","5 is reserved for temporal antialiasing."));return NO;}
  for(NSNumber *key in shortcuts)if(key.unsignedLongLongValue==s){self.error.stringValue=@(TOTK_UI("游戏绑定与快捷键冲突，请更换其中一个。","Game binding conflicts with a shortcut; change one of them."));return NO;}}
 if(!controller_bindings_valid(&_controller)){self.error.stringValue=@(TOTK_UI("手柄 aim 的开火按钮不能是它自身。","An aim binding needs a different physical fire button."));return NO;}self.error.stringValue=@"";return YES;
}
-(BOOL)apply{
 RuntimeControllerBindings previous;RuntimeKeyboardBindings previousKeys;RuntimeActionBindings previousActions;RuntimeButtonBindings previousButtons;runtime_gamepad_bindings_get(&previous);runtime_input_bindings_get(&previousKeys);runtime_action_bindings_get(&previousActions);runtime_button_bindings_get(&previousButtons);
 if(!runtime_gamepad_bindings_set(&_controller)){self.error.stringValue=@(TOTK_UI("手柄设置未应用，请重试。","Controller settings were not applied; try again."));return NO;}
 if(!runtime_input_bindings_set(&_keyboard)){runtime_gamepad_bindings_set(&previous);self.error.stringValue=@(TOTK_UI("键鼠设置未应用，请重试。","Keyboard/mouse settings were not applied; try again."));return NO;}
 if(!button_bindings_equal(&_gameButtons,&previousButtons)&&!runtime_button_bindings_set(&_gameButtons)){
  runtime_gamepad_bindings_set(&previous);runtime_input_bindings_set(&previousKeys);self.error.stringValue=@(TOTK_UI("全局按键未应用，其他输入设置已还原。","All-buttons mapping was not applied; other input settings were restored."));return NO;
 }
 if(!action_bindings_equal(&_playerActions,&previousActions)&&!runtime_action_bindings_set(&_playerActions)){
  runtime_gamepad_bindings_set(&previous);runtime_input_bindings_set(&previousKeys);runtime_button_bindings_set(&previousButtons);self.error.stringValue=@(TOTK_UI("游戏动作未应用，其他输入设置已还原。","Player actions were not applied; other input settings were restored."));return NO;
 }
 [displayDefaults setObject:input_button_values(&_gameButtons) forKey:@"allButtonBindingsV1"];
 if(runtime_action_bindings_available())[displayDefaults setObject:input_player_action_values(&_playerActions) forKey:@"playerActionBindingsV3"];
 [displayDefaults setObject:input_keyboard_values(&_keyboard) forKey:@"keyboardMouseBindingsV1"];[displayDefaults setObject:input_controller_values(&_controller) forKey:@"controllerBindingsV1"];
 [displayDefaults setInteger:self.preset.indexOfSelectedItem forKey:@"controllerPresetV1"];return YES;
}
-(void)dealloc{if(_monitor)[NSEvent removeMonitor:_monitor];}
@end
