from __future__ import annotations

import unittest

from scripts.rebof3.toolchain import old_gcc_catalog as MODULE


class OldGccCatalogTests(unittest.TestCase):
    def test_expand_compiler_ids_uses_default_ids_when_selection_missing(self) -> None:
        result = MODULE.expand_compiler_ids(
            None,
            None,
            default_ids=("gcc-2.7.2-psx",),
        )

        self.assertEqual(result, ("gcc-2.7.2-psx",))

    def test_expand_compiler_ids_adds_prefix_ids_per_set(self) -> None:
        result = MODULE.expand_compiler_ids(
            None,
            [MODULE.DEFAULT_OLD_GCC_COMPILER_SET],
            set_prefix_ids={
                MODULE.DEFAULT_OLD_GCC_COMPILER_SET: ("gcc-2.7.2-psx",),
            },
        )

        self.assertEqual(result[0], "gcc-2.7.2-psx")
        self.assertIn("gcc-2.7.0-mipsel", result)

    def test_expand_compiler_ids_dedupes_compilers(self) -> None:
        result = MODULE.expand_compiler_ids(
            ["gcc-2.8.0-psx"],
            [MODULE.DEFAULT_OLD_GCC_COMPILER_SET],
        )

        self.assertEqual(result.count("gcc-2.8.0-psx"), 1)


if __name__ == "__main__":
    unittest.main()
