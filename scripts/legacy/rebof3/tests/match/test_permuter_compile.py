from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from scripts.rebof3.match import permuter_compile as MODULE


class PermuterCompileTests(unittest.TestCase):
    def test_rewrite_compile_command_replaces_source_and_output(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            source = root / "func.c"
            input_c = root / "perm" / "base.c"
            output = root / "perm" / "base.o"
            source.write_text("", encoding="utf-8")
            input_c.parent.mkdir(parents=True, exist_ok=True)

            command = MODULE.rewrite_compile_command(
                f'cc -Ifoo -o "{root / "out.o"}" -c "{source}"',
                source_file=source,
                input_c=input_c,
                output=output,
            )

        self.assertIn(str(input_c.resolve()), command)
        self.assertIn(str(output.resolve()), command)

    def test_rewrite_compile_entry_supports_arguments_form(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            root = Path(tmp_dir)
            source = root / "func.c"
            input_c = root / "perm" / "base.c"
            output = root / "perm" / "base.o"
            source.write_text("", encoding="utf-8")
            input_c.parent.mkdir(parents=True, exist_ok=True)

            command = MODULE.rewrite_compile_entry(
                {
                    "arguments": [
                        "cc",
                        "-Ifoo",
                        "-o",
                        str(root / "out.o"),
                        "-c",
                        str(source),
                    ]
                },
                source_file=source,
                input_c=input_c,
                output=output,
            )

        self.assertIn(str(input_c.resolve()), command)
        self.assertIn(str(output.resolve()), command)


if __name__ == "__main__":
    unittest.main()
