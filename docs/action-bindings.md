# Player action and all-button bindings (TOTK 1.0.0 Mac)

Two independent layers sit on top of the host controller:

1. **All Buttons** remaps the 16 buttons the game finally sees. It therefore
   changes menus, dialogue, the map and gameplay together, including Plus and
   Minus. It is applied to the Npad state in the macOS runtime, before the game
   builds its internal 32-bit mask; no guest code is patched and the layer works
   on every supported game version.
2. **Player Actions** patches the operation masks built by `main+0x1ccf980`.
   It changes only gameplay; the raw mask consumed by menus, dialogue and maps
   is untouched. All 16 gameplay sources are exposed: A, B, X, Y, L, R, ZL, ZR,
   D-pad up/down/left/right, L/R-stick clicks, Plus and Minus. Each source selects one of 17
   codes: Unassigned, Interact, Cancel/Run, Jump, Attack, Ability, Throw,
   Shield/Aim, Bow, Quick select up/down/left/right, Crouch, Scope, Pause/Inventory and Map.

Applying All Buttons first and Player Actions second gives independent menu and
gameplay layouts: All Buttons defines what the game sees, and Player Actions
chooses what each of those game-visible buttons does in gameplay. When All
Buttons is left at defaults, Player Actions behaves exactly like the earlier
gameplay-only feature.

## Player operation sites

The original 26 edge/held operation words, entry load at `0x1ccf96c`,
continuation at `0x1ccf9f4`, three gameplay-UI request loads,
and the gameplay query publication at `0xc7a0dc` must match
before anything changes (32 guards). A single
entry branch redirects translation to checked text padding at aligned text-end
+512, after the existing aspect, bounce and cloth veneers. The original 26 words
stay intact. The translator reads the device's pressed (`+8`), held (`+0x114`)
and released (`+12`) masks, snapshots all 16 mappings from a sequence-checked
table at writable data-page +64, and builds w22/w21/w20 respectively.
It preserves x8-x17, SP, LR and NZCV and never writes the device object.

The table contains an atomic odd/even sequence and 16 atomic 16-bit codes.
A reader uses acquire loads and a read barrier, retrying a concurrent Apply;
the serialized writer publishes the odd marker before changing codes, then the
even marker with release ordering. Pause/Map are handled by the gameplay screen
router and excluded from player operation masks.
Attack/ability release bits now follow their assigned source instead of raw
Y/L. Stick clicks can drive held actions such as running, shielding or charging;
Crouch/Scope remain press-edge target actions, as in the game. The downstream
operation dispatcher and its contextual restrictions remain unchanged.
Code is installed before RX sealing; Apply only publishes the writable table.
No menu detection, shared raw-mask rewrite, live executable-memory rewrite or
on-disk game modification is used.

The gameplay screen-request router separately consumes raw-shaped input at
`0x10635bc` (press), `0x10635dc` (release), and `0x1063b64` (held Bow gate).
Those three loads use a second translator driven by the same table.
This prevents a remapped D-pad press from also opening its old shortcut and
lets a new source request the desired shortcut. The router translates Plus/Minus too: when either is assigned Jump, it no longer
requests its original screen in gameplay. Original menu/dialogue/map readers
keep the unmodified device object. The Bow byte load is widened before translation
so a high-bit source such as a D-pad button can be mapped to Bow.

The generic UI query publishes pressed/held/released/repeat/combined masks in
`0xc7a034`. Only `_LocalModule_31bc5067250e.module`, identified as the gameplay
ability trigger, gets the complete action translation at publication. The
SpecialPower/Material/Weapon/Shield/Bow shortcut modules additionally recognize
the custom trigger in their held/release gate. Original shortcut controls remain
available, and releasing one trigger cannot close a shortcut while another is
held. Other query modules, including ordinary menus and transitions between
shortcuts, pass through unchanged. The adapter preserves the live GPRs and flags;
no shared input device is modified.

The feature refuses unsupported versions and iOS/export builds. Saved arrays are
validated, and failed Apply does not save rejected actions. Legacy local
`TOTK_ACTION_LAYOUT=xbox` remains an initial fallback without valid saved
actions. `TOTK_ACTION_MAP` is a diagnostic 16-value override; legacy 14-value maps
retain default Plus/Minus actions. Preferences use `playerActionBindingsV3`;
valid V2 (14 entries) and V1 (three face buttons) migrate on Apply.

## All-button layer

The runtime builds one Npad full-key state in
`games/totk/runtime/input.c` (`pad_states`). Keyboard bindings, synthetic taps
and the mapped gamepad buttons are OR-ed into its 64-bit `buttons` field. The
All Buttons table is the final 16-bit permutation applied to that field; bits
above 15 (host-only/touchpad state) are preserved. Each source can select one
of the 16 buttons or Unassigned; two sources may select the same button. Player Actions now covers these 16 sources independently in gameplay as well.
Plus/Minus use the gameplay screen-request router instead of the original
14-code player operation dispatcher, preserving their ordinary menu path.

