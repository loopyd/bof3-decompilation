root := justfile_directory()
python_env := env_var_or_default("VIRTUAL_ENV", root + "/.venv")
python := python_env + "/bin/python"
pythonpath := root + "/tools/python"

default: help

help:
    @just --list

[private]
venv:
    @if [ -n "${VIRTUAL_ENV:-}" ]; then test -x "{{ python }}"; \
    elif [ ! -x "{{ python }}" ]; then UV_CACHE_DIR="{{ root }}/.uv-cache" uv sync --extra dev --frozen; fi

# Initialize retained dependencies and the local analysis environment.
setup: venv
    @PYTHONPATH={{ pythonpath }} {{ python }} -m harness.commands.bootstrap

doctor: venv
    @{{ root }}/bin/symbols check

# Restore reviewed EMI images, then verify every normalized binary is present.
binaries: venv
    @PYTHONPATH={{ pythonpath }} {{ python }} -c 'from pathlib import Path; from harness.domain import load_target_manifests; from harness.emi.catalog import load_catalog, materialize_reviewed_targets; root = Path("{{ root }}"); materialize_reviewed_targets(root=root, catalog=load_catalog(root)); missing = [m.binary for m in load_target_manifests(root).values() if not (root / m.binary).is_file()]; print("binaries: OK" if not missing else "missing binaries: " + ", ".join(missing)); raise SystemExit(bool(missing))'

build:
    @{{ root }}/bin/build all
    @PYTHONPATH={{ pythonpath }} {{ python }} -m harness.commands.compile_commands

check: venv
    @PYTHONDONTWRITEBYTECODE=1 PYTHONPATH={{ pythonpath }} {{ python }} -m pytest -q -p no:cacheprovider tools/python/tests
    @PYTHONPATH={{ pythonpath }} {{ python }} -m ruff check tools/python
    @{{ root }}/bin/symbols check
    @PYTHONPATH={{ pythonpath }} {{ python }} -m harness.commands.validate_sources

format: venv
    @PYTHONPATH={{ pythonpath }} {{ python }} -m ruff format tools/python
    @find src include -type f \( -name '*.c' -o -name '*.h' \) -print0 | xargs -0 -r clang-format -i

index: venv
    @{{ root }}/bin/index

clean:
    @{{ root }}/bin/build clean
