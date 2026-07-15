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

# Setup is orchestration; each dependency has its own small entry point.
setup: venv setup-toolchain setup-psyq setup-rust setup-wibo extract unpack discover doctor
    @printf '%s\n' 'workspace setup complete'

setup-toolchain: venv
    @{{ root }}/bin/setup-toolchain

setup-psyq: venv
    @{{ root }}/bin/setup-psyq

setup-rust:
    @{{ root }}/bin/setup-rust

setup-wibo:
    @{{ root }}/bin/setup-wibo --download

# Extract the disc into out/extracted. Builds the native extractor first.
extract: venv setup-rust
    @PYTHONPATH={{ pythonpath }} {{ python }} -m harness.commands.setup --task submodules
    @PYTHONPATH={{ pythonpath }} {{ python }} -m harness.commands.setup --task extract

# Unpack EMI archives from the extracted disc tree into out/extracted.
unpack: venv setup-rust
    @PYTHONPATH={{ pythonpath }} {{ python }} -m harness.commands.setup --task unpack

# Repack unpacked EMI manifests into the extracted disc tree.
pack: venv setup-rust
    @{{ root }}/bin/emi-pack

# Split one tracked Splat layout into generated assembly/data evidence.
split CONFIG: venv
    @{{ root }}/bin/splat split {{CONFIG}}

# Refresh the EMI catalog.
discover: venv
    @{{ root }}/bin/harness discover

# Validate tracked workspace configuration.
doctor *args: venv
    @{{ root }}/bin/harness doctor {{args}}

# Compile every authored C and assembly source into build/src/.
build:
    @make --no-print-directory all

# Compile one source without invoking the full build.
build-one FUNC:
    @make --no-print-directory build-one FUNC={{FUNC}}

# Run the focused function comparison.
diff FUNC:
    @make --no-print-directory diff FUNC={{FUNC}}

# Prepare and run the focused decomp-permuter workflow.
permute FUNC:
    @make --no-print-directory permute FUNC={{FUNC}}

# Rebuild one target using the transitional function-text-only workflow.
rebuild TARGET: venv
    @{{ root }}/bin/rebuild {{TARGET}}

# Run matching checks, Python checks, formatting checks, and workspace validation.
check: venv check-format-c
    @make --no-print-directory check
    @PYTHONDONTWRITEBYTECODE=1 PYTHONPATH={{ pythonpath }} {{ python }} -m pytest -q -p no:cacheprovider tools/python/tests
    @PYTHONPATH={{ pythonpath }} {{ python }} -m ruff check tools/python
    @{{ root }}/bin/harness doctor --strict

# Verify one target, or all active targets when TARGET is omitted.
verify *args: venv
    @{{ root }}/bin/verify {{args}}

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

# Generate IDE compile commands by observing the Make build.
compile-commands:
    @if command -v compiledb >/dev/null 2>&1; then \
        compiledb make --no-print-directory all; \
    elif command -v bear >/dev/null 2>&1; then \
        bear -- make --no-print-directory all; \
    else \
        printf '%s\n' 'compiledb or bear is required for compile_commands.json' >&2; \
        exit 1; \
    fi

# Harness shortcuts retained for workflows that are not project builds.
targets: venv
    @{{ root }}/bin/harness targets

assets *args: venv
    @{{ root }}/bin/harness assets {{args}}

promote *args: venv
    @{{ root }}/bin/harness promote {{args}}

reverse *args: venv
    @{{ root }}/bin/harness reverse {{args}}

# Remove only compiler output; preserve generated evidence under out/.
clean:
    @make --no-print-directory clean
