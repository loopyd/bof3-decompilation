from __future__ import annotations

import io
import unittest
from contextlib import redirect_stderr
from types import SimpleNamespace
from unittest import mock

from scripts.rebof3 import main as MODULE


class MainCliTests(unittest.TestCase):
    def test_render_help_hides_inventory_group_without_assets_group(self) -> None:
        help_text = MODULE.render_help()

        self.assertNotIn("inventory:", help_text)
        self.assertNotIn("assets:", help_text)
        self.assertNotIn("search", help_text)

    def test_inventory_group_dispatches_to_inventory_module(self) -> None:
        fake_module = SimpleNamespace(main=mock.Mock(return_value=0))

        with mock.patch.object(
            MODULE.importlib, "import_module", return_value=fake_module
        ) as patched:
            result = MODULE.main(["inventory", "ghidra-symbols", "tmp/raw.json"])

        self.assertEqual(result, 0)
        patched.assert_called_once_with("scripts.rebof3.inventory.inventory")
        fake_module.main.assert_called_once_with(["ghidra-symbols", "tmp/raw.json"])

    def test_inventory_build_help_dispatches_to_inventory_module(self) -> None:
        fake_module = SimpleNamespace(main=mock.Mock(return_value=0))

        with mock.patch.object(
            MODULE.importlib, "import_module", return_value=fake_module
        ) as patched:
            result = MODULE.main(["inventory", "build", "--help"])

        self.assertEqual(result, 0)
        patched.assert_called_once_with("scripts.rebof3.inventory.inventory")
        fake_module.main.assert_called_once_with(["build", "--help"])

    def test_re_group_help_renders_locally(self) -> None:
        result = MODULE.main(["re", "--help"])

        self.assertEqual(result, 0)

    def test_public_command_registry_only_contains_public_entrypoints(self) -> None:
        self.assertIn(("stubs", "sync"), MODULE.PUBLIC_COMMAND_MODULES)
        self.assertIn(("match", "init"), MODULE.PUBLIC_COMMAND_MODULES)
        self.assertIn(("match", "target"), MODULE.PUBLIC_COMMAND_MODULES)
        self.assertIn(("match", "compiler-report"), MODULE.PUBLIC_COMMAND_MODULES)
        self.assertIn(("match", "refresh"), MODULE.PUBLIC_COMMAND_MODULES)
        self.assertIn(("match", "report"), MODULE.PUBLIC_COMMAND_MODULES)
        self.assertIn(("match", "scaffold"), MODULE.PUBLIC_COMMAND_MODULES)
        self.assertIn(("match", "asm-patch"), MODULE.PUBLIC_COMMAND_MODULES)
        self.assertIn(("match", "status"), MODULE.PUBLIC_COMMAND_MODULES)
        self.assertIn(("match", "semantic-diff"), MODULE.PUBLIC_COMMAND_MODULES)
        self.assertIn(("match", "permuter"), MODULE.PUBLIC_COMMAND_MODULES)
        self.assertIn(("match", "view"), MODULE.PUBLIC_COMMAND_MODULES)
        self.assertIn(("re", "setup-old-gcc"), MODULE.PUBLIC_COMMAND_MODULES)
        self.assertNotIn(("re", "decomp-cache"), MODULE.PUBLIC_COMMAND_MODULES)
        self.assertNotIn(("re", "owner-queue"), MODULE.PUBLIC_COMMAND_MODULES)
        self.assertNotIn(("re", "owner-opencode"), MODULE.PUBLIC_COMMAND_MODULES)
        self.assertNotIn(("re", "extract-all-emis"), MODULE.PUBLIC_COMMAND_MODULES)

    def test_match_help_prefers_init_over_workspace_init(self) -> None:
        help_text = MODULE.render_group_help("match")

        self.assertIn("  init", help_text)
        self.assertIn("  candidate-prepare", help_text)
        self.assertIn("  candidate-build", help_text)
        self.assertIn("  candidate-full", help_text)
        self.assertIn("  report", help_text)
        self.assertIn("  scaffold", help_text)
        self.assertIn("  asm-patch", help_text)
        self.assertIn("  semantic-diff", help_text)
        self.assertNotIn("  workspace-init", help_text)

    def test_match_init_dispatches_to_workspace_module_init_mode(self) -> None:
        fake_module = SimpleNamespace(main=mock.Mock(return_value=0))

        with mock.patch.object(
            MODULE.importlib, "import_module", return_value=fake_module
        ) as patched:
            result = MODULE.main(
                ["match", "init", "-p", "/boot/SLUS_004.22", "-e", "0x80162d00"]
            )

        self.assertEqual(result, 0)
        patched.assert_called_once_with("scripts.rebof3.match.workspace")
        fake_module.main.assert_called_once_with(
            ["-p", "/boot/SLUS_004.22", "-e", "0x80162d00"]
        )

    def test_match_workspace_init_remains_supported(self) -> None:
        fake_module = SimpleNamespace(main=mock.Mock(return_value=0))

        with mock.patch.object(
            MODULE.importlib, "import_module", return_value=fake_module
        ) as patched:
            result = MODULE.main(
                [
                    "match",
                    "workspace-init",
                    "-p",
                    "/boot/SLUS_004.22",
                    "-e",
                    "0x80162d00",
                ]
            )

        self.assertEqual(result, 0)
        patched.assert_called_once_with("scripts.rebof3.match.workspace")
        fake_module.main.assert_called_once_with(
            [
                "__workspace_init_compat__",
                "-p",
                "/boot/SLUS_004.22",
                "-e",
                "0x80162d00",
            ]
        )

    def test_match_report_dispatches_to_report_module(self) -> None:
        fake_module = SimpleNamespace(main=mock.Mock(return_value=0))

        with mock.patch.object(
            MODULE.importlib, "import_module", return_value=fake_module
        ) as patched:
            result = MODULE.main(["match", "report", "--match-root", "tmp/matching"])

        self.assertEqual(result, 0)
        patched.assert_called_once_with("scripts.rebof3.match.report")
        fake_module.main.assert_called_once_with(["--match-root", "tmp/matching"])

    def test_match_scaffold_dispatches_to_scaffold_module(self) -> None:
        fake_module = SimpleNamespace(main=mock.Mock(return_value=0))

        with mock.patch.object(
            MODULE.importlib, "import_module", return_value=fake_module
        ) as patched:
            result = MODULE.main(["match", "scaffold", "--limit", "10"])

        self.assertEqual(result, 0)
        patched.assert_called_once_with("scripts.rebof3.match.scaffold")
        fake_module.main.assert_called_once_with(["--limit", "10"])

    def test_match_candidate_prepare_dispatches_to_candidate_prepare_module(self) -> None:
        fake_module = SimpleNamespace(main=mock.Mock(return_value=0))

        with mock.patch.object(
            MODULE.importlib, "import_module", return_value=fake_module
        ) as patched:
            result = MODULE.main(
                ["match", "candidate-prepare", "-p", "/boot/SLUS_004.22", "-e", "0x80162d00"]
            )

        self.assertEqual(result, 0)
        patched.assert_called_once_with("scripts.rebof3.match.candidate_prepare")
        fake_module.main.assert_called_once_with(
            ["-p", "/boot/SLUS_004.22", "-e", "0x80162d00"]
        )

    def test_match_asm_patch_dispatches_to_expected_asm_patch_module(self) -> None:
        fake_module = SimpleNamespace(main=mock.Mock(return_value=0))

        with mock.patch.object(
            MODULE.importlib, "import_module", return_value=fake_module
        ) as patched:
            result = MODULE.main(["match", "asm-patch", "--baseline-asm", "-"])

        self.assertEqual(result, 0)
        patched.assert_called_once_with("scripts.rebof3.match.expected_asm_patch")
        fake_module.main.assert_called_once_with(["--baseline-asm", "-"])

    def test_match_semantic_diff_dispatches_to_semantic_diff_module(self) -> None:
        fake_module = SimpleNamespace(main=mock.Mock(return_value=0))

        with mock.patch.object(
            MODULE.importlib, "import_module", return_value=fake_module
        ) as patched:
            result = MODULE.main(["match", "semantic-diff", "--objdiff-json", "-"])

        self.assertEqual(result, 0)
        patched.assert_called_once_with("scripts.rebof3.match.semantic_diff")
        fake_module.main.assert_called_once_with(["--objdiff-json", "-"])

    def test_hidden_re_plumbing_commands_are_rejected_by_package_entrypoint(
        self,
    ) -> None:
        stderr = io.StringIO()

        with redirect_stderr(stderr):
            result = MODULE.main(["re", "decomp-cache", "upsert"])

        self.assertEqual(result, 1)
        self.assertIn("unknown command: re decomp-cache", stderr.getvalue())

    def test_re_pipeline_decomp_dispatches_to_pipeline_decomp_module(self) -> None:
        fake_module = SimpleNamespace(main=mock.Mock(return_value=0))

        with mock.patch.object(
            MODULE.importlib, "import_module", return_value=fake_module
        ) as patched:
            result = MODULE.main(
                ["re", "pipeline-decomp", "build/extracted/SLUS_004.22", "0x80162d00"]
            )

        self.assertEqual(result, 0)
        patched.assert_called_once_with("scripts.rebof3.re.commands.pipeline_decomp")
        fake_module.main.assert_called_once_with(
            ["build/extracted/SLUS_004.22", "0x80162d00"]
        )


if __name__ == "__main__":
    unittest.main()
