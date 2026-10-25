#define main command_q_probe_main
#include "../games/totk/tests/command_q/probe.m"
#undef main
#include <assert.h>
/* The Mac control menu (games/totk/runtime/game_controls.h) on the real AppKit sheet, without game data: controls.txt
   slots, options and comments, only-changed saving, conflicts, shortcut checks and dispatch. */
static void spin(void){[NSRunLoop.currentRunLoop runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];}
static NSString *read_file(NSString *f){return [NSString stringWithContentsOfFile:f encoding:NSUTF8StringEncoding error:NULL];}
int main(int argc,char **argv){@autoreleasepool{
 assert(argc==2);NSString *folder=@(argv[1]);NSString *file=[folder stringByAppendingPathComponent:@"controls.txt"];setenv("TOTK_CONTROLS",file.fileSystemRepresentation,1);
 NSString *suite=[NSString stringWithFormat:@"org.macnativegames.probe.%d",getpid()];displayDefaults=[[NSUserDefaults alloc] initWithSuiteName:suite];controlsLoaded=YES;
 for(unsigned i=0;i<CONTROL_SHORTCUTS;i++)atomic_store(&controlKeys[i],control_shortcut_default(i));
 [NSApplication sharedApplication];[NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];[NSApp finishLaunching];
 window=[[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,900,700) styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO];window.releasedWhenClosed=NO;window.contentView=[TOTKInputView new];
 front_install_menu();TOTKGameControls *c=gameControls;NSError *error=nil;
 ControlsLayout read;NSData *original;assert(controls_read(&read,&original,&error)&&!original&&read.keys[38]==1);
 NSString *initial=@"# user comment\nsouth = x tap\nmouse_right = zr aim mouse5\nkey_38 = b # custom key\n";
 assert([initial writeToFile:file atomically:YES encoding:NSUTF8StringEncoding error:&error]);
 [c showSettings:nil];assert(controlsOpen&&c->layout.map[0]==4&&c->layout.keys[38]==2);
 assert(c.slotButtons.count==48&&c.padTargets.count==22&&c.actionTargets.count==16&&c.gameTargets.count==16&&c.hotkeyButtons.count==CONTROL_SHORTCUTS&&c.tabs.numberOfTabViewItems==7);
 /* Slots: J moves from B to A; 1 is reserved; mouse buttons only for game buttons. */
 c.recordingSlot=0;[c captureSource:38+1];assert(c->layout.keys[38]==1&&c.error.stringValue.length);
 c.recordingSlot=0;[c captureSource:18+1];assert(!c->layout.keys[18]&&c.error.stringValue.length);
 c.recordingSlot=16*2;[c captureSource:CONTROLS_SOURCE_MOUSE];assert(c->layout.map[CONTROLLER_MOUSE_FIRST]==8);
 c.recordingSlot=1;[c captureSource:CONTROLS_SOURCE_MOUSE+4];assert(c->layout.map[CONTROLLER_MOUSE_FIRST+4]==1);
 { unsigned s[2];[c sources:1 into:s];assert(s[0]==38+1&&s[1]==CONTROLS_SOURCE_MOUSE+4); }
 c->layout.game[0]=CONTROLS_GAME_NONE;c.swapSticks.state=NSControlStateValueOn;[c controllerOptionsChanged:nil];assert(c->layout.swap_sticks&&c.preset.indexOfSelectedItem==CONTROLLER_PRESET_CUSTOM);
 /* A shortcut on a plain key that drives the game is refused, file untouched. */
 c.draftKeys[CONTROL_SHORTCUT_PRESET]=@(control_hotkey(38,0));[c applySettings:nil];assert(controlsOpen&&c.error.stringValue.length&&[read_file(file) isEqualToString:initial]);
 c.draftKeys[CONTROL_SHORTCUT_PRESET]=@(control_hotkey(38,CONTROL_MOD_SHIFT));[c applySettings:nil];spin();assert(!controlsOpen);
 NSString *saved=read_file(file);
 assert([saved containsString:@"# user comment"]&&[saved containsString:@"# custom key"]&&[saved containsString:@"game_a = none\n"]&&[saved containsString:@"swap_sticks = on\n"]&&[saved containsString:@"mouse5 = a hold\n"]&&[saved containsString:@"key_38 = a\n"]);
 assert(atomic_load(&controlKeys[CONTROL_SHORTCUT_PRESET])==control_hotkey(38,CONTROL_MOD_SHIFT)&&[[displayDefaults arrayForKey:@"controlShortcuts"] count]==CONTROL_SHORTCUTS);
 /* Nothing changed: nothing is written or saved (a player who never uses the menu keeps main's behaviour). */
 for(NSString *k in @[@"resolution",@"upscalePreset",@"vsync",@"nativeFrameCap",@"frameGeneration",@"taa",@"mode"])assert(![displayDefaults objectForKey:k]);
 [c showSettings:nil];[c applySettings:nil];spin();assert([read_file(file) isEqualToString:saved]);
 for(NSString *k in @[@"resolution",@"vsync",@"nativeFrameCap",@"frameGeneration",@"taa",@"mode"])assert(![displayDefaults objectForKey:k]);
 /* Changed values only: the resolution and the limit. */
 [c showSettings:nil];[c.resolution selectItemAtIndex:3];[c resolutionChanged:nil];[c.quality selectItemAtIndex:1];c.cap.stringValue=@"90";[c applySettings:nil];spin();
 assert([displayDefaults integerForKey:@"resolution"]==2&&[displayDefaults integerForKey:@"upscalePreset"]==1&&[displayDefaults integerForKey:@"nativeFrameCap"]==90&&renderRestart&&![displayDefaults objectForKey:@"vsync"]);
 [c showSettings:nil];c.cap.stringValue=@"7";[c applySettings:nil];assert(controlsOpen&&c.error.stringValue.length);[c cancelSettings:nil];spin();
 /* Key 3 hands the picture back to its own modes. */
 assert(runtime_controls_handle_key(20,0,NO));assert(![displayDefaults objectForKey:@"resolution"]);
 assert(runtime_controls_handle_key(18,0,NO)&&!runtime_controls_handle_key(18,NSEventModifierFlagShift,NO)&&runtime_controls_handle_key(38,NSEventModifierFlagShift,NO)&&!runtime_controls_handle_key(38,0,NO));
 /* External edit: Apply refuses and leaves the other program's file. */
 [c showSettings:nil];c->layout.keys[38]=3;assert([initial writeToFile:file atomically:YES encoding:NSUTF8StringEncoding error:&error]);[c applySettings:nil];
 assert(controlsOpen&&c.error.stringValue.length&&[read_file(file) isEqualToString:initial]);
 [c presetChanged:nil];[c.window orderFront:nil];spin();[c.window display];NSView *content=c.window.contentView;NSBitmapImageRep *bitmap=[content bitmapImageRepForCachingDisplayInRect:content.bounds];[content cacheDisplayInRect:content.bounds toBitmapImageRep:bitmap];
 [[bitmap representationUsingType:NSBitmapImageFileTypePNG properties:@{}] writeToFile:[folder stringByAppendingPathComponent:@"menu.png"] atomically:YES];
 [c cancelSettings:nil];spin();assert(!controlsOpen&&[read_file(file) isEqualToString:initial]);
 assert([@"key_18 = a" writeToFile:file atomically:YES encoding:NSUTF8StringEncoding error:&error]);assert(!controls_read(&read,&original,&error));
 [displayDefaults removePersistentDomainForName:suite];
 puts("PASS: AppKit control menu, controls.txt slots/options/comments, only-changed saving, conflicts, shortcut checks and dispatch");return 0;
}}