The game-side input pipeline was verified to have one Npad source:

- `main+0x29b4508` is the only game call to `nn::hid::GetNpadStates` for the
  full-key style; the host implements that service.
- `main+0x23f9038` converts the raw Npad bits to the internal mask. The exact
  mapping is recorded in
  `/Volumes/mac-APFS/MacNativeGames-artifacts/analysis/totk-v1.0.0/input_mask_mapping.tsv`:
  A bit0, B bit1, ZL bit2, X bit3, Y bit4, ZR bit5, R-stick bit6, L-stick bit7,
  Minus bits 9+12, Plus bits 10+11, L bit13, R bit14, Up bit16, Down bit17,
  Left bit18, Right bit19. Home/Capture and SL/SR are not part of this mask.
- `main+0x23fa1e4` merges the device masks into the container at `+0x114` and
  applies the game's own one-bit suppression at `+0x10c`.
- `main+0x1ccf980` builds the player operation masks from the edge (`+0x8`) and
  held (`+0x114`) masks. Menus, dialogue and maps read `+0x114` directly, which
  is why a shared raw-mask rewrite changes them. The optional All Buttons layer
  provides a version-independent global map, while Player Actions deliberately
  uses the separate player and gameplay-UI request paths.

## Verification

- `tests/test_action_bindings.py` executes the production translator on native ARM64
  for every source/target, pressed/held/released and combined inputs, verifies
  registers/flags, live reset after RX sealing, concurrent Apply and all-or-nothing guards, checks
  that the raw device object is unchanged, and rejects a 1.1.2 build.
- `tests/test_input_settings.py` executes the AppKit editor with
  `TOTK_RUN_APPKIT_PROBES=1`: action drafts, All Buttons drafts/apply/cancel/
  persistence, malformed settings and failed-Apply rollback.
- Device evidence (2026-10-22):
  - run6: A/B/X/Y/L-stick/R-stick Player Action sources, with menus unchanged.
  - run7: All Buttons Plus↔Minus swap; raw Minus opened the pause menu, raw
    Plus opened the map, raw B returned from both.
  - run8: All Buttons A↔B applied live; raw A closed the map (game saw B=Return),
    raw B opened Stamp/Pin (game saw A=Select), raw A while moving sprinted
    (game saw B=Cancel/Run).
  Evidence directories:
  `~/Library/Application Support/MacNativeGames/TOTK/tests/semantic-input-20261021/run6`,
  `run7` and `run8` (screenshots and console logs).

## Known limits

- Static UI labels such as "Select A" / "Return B" are game text and do not
  change with a remap; behavior changes, the printed icon does not. In-world
  prompts likewise may keep the original icon.
- Home and Capture are system buttons and are not delivered in the game's
  FullKey mask; SL/SR belong to Joy-Con sideways styles that the Mac runtime
  does not present. The 16-button table covers every button the game receives.
- The game's contextual restrictions still apply: a bound action requires its
  normal equipment/state. Other game versions and iOS/export builds retain
  original player actions. Home/Capture and analog stick directions are not
  additional digital FullKey buttons; stick swapping/look inversion remain on
  the Controller page.
- No keys, game images, extracted resources, private saves or screenshots are
  committed.

## Integration follow-up (2026-10-22)

The expanded local work is integrated into PR #141 with release/held translation
completed, strict preference decoding and V1 migration, and coherent sequence-checked
live binding snapshots. The focused input probe checks all 16 global sources and
targets through the production Npad sampler, including unassignment, merged
sources, release and untouched analog/high bits. AppKit verifies every action
row, persistence/Cancel and rollback across the two mapping layers. Earlier
run6/run7/run8 evidence above belongs to the supplied research build; validation
of this integration is recorded separately outside the source checkout.

## Current integration validation (2026-10-22)

The native ARM probe covers every one of 16 sources and 17 targets, including
Pause/Map, edge/held/release, the screen router, module-specific query adapter,
registers/flags and concurrent publication. AppKit covers all 16 action rows,
V1/V2 migration to V3, Apply/Cancel/persistence and rejected-Apply rollback.

Independent gameplay runs are stored outside this checkout under
`~/Library/Application Support/MacNativeGames/TOTK/tests/`:

- `all-button-bindings-final-core-20261022`: L/R/ZL/ZR and all four D-pad
  sources set to Jump, without original shortcut/ability side effects; L set to
  Material shortcut with held input, original Y sorting and B back; Up set to
  Ability; A set to Jump while raw A still confirms in the inventory.
- `sixteen-action-bindings-20261022`: final 16-source runtime; Plus/Minus
  gameplay and original menu controls are recorded in screenshots and console.

These are local game tests through the runtime's diagnostic input path, not
physical certification of every controller model or every equipment state.
