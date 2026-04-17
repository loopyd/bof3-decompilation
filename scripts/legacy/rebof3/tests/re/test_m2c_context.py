from __future__ import annotations

import tempfile
import unittest
from pathlib import Path
from subprocess import CompletedProcess
from unittest.mock import patch

from scripts.rebof3.re.services import m2c_context as MODULE


class M2CContextTests(unittest.TestCase):
    def test_build_m2c_context_source_uses_internal_header_and_repo_prototypes(
        self,
    ) -> None:
        def fake_find_source_mapping(entry: str, **_: object):
            if entry == "0x80161fdc":
                return {
                    "source_file": "bof3/src/core/emi/func_80161fdc.c",
                    "source_signature": "void func_80161fdc(s32 arg0)",
                }
            if entry == "0x80162178":
                return {
                    "source_file": "bof3/src/core/emi/func_80162178.c",
                    "source_signature": "void func_80162178(void)",
                }
            if entry == "0x80162b08":
                return {
                    "source_file": "bof3/src/core/emi/func_80162b08.c",
                    "source_signature": "s32 func_80162b08(u8 slot)",
                }
            return None

        with patch.object(
            MODULE.source_map,
            "find_source_mapping",
            side_effect=fake_find_source_mapping,
        ):
            text, metadata = MODULE.build_m2c_context_source(
                source_text="build/extracted/SLUS_004.22",
                requested_address=0x80161FDC,
                selected_asm_text=(
                    ".text\n"
                    "jal func_80162178\n"
                    "jal func_80162b08\n"
                    "lui a0, %hi(LAB_80162034)\n"
                ),
                program_name="SLUS_004.22",
            )

        self.assertIn('#include "bof3/psyq_compat.h"', text)
        self.assertIn("#include <libcd.h>", text)
        self.assertIn('#include "bof3/src/core/emi/internal.h"', text)
        self.assertIn("void func_80162178(void);", text)
        self.assertIn("s32 func_80162b08(u8 slot);", text)
        self.assertEqual(metadata["internal_header"], "bof3/src/core/emi/internal.h")
        self.assertEqual(metadata["prototype_count"], 2)

    def test_build_m2c_context_preprocess_command_writes_to_output_path(self) -> None:
        command = MODULE.build_m2c_context_preprocess_command(
            source_path=Path("/tmp/ctx.c"),
            output_path=Path("/tmp/ctx.i"),
        )

        self.assertIn("-P", command)
        self.assertIn("-o", command)
        self.assertIn("/tmp/ctx.i", command)
        self.assertEqual(command[-1], "/tmp/ctx.c")

    def test_generate_m2c_context_artifacts_writes_source_and_metadata(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            source_path = Path(tmp_dir) / "func.m2c.ctx.c"
            output_path = Path(tmp_dir) / "func.m2c.ctx.i"

            with (
                patch.object(
                    MODULE,
                    "build_m2c_context_source",
                    return_value=(
                        '#include "bof3/psyq_compat.h"\n',
                        {
                            "status": "ok",
                            "internal_header": None,
                            "prototype_count": 0,
                            "psyq_headers": [],
                        },
                    ),
                ),
                patch.object(
                    MODULE,
                    "run_command",
                    side_effect=lambda command: (
                        output_path.write_text("typedef int s32;\n", encoding="utf-8"),
                        CompletedProcess(
                            args=command, returncode=0, stdout="", stderr=""
                        ),
                    )[1],
                ),
            ):
                metadata = MODULE.generate_m2c_context_artifacts(
                    source_text="build/extracted/SLUS_004.22",
                    requested_address=0x80161FDC,
                    selected_asm_text=".text\n",
                    context_source_path=source_path,
                    context_preprocessed_path=output_path,
                    program_name="SLUS_004.22",
                )

            self.assertEqual(
                source_path.read_text(encoding="utf-8"),
                '#include "bof3/psyq_compat.h"\n',
            )
            self.assertEqual(
                output_path.read_text(encoding="utf-8"), "typedef int s32;\n"
            )
            self.assertEqual(metadata["status"], "ok")
            self.assertTrue(str(metadata["path"]).endswith("func.m2c.ctx.i"))


if __name__ == "__main__":
    unittest.main()
