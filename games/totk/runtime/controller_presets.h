/* Controller presets operate on normalized physical positions. GameController's
   Nintendo label normalization stays in gamepad_layout.h; do not swap it twice. */
#pragma once
#include "input_bindings.h"
enum { CONTROLLER_PRESET_SWITCH, CONTROLLER_PRESET_XBOX, CONTROLLER_PRESET_PLAYSTATION,
       CONTROLLER_PRESET_GENERIC, CONTROLLER_PRESET_CUSTOM };
static inline int controller_preset_apply(RuntimeControllerBindings *b,unsigned preset) {
    if(preset>=CONTROLLER_PRESET_CUSTOM)return 0;
    RuntimeControllerBindings next={0};controller_map_default(next.map,next.modes);
    if(preset==CONTROLLER_PRESET_XBOX) {
        next.map[0]=1;next.map[1]=2;next.map[2]=4;next.map[3]=8;
    } else if(preset==CONTROLLER_PRESET_PLAYSTATION) {
        next.map[0]=1;next.map[1]=2; /* Cross confirms, Circle returns; Square attacks, Triangle jumps. */
    }
    *b=next;return 1;
}
static inline int controller_bindings_equal(const RuntimeControllerBindings *a,const RuntimeControllerBindings *b) {
    return !memcmp(a->map,b->map,sizeof a->map)&&!memcmp(a->modes,b->modes,sizeof a->modes)
           &&a->swap_sticks==b->swap_sticks&&a->invert_look_y==b->invert_look_y;
}
static inline int controller_preset_matches(const RuntimeControllerBindings *b,unsigned preset) {
    RuntimeControllerBindings expected;return controller_preset_apply(&expected,preset)&&controller_bindings_equal(b,&expected);
}
