# Game Controls menu (Mac)

Press **8** (a rebindable shortcut), press **Command+,**, or choose **Game Controls…**
from the app menu. One sheet with seven pages; nothing changes until **Apply**,
and **Cancel** discards the draft. The sheet is compiled only into the Mac frontend
(`runtime/game_controls.h`, included by `frontend.m`, which the iOS target excludes);
shared runtime code gains plain setters only, no references to the menu.

**Nothing changes for a player who does not use it.** A setting is saved and applied
only after the player changes it and presses Apply. Every page opens on what is in
force now (the launcher's environment, keys 1–7, `controls.txt`), so applying without
edits saves nothing, and an option that was never changed is never written. The
default shortcuts are the keys the game already had (1–7) plus 8 for the menu (F1
needs fn on a Mac laptop).

The HUD's shortcut list shows the keys bound on the Shortcuts page, with 8 for the menu
at the top. Key 1 now hides the list first and keeps the readings (FPS, shaders,
pipelines, load); a second press hides everything, a third shows it all again.

| Page | Contents | Stored in | Applied through |
| --- | --- | --- | --- |
| Video | target resolution (Default, 720p, 900p, 1080p, 1440p), MetalFX spatial preset (off 100 %, quality ≈67 %, balanced ≈59 %, performance 50 %), antialiasing (FXAA / MetalFX TAA, 1.4.3), window mode (windowed, full screen, borderless) | display defaults | `runtime_render_configure` (latched at the present boundary, like key 3), `taa_toggle`, the window |
| FPS | vertical sync, real frame-rate limit (0 or 15–240), frame interpolation (off, auto, up to the display rate, 30+30, uncapped) | display defaults | graphics flags `vsync`, `uncapped_hz`, `mfx_interp`, `mfx_auto` |
| Shortcuts | the menu, keys 1–7's actions and the upscale preset cycle, each a key or chord | display defaults | the keys' own handlers |
| Keyboard | two keys or mouse buttons per action, right-drag look, mouse Y inversion, sensitivity 1–100 | `controls.txt` | `runtime_keyboard_layout_set`, `runtime_controller_layout_set` |
| Controller | presets (Switch, Xbox, PlayStation, Generic), all 22 physical buttons with hold / tap / toggle / aim, swap sticks, invert look Y | `controls.txt` | `runtime_controller_layout_set` |
| Actions | the 16 gameplay sources (#545, 1.0.0 and 1.4.3) | `player-actions.txt` | `runtime_action_bindings_set` |
| All Buttons | what the game finally sees for each of its 16 buttons (menus follow) | `controls.txt` `game_<button>` | `runtime_game_buttons_set` |

Display defaults are the NSUserDefaults suite `org.macnativegames.<game>.display`
(`.tests` for agent test sessions); the shortcuts are its `controlShortcuts` array.

## Video

The target resolution is the game's canvas; the preset is the share of it the 3D
scene renders at, which MetalFX rebuilds to the canvas (1080p + Quality renders
1280×720). **Default** leaves key 3's picture modes in charge, and pressing key 3
clears the menu's resolution. On 1.0.0 the render patch is installed only when a
resolution is saved at start, so the first choice in a session applies from the next
start (the status line says so); 1.4.3 applies while playing. Versions without the
patches (1.1.2, 1.2.1, Breath of the Wild) show the page with the resolution
disabled. The window mode is saved only when changed; a full-screen change made with
the green button holds for the session without being saved.

## FPS

These drive the pacing that already exists; there is no second pacing path.

- **Vertical sync** (default on, as before) is the layer's `displaySyncEnabled` and
  the display link. Off presents on the classic thread with no display link.
- **Real frame-rate limit** is the uncapped clock's ceiling (`TOTK_UNCAPPED_HZ` while
  playing). 0 is that clock's maximum, 240. With `TOTK_UNCAPPED=0` the field is
  disabled.
- **Frame interpolation** chooses among the existing modes (key 2's cycle). "Up to the
  display rate" and 30+30 pace on the display link, so vsync is forced on and the
  limit is greyed out. Uncapped interpolates every real frame and leaves vsync and the
  limit to the player. **Auto** starts at half the refresh rate, drops to 30+30 after
  2 s of real frames missing their slot and tries again after 5 s back inside the
  30 fps budget (`TOTK_FI_AUTO=<down ms>,<up ms>`). Pressing key 2 ends auto.

## Input pages

The Keyboard page shows, for each of the 24 targets, the first two keys
(`key_<code>`) or mouse buttons (`mouse_left` … `mouse5`, active in mouse mode)
that drive it. Recording a source that is bound elsewhere moves it here and says
so. Keys 1–7 and Command stay reserved for shortcuts. A shortcut on a plain key that
also drives the game is refused at Apply. Options are `controls.txt` lines, written
only when they differ from the default:

```text
swap_sticks = on          # controllers only
invert_look_y = on        # controllers' look stick
mouse_drag_look = on      # hold the right button and drag to look, outside mouse mode
mouse_sensitivity = 70    # absent: TOTK_MOUSE_SENSITIVITY, or 50
mouse_invert_y = on       # absent: TOTK_MOUSE_INVERT_Y
game_a = b                # the game sees B when A is pressed
game_minus = none         # the game never sees Minus
```

The launcher validates the same file with the same parser
(`runtime/controls_layout.h`). Apply keeps the player's comments, rewrites the
binding lines and refuses to overwrite a file changed elsewhere since the sheet
opened. Files it cannot read produce an error and are never replaced. While the sheet
is open the pointer is released and guest input is neutral.

## Validation

`TOTK_RUN_APPKIT_PROBES=1 python3 -m unittest tests.test_controls_layout tests.test_action_bindings`
covers the option parser and formatter, `control_policy.h` (sizes, presets, limit,
interpolation tiers, shortcut checks), the input layer (host-dispatched shortcuts,
the game-button layer) and the real AppKit sheet: slots, options, comments,
only-changed saving, conflicts, shortcut dispatch and the Actions page. They do not
certify controller models or retail gameplay.
