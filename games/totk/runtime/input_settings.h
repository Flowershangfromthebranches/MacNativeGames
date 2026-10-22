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
static NSArray *input_player_action_values(const RuntimeActionBindings *b){return @[@(b->target[0]),@(b->target[1]),@(b->target[2])];}
static BOOL input_player_action_decode(id values,RuntimeActionBindings *b){
 if(![values isKindOfClass:NSArray.class]||[values count]!=ACTION_BINDING_COUNT)return NO;RuntimeActionBindings next;
 for(unsigned i=0;i<ACTION_BINDING_COUNT;i++){if(!input_number(values[i],0,8))return NO;next.target[i]=[values[i] unsignedIntValue];}
 if(!action_bindings_valid(&next))return NO;*b=next;return YES;
}
static void display_input_load(void){
 RuntimeKeyboardBindings k;RuntimeControllerBindings c;RuntimeActionBindings a;
 if(input_player_action_decode([displayDefaults objectForKey:@"playerActionBindingsV1"],&a)&&!runtime_action_bindings_set(&a))fprintf(stderr,"[totk] saved player actions unsupported by this runtime; original actions retained\n");
 if(input_keyboard_decode([displayDefaults objectForKey:@"keyboardMouseBindingsV1"],&k))runtime_input_bindings_set(&k);
 if(input_controller_decode([displayDefaults objectForKey:@"controllerBindingsV1"],&c))runtime_gamepad_bindings_set(&c);
}
@interface TOTKInputSettings:NSObject
@property RuntimeKeyboardBindings keyboard;
@property RuntimeControllerBindings controller;
@property RuntimeActionBindings playerActions;
@property(strong) NSArray<NSPopUpButton *> *actionTargets;
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
 NSView *tab=controls_tab(tabs,@(TOTK_UI("键盘与鼠标","Keyboard & Mouse")));self.keyboardTab=tab;
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
 static const unsigned values[]={0,ACTION_INTERACT,ACTION_JUMP,ACTION_ATTACK};BOOL available=runtime_action_bindings_available();
 for(unsigned i=0;i<ACTION_BINDING_COUNT;i++){NSInteger selected=0;for(unsigned j=0;j<4;j++)if(_playerActions.target[i]==values[j])selected=j;[self.actionTargets[i] selectItemAtIndex:selected];self.actionTargets[i].enabled=available;}
 self.actionOriginal.enabled=self.actionExample.enabled=available;
 self.actionHint.stringValue=available?@(TOTK_UI("只修改场景中的角色动作。菜单、对话、地图仍用原来的确认与返回键。\nA/X/Y 指“手柄”页右侧的游戏按键；B 保持奔跑。","Changes player actions only. Menus, dialogue and maps retain confirm/return.\nA/X/Y refer to the Switch buttons in Controller; B keeps running.")):@(TOTK_UI("游戏动作绑定仅支持已验证的 1.0.0 Mac。\n当前版本不支持，或游戏指令校验未通过；保持原版动作。","Player action bindings require the verified 1.0.0 Mac runtime.\nThis version is unsupported or instruction guards failed; original actions are retained."));
}
-(void)playerActionChanged:(NSPopUpButton *)sender{static const unsigned values[]={0,ACTION_INTERACT,ACTION_JUMP,ACTION_ATTACK};NSInteger index=sender.indexOfSelectedItem;if(sender.tag>=0&&sender.tag<ACTION_BINDING_COUNT&&index>=0&&index<4)_playerActions.target[sender.tag]=values[index];self.error.stringValue=@"";}
-(void)resetPlayerActions:(id)sender{(void)sender;action_bindings_default(&_playerActions);[self refreshPlayerActions];self.error.stringValue=@"";}
-(void)examplePlayerActions:(id)sender{(void)sender;_playerActions=(RuntimeActionBindings){{ACTION_JUMP,ACTION_ATTACK,ACTION_INTERACT}};[self refreshPlayerActions];self.error.stringValue=@"";}
-(void)addActionTab:(NSTabView *)tabs{
 NSView *tab=controls_tab(tabs,@(TOTK_UI("游戏动作","Player Actions")));
 self.actionHint=controls_hint(tab,@"",302,77);NSArray *names=@[@(TOTK_UI("A（原为交互）","A (originally interact)")),@(TOTK_UI("X（原为跳跃）","X (originally jump)")),@(TOTK_UI("Y（原为攻击）","Y (originally attack)"))];NSMutableArray *targets=[NSMutableArray new];
 for(unsigned i=0;i<ACTION_BINDING_COUNT;i++){CGFloat y=253-i*47;controls_label(tab,names[i],y);NSPopUpButton *target=controls_popup(tab,@[@(TOTK_UI("未分配","Unassigned")),@(TOTK_UI("交互","Interact")),@(TOTK_UI("跳跃","Jump")),@(TOTK_UI("攻击","Attack"))],y-3);target.tag=i;target.target=self;target.action=@selector(playerActionChanged:);[targets addObject:target];}self.actionTargets=targets;
 controls_label(tab,@(TOTK_UI("B","B")),111);NSTextField *run=[NSTextField labelWithString:@(TOTK_UI("奔跑（保持原游戏行为）","Run (original game behavior)"))];run.frame=NSMakeRect(218,111,350,25);[tab addSubview:run];
 self.actionOriginal=[NSButton buttonWithTitle:@(TOTK_UI("恢复原版动作","Original actions")) target:self action:@selector(resetPlayerActions:)];self.actionOriginal.frame=NSMakeRect(20,65,180,30);[tab addSubview:self.actionOriginal];
 self.actionExample=[NSButton buttonWithTitle:@(TOTK_UI("A 跳跃 · X 攻击 · Y 交互","A jump · X attack · Y interact")) target:self action:@selector(examplePlayerActions:)];self.actionExample.frame=NSMakeRect(218,65,350,30);[tab addSubview:self.actionExample];
 controls_hint(tab,@(TOTK_UI("键鼠与手柄共用这组动作；点击应用后生效。\n场景内的动作提示图标暂时仍显示原版按键。","Keyboard/mouse and controllers share these actions; click Apply to activate.\nIn-world action prompts still show the original button icons.")),7,49);
}
-(void)beginDraft{[self stopRecording];runtime_input_bindings_get(&_keyboard);runtime_gamepad_bindings_get(&_controller);
 id saved=[displayDefaults objectForKey:@"controllerPresetV1"];NSInteger preset=input_number(saved,0,CONTROLLER_PRESET_CUSTOM)?[saved integerValue]:CONTROLLER_PRESET_CUSTOM;
 if(preset<CONTROLLER_PRESET_CUSTOM&&!controller_preset_matches(&_controller,(unsigned)preset))preset=CONTROLLER_PRESET_CUSTOM;
 [self.preset selectItemAtIndex:preset];runtime_action_bindings_get(&_playerActions);[self refreshPlayerActions];[self refreshKeyboard];[self refreshController];self.error.stringValue=@"";}
