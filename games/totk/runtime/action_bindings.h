/* Player operation codes, distinct from Npad/UI button bits. Only the verified
   A/X/Y operation sites are configurable; B keeps its original run/cancel path. */
#pragma once
#include <stdint.h>
#include <string.h>
enum { ACTION_INTERACT=1, ACTION_JUMP=4, ACTION_ATTACK=8, ACTION_BINDING_COUNT=3 };
typedef struct { uint8_t target[ACTION_BINDING_COUNT]; } RuntimeActionBindings; /* raw A, X, Y */
static inline void action_bindings_default(RuntimeActionBindings *b){*b=(RuntimeActionBindings){{ACTION_INTERACT,ACTION_JUMP,ACTION_ATTACK}};}
static inline int action_bindings_valid(const RuntimeActionBindings *b){
 for(unsigned i=0;i<ACTION_BINDING_COUNT;i++)if(b->target[i]!=0&&b->target[i]!=ACTION_INTERACT&&b->target[i]!=ACTION_JUMP&&b->target[i]!=ACTION_ATTACK)return 0;return 1;
}
static inline uint32_t action_bindings_pack(const RuntimeActionBindings *b){return b->target[0]|(uint32_t)b->target[1]<<4|(uint32_t)b->target[2]<<8;}
static inline RuntimeActionBindings action_bindings_unpack(uint32_t n){return (RuntimeActionBindings){{n&15,(n>>4)&15,(n>>8)&15}};}
static inline const char *action_binding_name(unsigned n){return n==1?"interact":n==4?"jump":n==8?"attack":"none";}
