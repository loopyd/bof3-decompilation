root := justfile_directory()
python_env := env_var_or_default("VIRTUAL_ENV", root + "/.venv")
python := python_env + "/bin/python"
pythonpath := root + "/tools/python"

default: help

# Show the supported repository tasks.
help:
    @just --list

# Use an activated environment when available; otherwise sync the local one on demand.
[private]
venv:
    @if [ -n "${VIRTUAL_ENV:-}" ]; then \
        test -x "{{ python }}" || { \
            printf 'active Python environment is missing: %s\n' "{{ python }}" >&2; \
            exit 1; \
        }; \
    elif [ ! -x "{{ python }}" ]; then \
        command -v uv >/dev/null || { \
            printf 'uv is required to create the repository Python environment\n' >&2; \
            exit 1; \
        }; \
        UV_CACHE_DIR="{{ root }}/.uv-cache" uv sync --extra dev --frozen; \
    fi

# Download and stage the required PsyQ 4.7 SDK.
psyq: venv
    @PYTHONPATH={{ pythonpath }} {{ python }} -m harness.commands.toolchain psyq import

# Prepare tools, extract the disc, and refresh binary evidence.
setup: venv
    @PYTHONPATH={{ pythonpath }} {{ python }} -m harness.commands.setup
    @{{ root }}/bin/harness normalize
    @{{ root }}/bin/harness scan
    @{{ root }}/bin/harness index build
    @{{ root }}/bin/harness doctor

# Extract the disc and unpack every EMI archive into out/extracted.
extract: venv
    @PYTHONPATH={{ pythonpath }} {{ python }} -m harness.commands.disk disk-extract --output {{ root }}/out/extracted
    @PYTHONPATH={{ pythonpath }} {{ python }} -m harness.commands.emi emi-unpack --input-dir {{ root }}/out/extracted --output-dir {{ root }}/out/extracted

# Refresh the EMI catalog.
scan: venv
    @{{ root }}/bin/harness scan
    @{{ root }}/bin/harness index build

# Validate tracked workspace configuration.
doctor: venv
    @{{ root }}/bin/harness doctor

# Configure and build every registered executable and overlay artifact.
build:
    @cmake --fresh --preset default
    @cmake --build --preset default --parallel --target artifacts

# Run focused repository checks.
check: venv check-format-c
    @PYTHONDONTWRITEBYTECODE=1 PYTHONPATH={{ pythonpath }} {{ python }} -m pytest -q -p no:cacheprovider tools/python/tests
    @PYTHONPATH={{ pythonpath }} {{ python }} -m ruff check tools/python
    @{{ root }}/bin/harness doctor

format: format-python format-c

# Format Python tooling.
format-python: venv
    @PYTHONPATH={{ pythonpath }} {{ python }} -m ruff format tools/python

# Format authored C headers and functions.
format-c:
    @find src include -type f \( -name '*.c' -o -name '*.h' \) -print0 2>/dev/null | xargs -0 -r -P 0 -n 32 clang-format -i

# Check C formatting without rewriting source.
check-format-c:
    @find src include -type f \( -name '*.c' -o -name '*.h' \) -print0 2>/dev/null | xargs -0 -r -P 0 -n 32 clang-format --dry-run --Werror

# Remove generated build and analysis output.
clean:
    @rm -rf build out