-(BOOL)validate:(NSArray<NSNumber *> *)shortcuts{
 _keyboard.mouse_look=self.mouseLook.state==NSControlStateValueOn;_keyboard.mouse_invert_y=self.mouseInvert.state==NSControlStateValueOn;_keyboard.mouse_sensitivity=(int)self.sensitivity.integerValue;
 _controller.swap_sticks=self.swapSticks.state==NSControlStateValueOn;_controller.invert_look_y=self.padInvert.state==NSControlStateValueOn;
 RuntimeActionBindings current;runtime_action_bindings_get(&current);
 if(!action_bindings_valid(&_playerActions)||(!runtime_action_bindings_available()&&action_bindings_pack(&_playerActions)!=action_bindings_pack(&current))){self.error.stringValue=@(TOTK_UI("当前运行版本不支持这些游戏动作绑定。","This runtime does not support these player action bindings."));return NO;}
 if(!input_bindings_valid(&_keyboard)){self.error.stringValue=@(TOTK_UI("键鼠绑定重复，请先清除原绑定。","Duplicate keyboard/mouse binding; clear the previous binding first."));return NO;}
 for(int i=0;i<INPUT_ACTIONS;i++)for(int j=0;j<2;j++){uint64_t s=_keyboard.source[i][j];if(!s||s>INPUT_KEYS)continue;
  if(s==24){self.error.stringValue=@(TOTK_UI("5 为时域抗锯齿保留。","5 is reserved for temporal antialiasing."));return NO;}
  for(NSNumber *key in shortcuts)if(key.unsignedLongLongValue==s){self.error.stringValue=@(TOTK_UI("游戏绑定与快捷键冲突，请更换其中一个。","Game binding conflicts with a shortcut; change one of them."));return NO;}}
 if(!controller_bindings_valid(&_controller)){self.error.stringValue=@(TOTK_UI("手柄 aim 的开火按钮不能是它自身。","An aim binding needs a different physical fire button."));return NO;}self.error.stringValue=@"";return YES;
}
-(BOOL)apply{
 RuntimeControllerBindings previous;RuntimeKeyboardBindings previousKeys;RuntimeActionBindings previousActions;runtime_gamepad_bindings_get(&previous);runtime_input_bindings_get(&previousKeys);runtime_action_bindings_get(&previousActions);
 if(!runtime_gamepad_bindings_set(&_controller)){self.error.stringValue=@(TOTK_UI("手柄设置未应用，请重试。","Controller settings were not applied; try again."));return NO;}
 if(!runtime_input_bindings_set(&_keyboard)){runtime_gamepad_bindings_set(&previous);self.error.stringValue=@(TOTK_UI("键鼠设置未应用，请重试。","Keyboard/mouse settings were not applied; try again."));return NO;}
 if(action_bindings_pack(&_playerActions)!=action_bindings_pack(&previousActions)&&!runtime_action_bindings_set(&_playerActions)){
  runtime_gamepad_bindings_set(&previous);runtime_input_bindings_set(&previousKeys);self.error.stringValue=@(TOTK_UI("游戏动作未应用，其他输入设置已还原。","Player actions were not applied; other input settings were restored."));return NO;
 }
 if(runtime_action_bindings_available())[displayDefaults setObject:input_player_action_values(&_playerActions) forKey:@"playerActionBindingsV1"];
 [displayDefaults setObject:input_keyboard_values(&_keyboard) forKey:@"keyboardMouseBindingsV1"];[displayDefaults setObject:input_controller_values(&_controller) forKey:@"controllerBindingsV1"];
 [displayDefaults setInteger:self.preset.indexOfSelectedItem forKey:@"controllerPresetV1"];return YES;
}
-(void)dealloc{if(_monitor)[NSEvent removeMonitor:_monitor];}
@end
