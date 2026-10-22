"""Core action-layer regression: execute generated veneers on the native ARM CPU."""
from pathlib import Path
import os
import platform
import subprocess
import tempfile
import unittest

@unittest.skipUnless(platform.system() == "Darwin" and platform.machine() == "arm64", "Native ARM64 macOS veneers")
class PlayerActions(unittest.TestCase):
    def test_live_actions_and_version_guard(self):
        root = Path(__file__).resolve().parents[1]
        env = {k: v for k, v in os.environ.items() if k not in ("TOTK_INPUT_FILE", "TOTK_INPUT_SCRIPT", "TOTK_ACTION_LAYOUT", "TOTK_EXPORT_IOS", "TOTK_ACTION_MAP", "TOTK_BUTTON_MAP")}
        with tempfile.TemporaryDirectory() as folder:
            for defines in ([], ["-DTOTK_GAME_V112=1"]):
                exe = Path(folder) / ("actions-old" if defines else "actions")
                command = ["xcrun", "clang", "-std=c11", "-ffunction-sections", "-fdata-sections", "-Wl,-dead_strip", *defines,
                           str(root / "tests/test_action_bindings.c"), str(root / "tests/action_stub_probe.S"), str(root / "games/totk/runtime/semantic_action_stub.S"), "-o", str(exe)]
                build = subprocess.run(command, capture_output=True, text=True, timeout=60)
                self.assertEqual(build.returncode, 0, build.stdout + build.stderr)
                run = subprocess.run([str(exe)], capture_output=True, text=True, env=env, timeout=15)
                self.assertEqual(run.returncode, 0, run.stdout + run.stderr)
                self.assertIn("PASS:", run.stdout)
