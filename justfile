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

# Initialize retained dependencies, user-authorized media, and local toolchains.
setup: venv
    @PYTHONPATH={{ pythonpath }} {{ python }} -m harness.commands.setup

doctor: venv
    @PYTHONPATH={{ pythonpath }} {{ python }} -m harness.commands.doctor

# Restore reviewed executable payloads and EMI images without running setup.
binaries: venv
    @PYTHONPATH={{ pythonpath }} {{ python }} -m harness.commands.binaries

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
