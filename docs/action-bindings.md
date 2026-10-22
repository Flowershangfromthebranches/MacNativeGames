# Player-only action bindings (TOTK 1.0.0 Mac)

Npad remapping also changes menu confirmation. Player bindings instead change
the operation masks built by main+0x1ccf980; raw masks consumed by menus, dialogue
and maps remain unchanged. A/X/Y can select Interact (1), Jump (4), Attack (8),
or no operation (0). B follows its original path.

The six guarded sites are 0x1ccfe28, 0x1ccfc60, 0x1ccfe38 (edge mask w22), and
0x1ccfe90, 0x1ccfc68, 0x1ccfea0 (held mask w21). All original words and scratch
padding must match before anything changes. Each site branches to an eight-word
veneer that saves/restores x16/x17, acquire-loads the selected operation, performs
the original MOV/OR with that selection, and branches back. SP, LR and NZCV are
preserved. Checked text padding at aligned text-end +512 follows the existing
aspect, bounce and cloth veneers. The loader's writable data page at +64 holds
one atomic configuration word. Code is installed before RX sealing; live Apply
only changes writable data. No menu detection, shared raw-mask rewrite, live
executable-memory rewrite or on-disk game modification is used.

The feature refuses unsupported versions and iOS/export builds. Saved arrays are
validated, and failed Apply does not save rejected actions. Legacy local
TOTK_ACTION_LAYOUT=xbox is an initial fallback without valid saved actions.

Core checks:

- tests/test_action_bindings.py executes production veneers on native ARM64 for
  supported operation selections, verifies register/flag preservation and live
  reset after RX sealing, checks all-or-nothing rejection, confirms raw A/B/X/Y
  remain unchanged, and checks that a 1.1.2 build rejects the setting. It loads
  synthetic guarded words only, with no game image or player data.
- tests/test_input_settings.py covers action drafts, Apply/Cancel, persistence,
  unsupported UI, malformed settings and failed-Apply rollback while preserving
  the controller menu map.

The original fixed-layout proof and its game evidence remain outside the source
repository in /Volumes/mac-APFS/MacNativeGames-artifacts/analysis/totk-v1.0.0/REPORT.md
and the user's independent tests/semantic-input-20261021/run4 directory. Those
results do not certify every new live-binding permutation or controller model.
The updated runtime's own evidence is recorded separately in external artifacts.

Known limits: original in-world prompt icons remain. Other scene-specific raw
input consumers may require separate investigation. Other game versions and
ZL/ZR/abilities are outside the verified A/X/Y sites. No keys, game images,
extracted resources, private saves or screenshots are committed.
