/* Final mapping of the 16 Switch FullKey buttons. It runs after the keyboard and
   controller layouts, so the whole game (menus, dialogue, map and gameplay)
   sees the same remapped buttons. Sticks and the host-only Home/Capture
   buttons are not part of this FullKey mask. */
#pragma once
#include <stdint.h>
#include <string.h>
enum { BUTTON_BINDING_COUNT = 16, BUTTON_BINDING_NONE = 0xFF };
typedef struct { uint8_t target[BUTTON_BINDING_COUNT]; } RuntimeButtonBindings;
static inline const char *button_binding_name(unsigned i) {
    static const char *names[BUTTON_BINDING_COUNT] = {"A", "B", "X", "Y", "L3", "R3", "L", "R",
        "ZL", "ZR", "+", "-", "Left", "Up", "Right", "Down"};
    return i < BUTTON_BINDING_COUNT ? names[i] : "?";
}
static inline void button_bindings_default(RuntimeButtonBindings *b) {
    for (unsigned i = 0; i < BUTTON_BINDING_COUNT; i++) b->target[i] = (uint8_t)i;
}
static inline int button_bindings_valid(const RuntimeButtonBindings *b) {
    for (unsigned i = 0; i < BUTTON_BINDING_COUNT; i++)
        if (b->target[i] != BUTTON_BINDING_NONE && b->target[i] >= BUTTON_BINDING_COUNT) return 0;
    return 1;
}
static inline int button_bindings_equal(const RuntimeButtonBindings *a, const RuntimeButtonBindings *b) {
    return memcmp(a->target, b->target, sizeof a->target) == 0;
}
/* Bits 16+ are not FullKey buttons and are preserved unchanged. Two sources may
   select the same target; a source set to NONE is dropped. */
static inline uint64_t button_bindings_apply(const RuntimeButtonBindings *b, uint64_t buttons) {
    uint64_t out = buttons & ~0xffffULL;
    for (unsigned i = 0; i < BUTTON_BINDING_COUNT; i++)
        if (buttons & (1ULL << i)) {
            unsigned t = b->target[i];
            if (t < BUTTON_BINDING_COUNT) out |= 1ULL << t;
        }
    return out;
}
