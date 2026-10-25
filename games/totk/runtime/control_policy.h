/* The Mac control menu's choices (runtime/game_controls.h) as pure C, so tests/test_control_policy.c checks exactly
   what the menu, the game patches and the graphics flags use. Every choice has a "not set" state that leaves the
   launcher's environment and the keys in charge: a player who never applies the menu runs exactly as before. */
#pragma once
#include <stdint.h>
#include "../third_party/d2r_gfx/gfx/nvn/totk_mfx_mode.h"

/* Target resolution (the game's canvas) and the MetalFX spatial preset (the share of it the 3D scene renders at). The
   ratios are this menu's presets, not Apple quality enums; sizes stay exactly 16:9, the scene in 16 x 9 steps. */
enum { CONTROL_SIZES = 4, CONTROL_QUALITIES = 4 };
static const unsigned control_sizes[CONTROL_SIZES][2] = {{1280, 720}, {1600, 900}, {1920, 1080}, {2560, 1440}};
static inline int totk_control_size(unsigned w, unsigned h) { return w >= 640 && w <= 2560 && h >= 360 && h <= 1440 && w * 9 == h * 16; }
static inline uint64_t totk_control_render(unsigned w, unsigned h, int quality) {   /* scene w | h << 32, or 0 */
    if (!totk_control_size(w, h) || quality < 0 || quality >= CONTROL_QUALITIES) return 0;
    static const unsigned numerator[] = {1, 2, 10, 1}, denominator[] = {1, 3, 17, 2};   /* off 100%, quality 2/3, balanced 10/17, performance 1/2 */
    unsigned units = (w * numerator[quality] / denominator[quality]) / 16;
    if (units < 40) units = 40;
    return (uint64_t)(units * 16) | ((uint64_t)(units * 9) << 32);
}
/* One request word for runtime_render_configure: canvas w (12 bits), h (12), scene w (12), h (12), preset (4). */
static inline uint64_t totk_control_request(unsigned w, unsigned h, int quality) {
    uint64_t scene = totk_control_render(w, h, quality);
    return scene ? w | ((uint64_t)h << 12) | ((scene & 4095) << 24) | (((scene >> 32) & 4095) << 36) | ((uint64_t)quality << 48) : 0;
}

/* Real frame-rate limit: the uncapped clock's ceiling (TOTK_UNCAPPED_HZ at run time, "flag uncapped_hz"). 0 is the
   menu's "no limit", the highest ceiling that clock takes; the clock itself was never unbounded (hle_nvn.c UNCAPPED_HZ). */
enum { CONTROL_CAP_MIN = 15, CONTROL_CAP_MAX = 240 };
static inline int control_cap_valid(int cap) { return cap == 0 || (cap >= CONTROL_CAP_MIN && cap <= CONTROL_CAP_MAX); }
static inline int control_cap_hz(int cap) { return cap ? cap : CONTROL_CAP_MAX; }

/* Frame interpolation in the menu's order. Auto starts at half the refresh rate (hle_nvn.c nvn_fi_auto). Display rate and
   30+30 pace on the display link, so vsync is on for them whatever the checkbox says; uncapped leaves vsync and the
   limit to the player. */
enum { CONTROL_FI_OFF, CONTROL_FI_AUTO, CONTROL_FI_DISPLAY, CONTROL_FI_30, CONTROL_FI_UNCAPPED, CONTROL_FI_TIERS };
static inline int control_fi_mode(int tier) {
    static const int mode[CONTROL_FI_TIERS] = {MFX_OFF, MFX_HALF, MFX_HALF, MFX_30, MFX_MAX};
    return tier >= 0 && tier < CONTROL_FI_TIERS ? mode[tier] : MFX_OFF;
}
static inline int control_fi_tier(int mode, int automatic) {
    if (automatic) return CONTROL_FI_AUTO;
    return mode == MFX_HALF ? CONTROL_FI_DISPLAY : mode == MFX_30 ? CONTROL_FI_30 : mode == MFX_MAX ? CONTROL_FI_UNCAPPED : CONTROL_FI_OFF;
}
static inline int control_fi_paces(int tier) { return tier == CONTROL_FI_AUTO || tier == CONTROL_FI_DISPLAY || tier == CONTROL_FI_30; }

/* Application shortcuts: a Mac key code plus Command / Option / Control / Shift bits, packed as (code + 1) | mods << 8
   (0 = unassigned). The defaults are the keys the game already had (1-7, key 3 the picture mode, key 5 TAA) and 8 for
   the menu (F1 needs fn on a Mac laptop); the upscale preset cycle is new and unassigned. */
enum { CONTROL_SHORTCUT_MENU, CONTROL_SHORTCUT_HUD, CONTROL_SHORTCUT_INTERP, CONTROL_SHORTCUT_PICTURE, CONTROL_SHORTCUT_ASPECT,
       CONTROL_SHORTCUT_TAA, CONTROL_SHORTCUT_PAD, CONTROL_SHORTCUT_MOUSE, CONTROL_SHORTCUT_PRESET, CONTROL_SHORTCUTS };
enum { CONTROL_MOD_COMMAND = 1, CONTROL_MOD_OPTION = 2, CONTROL_MOD_CONTROL = 4, CONTROL_MOD_SHIFT = 8 };
static const unsigned control_shortcut_defaults[CONTROL_SHORTCUTS] = {28, 18, 19, 20, 21, 23, 22, 26, 0x7fffffff};
static inline uint64_t control_hotkey(unsigned code, unsigned mods) { return code < 128 && mods < 16 ? (code + 1) | ((uint64_t)mods << 8) : 0; }
static inline uint64_t control_shortcut_default(unsigned action) {
    return action < CONTROL_SHORTCUTS && control_shortcut_defaults[action] < 128 ? control_hotkey(control_shortcut_defaults[action], 0) : 0;
}
static inline int control_hotkey_valid(uint64_t key) { return !key || ((key & 255) >= 1 && (key & 255) <= 128 && (key >> 8) < 16); }
/* Command-Q and Command-, belong to the app menu. */
static inline int control_hotkey_reserved(uint64_t key) { return key == control_hotkey(12, CONTROL_MOD_COMMAND) || key == control_hotkey(43, CONTROL_MOD_COMMAND); }
/* A plain key a controls.txt keyboard binding drives (targets[] = ControlsLayout.keys) cannot also be a shortcut. */
static inline int control_hotkey_conflicts(uint64_t key, const uint8_t targets[128]) { return key && !(key >> 8) && targets[(key & 255) - 1]; }
/* Valid, no reserved chord, no duplicate and no gameplay key: what Apply accepts. Returns the first bad index + 1 or 0. */
static inline int control_shortcuts_check(const uint64_t keys[CONTROL_SHORTCUTS], const uint8_t targets[128]) {
    for (unsigned i = 0; i < CONTROL_SHORTCUTS; i++) {
        if (!control_hotkey_valid(keys[i]) || control_hotkey_reserved(keys[i]) || control_hotkey_conflicts(keys[i], targets)) return (int)i + 1;
        for (unsigned j = 0; j < i; j++) if (keys[i] && keys[i] == keys[j]) return (int)i + 1;
    }
    return 0;
}
