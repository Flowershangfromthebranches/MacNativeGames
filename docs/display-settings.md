# macOS control menu

Open **Game Controls…** from the application's menu, click **Game Controls** in
the game window, or press **F1** / **Command+,**. This replaces the passive
shortcut reminder. The menu has Graphics, Frame Rate and Shortcuts tabs.
Apply saves changes; Cancel discards the draft. Full screen uses a separate
macOS Space; borderless fills the current desktop; windowed restores the prior
window frame. No option changes the physical display mode.

## Actual render resolution and MetalFX

Target presets are 1280×720, 1600×900, 1920×1080 and 2560×1440. Both the guest
canvas and Metal drawable use the selected target. Window resizing only scales
the displayed image with its aspect ratio preserved. The upstream ultrawide
camera and HUD path remains available: its fixed 16:9 canvas is expanded to the
selected wide camera ratio when presented. `TOTK_STRETCH=1` still fills the view.
The compact FPS/shader/load panel remains available without the old shortcut
reminder; `TOTK_HUD=corners` keeps the readings in the picture. Unlike the previous
output-only setting, the scene render dimensions now also follow this choice:

| MetalFX preset | Scene size relative to target | At 1920×1080 |
| --- | --- | --- |
| Off | 100% | 1920×1080 |
| Quality | about 2/3 per dimension | 1280×720 |
| Balanced | about 1/1.7 per dimension | 1120×630 |
| Performance | 1/2 per dimension | 960×540 |

Sizes are rounded to an exact 16:9 integer grid with a minimum of 640×360.
These are application-defined render-scale presets, not Apple quality enums.
1080p Quality shades about 44% as many scene pixels as native 1080p, before
upscaling cost. This is a pixel count, not an FPS improvement promise. Shadows,
compute, CPU work and fixed-size passes do not necessarily scale with this ratio.

MetalFX spatial scaling replaces the game's docked upscale before UI composition.
It works with a fixed drawable and does not depend on a larger window. A real
size change is latched at the guest present boundary into the verified render-size
and canvas data pages, then requests the existing game display-layout refresh.
No guest instruction is rewritten during play. The texture/Graphics/application
heap reserve and generated RomFS overlay use the same maximum-scene formula;
the builder regenerates old overlays that lack `scene-render-reserve=1`.

The status line reads scene and canvas dimensions from the actual game upscale
constants, plus the number of successfully encoded MetalFX frames. A requested
checkbox alone is not treated as execution evidence. New scaler configurations
build in the background; the game's existing upscale is retained until ready.

Antialiasing currently preserves the game's FXAA. TAA/temporal scaling is shown
as unavailable because this port does not yet provide validated per-object motion
vectors and projection jitter. It is not emulated by renaming spatial scaling.
Apple documents [spatial and temporal scaling](https://developer.apple.com/documentation/MetalFX)
as distinct paths; [temporal AA and scaling](https://developer.apple.com/videos/play/wwdc2022/10103/)
requires motion, depth and jittered color inputs.

## Frame controls

- Vsync controls the display link and `CAMetalLayer.displaySyncEnabled`.
- Real-frame limit: 0 disables the application limit; 15–240 is an explicit cap.
  Vsync and cap can be used together; their lower rate governs. Non-divisor caps
  use their own clock rather than rounding to a divisor of refresh rate.
- MetalFX frame generation is an independent switch. It does not select 30 FPS
  or half-refresh pacing. Generated frames can increase displayed FPS without
  increasing the rate at which the game responds to input.
- With Vsync off and cap 0, no application pacing period is imposed. Presentation
  backpressure, GPU completion waits and resource lifetime protections remain.

Real-time frame stepping follows elapsed time, including above 120 real FPS
instead of advancing every frame by at least 1/120 second. Probes, iOS and
headless/environment-controlled runs retain their legacy policy unless the
macOS control configuration is activated.

## Shortcuts and preferences

Defaults: F1 opens the menu, 1 toggles overlays, 2 toggles frame generation, 3 cycles
Off/Quality/Balanced/Performance, and 4 toggles the upstream ultrawide camera
on supported versions (1.2.1/1.4.3). Every action can be rebound to a key or chord
in Shortcuts; Delete unassigns it and Escape cancels recording. Duplicate bindings
and reserved Command+Q/Command+, are rejected. Command+, remains a menu fallback.
Reassigned plain keys no longer also trigger the old hardcoded actions.

Preferences use `org.macnativegames.totk.display`; non-user game-slot owners use
the test suite. New configurations default to 1080p Quality, FXAA, Vsync on,
no explicit cap, and frame generation off. Prior window mode is retained.

## Focused verification

- `test_control_policy.py`: actual pixel-count reductions, valid dimensions and
  independent Vsync/limiter periods (including both disabled).
- `frontend_display_probe.m`: production AppKit controls with guest stubs; actual
  Apply actions, mode transitions, aspect ratio, validation, cancel, preferences
  and unchanged physical display pixels. No game data or Metal commands.
- Real-game save-copy evidence, logs and generated files are external under
  `MacNativeGames/TOTK/tests/control-menu-20261019` and the artifacts development
  directory. A user-takeover run is retained separately and not counted as a pass.

Local 1.0.0 / Apple M3 validation on 2026-10-19: all 42 source tests and the
AppKit core probe passed. A real save-copy run completed 10 live combinations,
including native 720p/1080p/1440p, the three 1080p upscale presets, caps of 30 and
40, both pacing controls off, and frame generation with an explicit real-frame
cap. Game constants matched every selected scene/canvas pair; MetalFX counters
increased in every enabled upscale stage. The Vsync-off stages reported
`displaySyncEnabled=0`. Frame generation produced 296 extra frames for 300 real
frames in the recorded sample. The run exited normally, with no GPU fault,
FATAL or translation failure. Representative screenshots were visually reviewed.
These are focused behavior checks with a warm cache, not a controlled FPS
benchmark or a long-session stability claim. All 51 formal profile file hashes,
including 25 `.sav` files, matched the pre-test snapshot.

The menu extends the guarded four-site render patch already merged from
[upstream PR #19](https://github.com/ly05010419/MacNativeGames/pull/19). Its live
layout refresh uses the 1.4.3 display-config flag; the iOS embedded-code guards
remain unchanged. The 1.4.3 patch and
frontend passed syntax checks only; no 1.4.3 game execution was performed.

## Upstream integration

The PR is based on main `7c8e9dd`. It retains the compact performance panel,
`TOTK_HUD=corners`, `TOTK_STRETCH`, the ultrawide camera/HUD path and key 4 (now
also configurable), and the iOS embedded-code guards. Headless probes, startup
verification and iOS exports do not load the macOS menu preferences.

On this rebased branch, the two control-policy/input regression checks, three
work-directory checks, the AppKit core probe and a full 1.0.0 runtime compile
passed. The AppKit probe also checks the preserved compact panel, five shortcut
actions and the wide presentation frame without changing drawable pixels.
The game-run results above describe the earlier installed local build; they
were not repeated on this rebased branch. 1.4.3 game execution, iOS execution
and long-session/FPS benchmarks remain unverified.
