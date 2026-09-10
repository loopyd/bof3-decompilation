#!/usr/bin/env python3
"""Run local mission diagnosis or audit without launching an agent."""

from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[4] / "tools" / "python"))

from harness.commands.agent_run import main

if __name__ == "__main__":
    raise SystemExit(main())
