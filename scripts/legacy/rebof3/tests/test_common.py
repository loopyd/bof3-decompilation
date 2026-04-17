from __future__ import annotations

import unittest

from scripts.rebof3 import common as MODULE


class RunCommandTests(unittest.TestCase):
    def test_run_command_replaces_non_utf8_output_bytes(self) -> None:
        result = MODULE.run_command(
            [
                "python3",
                "-c",
                "import sys; sys.stdout.buffer.write(b'\\x81\\n'); sys.stderr.buffer.write(b'\\x81\\n')",
            ]
        )

        self.assertEqual(result.returncode, 0)
        self.assertEqual(result.stdout, "\ufffd\n")
        self.assertEqual(result.stderr, "\ufffd\n")


if __name__ == "__main__":
    unittest.main()
