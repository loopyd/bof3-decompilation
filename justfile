root := justfile_directory()
python := root + "/.venv/bin/python"
pythonpath := root + "/tools/python"

default: help

# Show the supported repository tasks.
help:
    @just --list

# Sync the locked repository-local Python environment.
venv:
    @if [ ! -x "{{python}}" ]; then \
        command -v uv >/dev/null && \
        UV_CACHE_DIR="{{root}}/.uv-cache" uv sync --extra dev --frozen; \
    fi

# Download and stage the required PsyQ 4.7 SDK.
psyq: venv
    @PYTHONPATH={{pythonpath}} {{python}} -m rebof3.commands.toolchain psyq import

# Prepare tools, extract the disc, and refresh binary evidence.
setup: psyq extract
    @PYTHONPATH={{pythonpath}} {{python}} -m rebof3.commands.setup
    @{{root}}/bin/rebof3 normalize
    @{{root}}/bin/rebof3 scan
    @{{root}}/bin/rebof3 doctor

# Extract the disc and unpack every EMI archive into out/extracted.
extract: venv
    @PYTHONPATH={{pythonpath}} {{python}} -m rebof3.commands.disk disk-extract --output {{root}}/out/extracted
    @PYTHONPATH={{pythonpath}} {{python}} -m rebof3.commands.emi emi-unpack --input-dir {{root}}/out/extracted --output-dir {{root}}/out/extracted

# Refresh the EMI catalog.
scan: venv
    @{{root}}/bin/rebof3 scan

# Validate tracked workspace configuration.
doctor: venv
    @{{root}}/bin/rebof3 doctor

# Configure and build through Unix Makefiles using all available cores.
build:
    @cmake --fresh --preset default
    @cmake --build --preset default --parallel

# Run focused repository checks.
check: venv
    @PYTHONDONTWRITEBYTECODE=1 PYTHONPATH={{pythonpath}} {{python}} -m pytest -q -p no:cacheprovider tools/python/tests
    @PYTHONPATH={{pythonpath}} {{python}} -m ruff check tools/python
    @{{root}}/bin/rebof3 doctor

format: format-python format-c

# Format Python tooling.
format-python: venv
    @PYTHONPATH={{pythonpath}} {{python}} -m ruff format tools/python

# Format authored C headers and functions.
format-c:
    @find src bof3/include bof3/src -type f \( -name '*.c' -o -name '*.h' \) -print0 2>/dev/null | xargs -0 -r clang-format -i

# Remove generated build and analysis output.
clean:
    @rm -rf build out
