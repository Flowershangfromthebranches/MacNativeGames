/* Core AppKit binding editor check; no guest, saves, controller or GPU work. */
#define main display_probe_original_main
#include "frontend_display_probe.m"
#undef main
static NSEvent *key_event(NSWindow *w,unsigned code,NSEventModifierFlags mods,BOOL repeat){return [NSEvent keyEventWithType:NSEventTypeKeyDown location:NSZeroPoint modifierFlags:mods timestamp:0 windowNumber:w.windowNumber context:nil characters:@"b" charactersIgnoringModifiers:@"b" isARepeat:repeat keyCode:code];}
static void capture_input_tab(NSInteger index,NSString *path){
 [displaySettings.tabs selectTabViewItemAtIndex:index];[displaySettings.window.contentView layoutSubtreeIfNeeded];[NSRunLoop.currentRunLoop runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
 NSView *v=displaySettings.window.contentView;NSBitmapImageRep *rep=[v bitmapImageRepForCachingDisplayInRect:v.bounds];[v cacheDisplayInRect:v.bounds toBitmapImageRep:rep];assert([[rep representationUsingType:NSBitmapImageFileTypePNG properties:@{}] writeToFile:path atomically:YES]);
}
int main(void){@autoreleasepool{
 setenv("TOTK_SLOT_OWNER","binding-probe",1);setenv("TOTK_ASSUME_FOCUS","1",1);[NSApplication sharedApplication];NSApp.appearance=[NSAppearance appearanceNamed:NSAppearanceNameAqua];[NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
 display_load();NSString *suite=@"org.macnativegames.totk.display.tests";[displayDefaults removePersistentDomainForName:suite];display_load();
 window=[[TOTKGameWindow alloc] initWithContentRect:NSMakeRect(100,100,900,600) styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO];
 [window makeKeyAndOrderFront:nil];displaySettings=[TOTKDisplaySettings new];[displaySettings showSettings:nil];
 [NSApp activateIgnoringOtherApps:YES];[NSRunLoop.currentRunLoop runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
 assert(front_modifier_down(56,NX_DEVICELSHIFTKEYMASK)&&!front_modifier_down(60,NX_DEVICELSHIFTKEYMASK));
 assert(front_modifier_down(60,NX_DEVICERSHIFTKEYMASK)&&!front_modifier_down(56,NX_DEVICERSHIFTKEYMASK));
 TOTKWindowDelegate *delegate=[TOTKWindowDelegate new];[delegate windowDidResignKey:[NSNotification notificationWithName:NSWindowDidResignKeyNotification object:window]];assert(probeSuspended);
 assert(displaySettings.tabs.numberOfTabViewItems==7&&probeSuspended);TOTKInputSettings *s=displaySettings.inputSettings;assert(s.keyButtons.count==48&&s.targets.count==17&&s.buttonTargets.count==BUTTON_BINDING_COUNT);[displaySettings.tabs selectTabViewItemAtIndex:3];
 RuntimeKeyboardBindings original=probeKeyboard;
 [s record:s.keyButtons[0]];assert(s.monitor);NSEvent *outside=key_event(window,11,0,NO);assert([s captureEvent:outside]==outside&&s.recording==0);
 [s captureEvent:key_event(s.hostWindow,11,0,YES)];assert(s.recording==0);
 [s captureEvent:key_event(s.hostWindow,11,NSEventModifierFlagCommand,NO)];assert(s.recording==0&&s.error.stringValue.length);
 [s captureEvent:key_event(s.hostWindow,53,0,NO)];assert(!s.monitor&&s.keyboard.source[0][0]==original.source[0][0]);
 s.mouseLook.state=NSControlStateValueOn;s.sensitivity.integerValue=175;[s keyboardOptionsChanged:nil];
 [s record:s.keyButtons[0]];[s captureEvent:key_event(s.hostWindow,11,0,NO)];assert(s.keyboard.source[0][0]==12&&!s.monitor&&s.keyboard.mouse_look&&s.keyboard.mouse_sensitivity==175);
 [s record:s.keyButtons[2]];NSPoint bindingPoint=[s.keyButtons[2] convertPoint:NSMakePoint(10,10) toView:nil];NSEvent *mouse=[NSEvent mouseEventWithType:NSEventTypeLeftMouseDown location:bindingPoint modifierFlags:0 timestamp:0 windowNumber:s.hostWindow.windowNumber context:nil eventNumber:1 clickCount:1 pressure:1];[s captureEvent:mouse];assert(s.keyboard.source[1][0]==129);
 [s record:s.keyButtons[0]];NSEvent *outsidePage=[NSEvent mouseEventWithType:NSEventTypeLeftMouseDown location:NSZeroPoint modifierFlags:0 timestamp:0 windowNumber:s.hostWindow.windowNumber context:nil eventNumber:2 clickCount:1 pressure:1];assert([s captureEvent:outsidePage]==outsidePage&&!s.monitor&&s.keyboard.source[0][0]==12);
 [s record:s.keyButtons[0]];[displaySettings.tabs selectTabViewItemAtIndex:4];assert(!s.monitor);
 [displaySettings cancelSettings:nil];assert(!probeSuspended&&!memcmp(&original,&probeKeyboard,sizeof original));[delegate windowDidResignKey:[NSNotification notificationWithName:NSWindowDidResignKeyNotification object:window]];assert(!probeSuspended);
 [displaySettings showSettings:nil];RuntimeKeyboardBindings k=s.keyboard;k.source[0][0]=12;s.keyboard=k;[s refreshKeyboard];
 assert([s validate:displaySettings.draftKeys]);k.source[1][0]=12;s.keyboard=k;assert(![s validate:displaySettings.draftKeys]);
 k.source[1][0]=129;k.source[0][0]=24;s.keyboard=k;assert(![s validate:displaySettings.draftKeys]);k.source[0][0]=19;s.keyboard=k;assert(![s validate:displaySettings.draftKeys]);
 k.source[0][0]=12;k.mouse_look=1;k.mouse_sensitivity=175;s.keyboard=k;[s refreshKeyboard];
 [s.targets[0] selectItemAtIndex:2];[s controllerChanged:s.targets[0]];s.swapSticks.state=NSControlStateValueOn;s.padInvert.state=NSControlStateValueOn;[s controllerOptionsChanged:nil];
 [s.modes[0] selectItemAtIndex:CONTROLLER_AIM];[s.fires[0] selectItemAtIndex:0];[s controllerChanged:s.modes[0]];assert(![s validate:displaySettings.draftKeys]);
 [s.fires[0] selectItemAtIndex:9];[s controllerChanged:s.fires[0]];assert([s validate:displaySettings.draftKeys]);
 const char *out=getenv("TOTK_INPUT_PROBE_IMAGES");if(out){capture_input_tab(3,[@(out) stringByAppendingPathComponent:@"keyboard.png"]);capture_input_tab(4,[@(out) stringByAppendingPathComponent:@"controller.png"]);}
 [displaySettings applySettings:nil];assert(!probeSuspended&&probeKeyboard.source[0][0]==12&&probeKeyboard.source[1][0]==129&&probeKeyboard.mouse_sensitivity==175&&probeController.map[0]==2&&probeController.swap_sticks&&probeController.invert_look_y);
 input_bindings_default(&probeKeyboard);controller_map_default(probeController.map,probeController.modes);probeController.swap_sticks=probeController.invert_look_y=0;display_input_load();assert(probeKeyboard.source[0][0]==12&&probeKeyboard.mouse_look&&probeController.swap_sticks);
 [displaySettings showSettings:nil];[s resetKeyboard:nil];[s resetController:nil];assert(s.keyboard.source[0][0]==39&&!s.controller.swap_sticks);[displaySettings cancelSettings:nil];assert(probeKeyboard.source[0][0]==12&&probeController.swap_sticks);
 RuntimeKeyboardBindings decoded;NSMutableArray *malformed=[input_keyboard_values(&probeKeyboard) mutableCopy];malformed[0]=@1.5;assert(!input_keyboard_decode(malformed,&decoded));malformed[0]=@"12";assert(!input_keyboard_decode(malformed,&decoded));assert(!input_keyboard_decode(@[],&decoded));
 RuntimeControllerBindings pad;assert(!input_controller_decode(@[],&pad));NSMutableArray *bad=[input_controller_values(&probeController) mutableCopy];bad[0]=@3;assert(!input_controller_decode(bad,&pad));
 [displayDefaults setObject:@[] forKey:@"keyboardMouseBindingsV1"];display_input_load();assert(probeKeyboard.source[0][0]==12);
 // Presets are drafts until Apply, survive preference reload, and manual edits become Custom.
 for(NSInteger preset=0;preset<CONTROLLER_PRESET_CUSTOM;preset++){
  [displaySettings showSettings:nil];RuntimeControllerBindings before=probeController;
  [s.preset selectItemAtIndex:preset];[NSApp sendAction:s.preset.action to:s.preset.target from:s.preset];
  RuntimeControllerBindings draft=s.controller;assert(controller_preset_matches(&draft,(unsigned)preset));
  assert(controller_bindings_equal(&before,&probeController));[displaySettings cancelSettings:nil];
  assert(controller_bindings_equal(&before,&probeController));
  [displaySettings showSettings:nil];[s.preset selectItemAtIndex:preset];[s presetChanged:nil];
  [displaySettings applySettings:nil];assert(!displaySettings.window.sheetParent&&controller_preset_matches(&probeController,(unsigned)preset));
  controller_map_default(probeController.map,probeController.modes);display_input_load();
  [displaySettings showSettings:nil];assert(s.preset.indexOfSelectedItem==preset&&controller_preset_matches(&probeController,(unsigned)preset));
  assert([s.controllerLabels[0].stringValue isEqualToString:input_controller_names(preset)[0]]);
  if(out&&preset==CONTROLLER_PRESET_XBOX)capture_input_tab(4,[@(out) stringByAppendingPathComponent:@"controller-xbox-preset.png"]);
  assert(s.targets[0].indexOfSelectedItem==__builtin_ctz(probeController.map[0])+1);
  [displaySettings cancelSettings:nil];
 }
 [displaySettings showSettings:nil];[s.preset selectItemAtIndex:CONTROLLER_PRESET_XBOX];[s presetChanged:nil];
 [s.targets[0] selectItemAtIndex:3];[s controllerChanged:s.targets[0]];assert(s.preset.indexOfSelectedItem==CONTROLLER_PRESET_CUSTOM);[displaySettings cancelSettings:nil];
 // Rejecting a runtime apply must keep the sheet open and leave the saved preset untouched.
 [displaySettings showSettings:nil];[s.preset selectItemAtIndex:CONTROLLER_PRESET_XBOX];[s presetChanged:nil];
 probeRejectController=1;[displaySettings applySettings:nil];assert(displaySettings.window.sheetParent&&s.error.stringValue.length);
 assert([displayDefaults integerForKey:@"controllerPresetV1"]==CONTROLLER_PRESET_GENERIC);
 probeRejectController=0;[displaySettings cancelSettings:nil];
 // A missing/malformed or stale preset label must never overwrite an existing custom mapping.
 [displayDefaults removeObjectForKey:@"controllerPresetV1"];[displaySettings showSettings:nil];assert(s.preset.indexOfSelectedItem==CONTROLLER_PRESET_CUSTOM);[displaySettings cancelSettings:nil];
 [displayDefaults setInteger:CONTROLLER_PRESET_XBOX forKey:@"controllerPresetV1"];[displaySettings showSettings:nil];assert(s.preset.indexOfSelectedItem==CONTROLLER_PRESET_CUSTOM);[displaySettings cancelSettings:nil];
 // Player actions are an independent draft; changing them must not change menu button mappings.
 [displaySettings showSettings:nil];RuntimeControllerBindings menuBefore=probeController;RuntimeActionBindings actionsBefore=probeActions;
 [s examplePlayerActions:nil];assert(s.playerActions.target[0]==ACTION_JUMP);assert(action_bindings_equal(&probeActions,&actionsBefore));
 [displaySettings cancelSettings:nil];assert(action_bindings_equal(&probeActions,&actionsBefore));
 [displaySettings showSettings:nil];[s examplePlayerActions:nil];[displaySettings applySettings:nil];
 assert(probeActions.target[0]==ACTION_JUMP&&probeActions.target[1]==ACTION_CANCEL&&probeActions.target[2]==ACTION_ATTACK&&probeActions.target[3]==ACTION_INTERACT);
 assert(controller_bindings_equal(&menuBefore,&probeController));action_bindings_default(&probeActions);display_input_load();assert(probeActions.target[0]==ACTION_JUMP);
 [displaySettings showSettings:nil];assert(s.actionTargets[0].indexOfSelectedItem==3);[s resetPlayerActions:nil];[displaySettings cancelSettings:nil];assert(probeActions.target[0]==ACTION_JUMP);
 [displaySettings showSettings:nil];[s resetPlayerActions:nil];probeRejectActions=1;[displaySettings applySettings:nil];assert(displaySettings.window.sheetParent&&s.error.stringValue.length);assert(controller_bindings_equal(&menuBefore,&probeController));probeRejectActions=0;[displaySettings cancelSettings:nil];
 RuntimeActionBindings badActions;assert(!input_player_action_decode(@[@2,@4,@8],&badActions));assert(!input_player_action_decode(@[@1,@4],&badActions));NSMutableArray *badActionValues=[NSMutableArray array];for(int i=0;i<ACTION_BINDING_COUNT;i++)[badActionValues addObject:@(ACTION_NONE)];badActionValues[0]=@3;assert(!input_player_action_decode(badActionValues,&badActions));
 [displayDefaults removeObjectForKey:@"playerActionBindingsV3"];[displayDefaults setObject:@[@4,@8,@1] forKey:@"playerActionBindingsV1"];display_input_load();assert(probeActions.target[0]==ACTION_JUMP&&probeActions.target[2]==ACTION_ATTACK&&probeActions.target[3]==ACTION_INTERACT);
 // Migrate the previous 14-source layout; newly exposed Plus/Minus keep UI defaults.
 [displayDefaults removeObjectForKey:@"playerActionBindingsV3"];
 NSArray *legacyV2=[[input_player_action_values(&probeActions) subarrayWithRange:NSMakeRange(0,14)] copy];
 [displayDefaults setObject:legacyV2 forKey:@"playerActionBindingsV2"];display_input_load();
 assert(probeActions.target[14]==ACTION_PAUSE&&probeActions.target[15]==ACTION_MAP&&probeActions.target[0]==ACTION_JUMP);
 [displayDefaults removeObjectForKey:@"playerActionBindingsV2"];
 // Fractions, negative/wrapped values and malformed legacy arrays must be ignored.
 NSMutableArray *validActionValues=[input_player_action_values(&probeActions) mutableCopy];
 for(NSNumber *bad in @[@1.5,@(-1),@4294967297ULL]){validActionValues[0]=bad;assert(!input_player_action_decode(validActionValues,&badActions));}
 [displayDefaults removeObjectForKey:@"playerActionBindingsV3"];[displayDefaults setObject:@[@4,@3,@1] forKey:@"playerActionBindingsV1"];
 RuntimeActionBindings beforeBad=probeActions;display_input_load();assert(action_bindings_equal(&beforeBad,&probeActions));
 // Every action row can change, apply, persist and cancel; raw menu bindings stay put.
 [displaySettings showSettings:nil];assert(s.actionTargets.count==ACTION_BINDING_COUNT);
 for(unsigned i=0;i<ACTION_BINDING_COUNT;i++){[s.actionTargets[i] selectItemAtIndex:(i+2)%ACTION_BINDING_CODE_COUNT];[s playerActionChanged:s.actionTargets[i]];}
 RuntimeActionBindings allRows=s.playerActions;[displaySettings applySettings:nil];assert(action_bindings_equal(&allRows,&probeActions));assert(controller_bindings_equal(&menuBefore,&probeController));
 action_bindings_default(&probeActions);display_input_load();assert(action_bindings_equal(&allRows,&probeActions));
 [displaySettings showSettings:nil];[s resetPlayerActions:nil];[displaySettings cancelSettings:nil];assert(action_bindings_equal(&allRows,&probeActions));
 probeActionsUnavailable=1;[displaySettings showSettings:nil];assert(!s.actionTargets[0].enabled&&!s.actionExample.enabled);[displaySettings cancelSettings:nil];probeActionsUnavailable=0;
 // All Buttons: a global 16-button draft that survives Apply and reload.
 [displayDefaults removeObjectForKey:@"allButtonBindingsV1"];button_bindings_default(&probeButtons);display_input_load();
 [displaySettings showSettings:nil];for(unsigned i=0;i<BUTTON_BINDING_COUNT;i++)assert(s.gameButtons.target[i]==i);
 [s.buttonTargets[0] selectItemAtIndex:2];[s buttonChanged:s.buttonTargets[0]];assert(s.gameButtons.target[0]==1);
 RuntimeButtonBindings buttonsBefore=probeButtons;[displaySettings cancelSettings:nil];assert(button_bindings_equal(&buttonsBefore,&probeButtons));
 [displaySettings showSettings:nil];[s.buttonTargets[0] selectItemAtIndex:2];[s buttonChanged:s.buttonTargets[0]];[s.buttonTargets[11] selectItemAtIndex:0];[s buttonChanged:s.buttonTargets[11]];[displaySettings applySettings:nil];
 assert(probeButtons.target[0]==1&&probeButtons.target[11]==BUTTON_BINDING_NONE);
 button_bindings_default(&probeButtons);display_input_load();assert(probeButtons.target[0]==1&&probeButtons.target[11]==BUTTON_BINDING_NONE);
 [displaySettings showSettings:nil];assert(s.buttonTargets[0].indexOfSelectedItem==2);[s resetAllButtons:nil];[displaySettings cancelSettings:nil];assert(probeButtons.target[0]==1&&probeButtons.target[11]==BUTTON_BINDING_NONE);
 [displaySettings showSettings:nil];[s resetAllButtons:nil];probeRejectButtons=1;[displaySettings applySettings:nil];assert(displaySettings.window.sheetParent&&s.error.stringValue.length);probeRejectButtons=0;[displaySettings cancelSettings:nil];
 RuntimeButtonBindings badButtons;assert(!input_button_decode(@[],&badButtons));NSMutableArray *badButtonValues=[input_button_values(&probeButtons) mutableCopy];badButtonValues[0]=@16;assert(!input_button_decode(badButtonValues,&badButtons));badButtonValues[0]=@255;assert(input_button_decode(badButtonValues,&badButtons));
 for(NSNumber *bad in @[@1.5,@(-1),@4294967297ULL]){badButtonValues[0]=bad;assert(!input_button_decode(badButtonValues,&badButtons));}
 // If player actions fail after a global-map change, restore all live input layers.
 [displaySettings showSettings:nil];RuntimeButtonBindings stableButtons=probeButtons;RuntimeKeyboardBindings stableKeys=probeKeyboard;
 [s.buttonTargets[3] selectItemAtIndex:2];[s buttonChanged:s.buttonTargets[3]];[s resetPlayerActions:nil];probeRejectActions=1;[displaySettings applySettings:nil];
 assert(displaySettings.window.sheetParent&&button_bindings_equal(&stableButtons,&probeButtons)&&controller_bindings_equal(&menuBefore,&probeController)&&!memcmp(&stableKeys,&probeKeyboard,sizeof stableKeys));probeRejectActions=0;[displaySettings cancelSettings:nil];
 if(out){capture_input_tab(5,[@(out) stringByAppendingPathComponent:@"player-actions.png"]);capture_input_tab(6,[@(out) stringByAppendingPathComponent:@"all-buttons.png"]);}
 [displayDefaults removePersistentDomainForName:suite];[window orderOut:nil];puts("PASS: capture scope, keyboard/mouse/controller/all-buttons drafts, validation, apply/cancel/reset and preference reload");
}}
