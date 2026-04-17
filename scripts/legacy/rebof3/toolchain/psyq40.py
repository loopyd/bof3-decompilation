from __future__ import annotations

from . import psyq


build_parser = psyq.build_original_parser
parse_args = psyq.parse_original_args
main = psyq.main_original


if __name__ == "__main__":
    raise SystemExit(main())
