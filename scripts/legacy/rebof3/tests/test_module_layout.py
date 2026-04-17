from __future__ import annotations

import unittest
from pathlib import Path

from scripts.rebof3.config import ROOT

NON_EMI_MODULE_FAMILIES = {"logo"}
ALLOWED_SRC_ONLY_FAMILIES = {"commu00", "game", "logo", "sce10eff", "scena16"}


def expected_module_archives() -> tuple[set[str], dict[str, set[str]]]:
    extracted_root = ROOT / "build" / "extracted" / "BIN"
    top_level_archives: set[str] = set()
    world_archives: dict[str, set[str]] = {}

    for family_dir in extracted_root.iterdir():
        if not family_dir.is_dir():
            continue
        emi_names = {
            path.stem.lower() for path in family_dir.glob("*.EMI") if path.is_file()
        }
        if not emi_names:
            continue
        family_name = family_dir.name.lower()
        if family_name.startswith("world"):
            world_archives[family_name] = emi_names
        else:
            top_level_archives.update(emi_names)

    return top_level_archives, world_archives


class ModuleLayoutTests(unittest.TestCase):
    def test_module_layout_matches_emi_archive_layout(self) -> None:
        top_level_archives, world_archives = expected_module_archives()
        world_names = set(world_archives)

        for module_root in (
            ROOT / "bof3" / "src" / "modules",
            ROOT / "bof3" / "stubs" / "modules",
        ):
            for path in sorted(module_root.iterdir()):
                if not path.is_dir():
                    continue
                name = path.name

                if name.startswith("world"):
                    self.assertIn(
                        name,
                        world_names,
                        f"{path} does not match an extracted WORLDxx family",
                    )
                    for archive_dir in sorted(path.iterdir()):
                        if not archive_dir.is_dir():
                            continue
                        self.assertRegex(archive_dir.name, r"^area[0-9]{3}$")
                        self.assertIn(
                            archive_dir.name,
                            world_archives[name],
                            f"{archive_dir} is not present in extracted disk layout",
                        )
                    continue

                self.assertNotRegex(
                    name,
                    r"^area[0-9]{3}$",
                    f"{path} should be nested under its world family",
                )
                if name in NON_EMI_MODULE_FAMILIES:
                    continue
                self.assertIn(
                    name,
                    top_level_archives,
                    f"{path} does not match any extracted non-world EMI archive",
                )

    def test_src_only_module_families_are_known_promoted_families(self) -> None:
        src_root = ROOT / "bof3" / "src" / "modules"
        stub_root = ROOT / "bof3" / "stubs" / "modules"
        src_families = {path.name for path in src_root.iterdir() if path.is_dir()}
        stub_families = {path.name for path in stub_root.iterdir() if path.is_dir()}

        self.assertEqual(
            src_families - stub_families,
            ALLOWED_SRC_ONLY_FAMILIES,
        )
        self.assertEqual(stub_families - src_families, set())


if __name__ == "__main__":
    unittest.main()
