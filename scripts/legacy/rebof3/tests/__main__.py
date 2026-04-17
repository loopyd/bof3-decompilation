#!/usr/bin/env python3

from __future__ import annotations

import unittest
from pathlib import Path


def main() -> int:
    tests_dir = Path(__file__).resolve().parent
    suite = unittest.defaultTestLoader.discover(
        start_dir=str(tests_dir),
        pattern="test_*.py",
        top_level_dir=str(tests_dir.parents[2]),
    )
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    raise SystemExit(main())
