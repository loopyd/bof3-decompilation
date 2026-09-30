"""Harness ownership and failure contracts for independent emulator commands."""

from __future__ import annotations

import hashlib
import json
import os
import sys

import pytest

from harness import cli
from harness.runtime import session
from harness.runtime.cli import build_parser


class Emulator:
    def __init__(self, program):
        self.program = program
        self.arguments = []

    def runtime_identity(self):
        return {
            "revision": "pinned",
            "binary_sha256": "fixture",
            "sdl_sha256": "fixture-sdl",
        }

    def verify(self):
        return "fixture ready"

    @property
    def environment(self):
        return os.environ.copy()

    def invocation(self, arguments):
        self.arguments = arguments
        return [sys.executable, "-c", self.program]


@pytest.fixture
def mission(tmp_path):
    bios = tmp_path / "renamed-bios.bin"
    bios.write_bytes(bytes(524288))
    exe = tmp_path / "game with spaces.exe"
    header = bytearray(2048)
    header[:8] = b"PS-X EXE"
    header[16:20] = (0x80010000).to_bytes(4, "little")
    exe.write_bytes(header)
    script = tmp_path / "commands.lua"
    script.write_text("-- trusted fixture\n")
    return dict(bios=bios, executable=exe, script=script, output=tmp_path / "capture")


def completed_program(captures=None):
    marker = json.dumps(
        {"schema": "psx.runtime-completion/v1", "captures": captures or []}
    )
    return f"from pathlib import Path; Path('completion.json').write_text({marker!r})"


def test_runtime_dispatches_to_one_owner():
    assert cli.DOMAINS["runtime"].passthrough.module == "harness.runtime.cli"


@pytest.fixture
def continuation(mission, tmp_path):
    mission.pop("executable")
    program = (
        "from pathlib import Path; Path('state.pbuf').write_bytes(b'state'); "
        + completed_program([{"path": "state.pbuf", "bytes": 5}])
    )
    session.run_session(Emulator(program), **mission)
    return mission | {
        "origin": mission["output"] / "receipt.json",
        "files": [f"state={mission['output'] / 'state.pbuf'}"],
        "output": tmp_path / "continuation",
    }


def test_origin_binds_capture_and_exposes_verified_provenance(continuation):
    emulator = Emulator(
        "import os; assert os.environ['PSX_RUNTIME_ORIGIN_VALIDATED'] == '1'; "
        + completed_program()
    )
    report = session.run_session(emulator, **continuation)
    assert report["origin"]["state_sha256"] == hashlib.sha256(b"state").hexdigest()
    assert report["origin"]["captures"] == ["state.pbuf"]
    assert (
        report["inputs"]["origin"]["sha256"]
        == hashlib.sha256(continuation["origin"].read_bytes()).hexdigest()
    )


@pytest.mark.parametrize(
    "mutation,diagnostic",
    [
        ("state", "state bytes"),
        ("emulator", "emulator identity"),
        ("bios", "BIOS/media"),
        ("disc", "BIOS/media"),
        ("failed", "successful runtime"),
        ("missing", "requires --input state"),
        ("malformed", "malformed origin"),
        ("duplicate", "duplicate origin"),
    ],
)
def test_origin_conflicts_reject_before_launch(continuation, mutation, diagnostic):
    path = continuation["origin"]
    original = json.loads(path.read_text())
    if mutation == "state":
        (path.parent / "state.pbuf").write_bytes(b"other")
    elif mutation == "emulator":
        original["emulator"]["binary_sha256"] = "changed"
    elif mutation == "bios":
        original["inputs"]["bios"]["sha256"] = "changed"
    elif mutation == "disc":
        original["inputs"]["disc:cue"] = {"sha256": "other", "bytes": 1}
    elif mutation == "failed":
        original["status"] = "failed"
    elif mutation == "missing":
        continuation["files"] = []
    elif mutation == "malformed":
        original["inputs"] = []
    if mutation == "duplicate":
        path.write_text('{"schema":1,"schema":2}')
    else:
        path.write_text(json.dumps(original))
    with pytest.raises(ValueError, match=diagnostic):
        session.run_session(
            Emulator("raise AssertionError('must not launch')"), **continuation
        )
    assert not continuation["output"].exists()


def test_origin_drift_rejects_after_execution(continuation):
    program = (
        "from pathlib import Path; "
        f"Path({str(continuation['origin'])!r}).write_text('changed'); "
        + completed_program()
    )
    with pytest.raises(ValueError, match="inputs changed"):
        session.run_session(Emulator(program), **continuation)
    report = json.loads((continuation["output"] / "receipt.json").read_text())
    assert report["status"] == "failed"


def test_disc_mount_pins_tracks_and_isolates_sidecars(mission, tmp_path):
    track = tmp_path / "track with spaces.bin"
    track.write_bytes(bytes(2352))
    cue = tmp_path / "disc.cue"
    cue.write_text(
        'FILE "track with spaces.bin" BINARY\n TRACK 01 MODE2/2352\n INDEX 01 00:00:00\n'
    )
    (tmp_path / "disc.ppf").write_bytes(b"unrelated sidecar")
    emulator = Emulator(
        "from pathlib import Path; import os, hashlib; "
        "assert set(p.name for p in Path('disc').iterdir()) == {'media.cue', 'track01.bin'}; "
        "assert Path('disc/track01.bin').read_bytes() == bytes(2352); "
        "assert 'track01.bin' in Path('disc/media.cue').read_text(); "
        "assert os.environ['PSX_RUNTIME_DIRECTORY'] == str(Path.cwd()); "
        "assert os.environ['PSX_RUNTIME_MEDIA'] == "
        "'disc:track01.bin\\t2352\\t' + hashlib.sha256(bytes(2352)).hexdigest() + '\\n'; "
        + completed_program()
    )
    report = session.run_session(emulator, **mission, disc=cue)
    assert (
        report["inputs"]["disc:track01.bin"]["sha256"]
        == hashlib.sha256(track.read_bytes()).hexdigest()
    )
    assert emulator.arguments[emulator.arguments.index("-iso") + 1] == str(
        mission["output"] / "disc/media.cue"
    )
    assert (
        report["disc"]["sha256"]
        == hashlib.sha256(
            (mission["output"] / "disc/media.cue").read_bytes()
        ).hexdigest()
    )


def test_media_bindings_replace_inherited_values_without_disc(mission, monkeypatch):
    monkeypatch.setenv("PSX_RUNTIME_MEDIA", "untrusted inherited identities")
    monkeypatch.setenv("PSX_RUNTIME_DIRECTORY", "/wrong")
    emulator = Emulator(
        "import os; from pathlib import Path; "
        "assert os.environ['PSX_RUNTIME_MEDIA'] == ''; "
        "assert os.environ['PSX_RUNTIME_DIRECTORY'] == str(Path.cwd()); "
        + completed_program()
    )
    assert session.run_session(emulator, **mission)["status"] == "passed"


def test_media_bindings_preserve_staged_aliases_and_order(mission, tmp_path):
    track = tmp_path / "shared.bin"
    track.write_bytes(bytes(2352))
    cue = tmp_path / "shared.cue"
    cue.write_text(
        'FILE "shared.bin" BINARY\n TRACK 01 MODE2/2352\n INDEX 01 00:00:00\n'
        'FILE "shared.bin" BINARY\n TRACK 02 AUDIO\n INDEX 01 00:00:00\n'
    )
    emulator = Emulator(
        "import os, hashlib; "
        "digest = hashlib.sha256(bytes(2352)).hexdigest(); "
        "assert os.environ['PSX_RUNTIME_MEDIA'] == ''.join("
        "f'disc:track{n:02d}.bin\\t2352\\t{digest}\\n' for n in (1, 2)); "
        + completed_program()
    )
    report = session.run_session(emulator, **mission, disc=cue)
    assert report["status"] == "passed"
    assert report["inputs"]["disc:track01.bin"] == report["inputs"]["disc:track02.bin"]


@pytest.mark.parametrize(
    "line", ['FILE "track.bin" WAVE', 'CDTEXTFILE "other.bin"', "REM no tracks"]
)
def test_unsupported_disc_descriptors_fail_before_launch(mission, tmp_path, line):
    cue = tmp_path / "disc.cue"
    cue.write_text(line + "\n")
    with pytest.raises(ValueError, match="disc"):
        session.run_session(
            Emulator("raise AssertionError('must not run')"), **mission, disc=cue
        )
    assert not mission["output"].exists()


@pytest.mark.parametrize(
    "mutation",
    [
        "Path('disc/media.cue').write_text('changed')",
        "Path('disc/extra.sub').write_bytes(b'changed')",
        "Path('disc/track01.bin').write_bytes(b'changed')",
    ],
)
def test_media_drift_cannot_publish_passing_capture(mission, tmp_path, mutation):
    track = tmp_path / "track.bin"
    track.write_bytes(bytes(2352))
    cue = tmp_path / "disc.cue"
    cue.write_text(
        'FILE "track.bin" BINARY\n TRACK 01 MODE2/2352\n INDEX 01 00:00:00\n'
    )
    emulator = Emulator(
        "from pathlib import Path; " + mutation + "; " + completed_program()
    )
    with pytest.raises(ValueError, match="changed"):
        session.run_session(emulator, **mission, disc=cue)
    assert (
        json.loads((mission["output"] / "receipt.json").read_text())["status"]
        == "failed"
    )


def test_disc_track_bound_rejects_before_hashing_or_launch(mission, tmp_path):
    track = tmp_path / "large.bin"
    with track.open("wb") as stream:
        stream.truncate(1024**3 + 1)
    cue = tmp_path / "disc.cue"
    cue.write_text('FILE "large.bin" BINARY\n')
    with pytest.raises(ValueError, match="GiB"):
        session.run_session(
            Emulator("raise AssertionError('must not run')"), **mission, disc=cue
        )
    assert not mission["output"].exists()


def test_declared_assets_are_staged_hashed_and_passed_literally(mission, tmp_path):
    module = tmp_path / "support with spaces.lua"
    module.write_text("return {}\n")
    state = tmp_path / "state with spaces.pbuf"
    state.write_bytes(b"snapshot\0bytes")
    emulator = Emulator(
        "import os; from pathlib import Path; "
        "assert Path(os.environ['PSX_RUNTIME_INPUT_STATE']).read_bytes() == b'snapshot\\0bytes'; "
        "assert Path('lua/support.lua').read_text() == 'return {}\\n'; "
        "assert os.environ['PSX_RUNTIME_ARGUMENT_NAMES'] == 'ACTION'; "
        "assert os.environ['LUA_PATH'].endswith('/lua/?.lua'); "
        "assert os.environ['LUA_CPATH'] == ''; " + completed_program()
    )
    receipt = session.run_session(
        emulator,
        **mission,
        modules=[f"support={module}"],
        files=[f"state={state}"],
        arguments=["action=query"],
    )
    assert receipt["assets"] == {
        "module:support": "lua/support.lua",
        "file:STATE": "inputs/STATE",
    }
    assert (
        receipt["inputs"]["file:STATE"]["sha256"]
        == hashlib.sha256(state.read_bytes()).hexdigest()
    )


@pytest.mark.parametrize("kind", ["module", "file"])
def test_asset_edits_in_staged_copy_invalidate_run(mission, kind):
    values = (
        {"modules": [f"support={mission['script']}"]}
        if kind == "module"
        else {"files": [f"state={mission['script']}"]}
    )
    path = "lua/support.lua" if kind == "module" else "inputs/STATE"
    emulator = Emulator(
        f"from pathlib import Path; Path({path!r}).write_text('changed'); "
        + completed_program()
    )
    with pytest.raises(ValueError, match="inputs changed"):
        session.run_session(emulator, **mission, **values)
    assert (
        json.loads((mission["output"] / "receipt.json").read_text())["status"]
        == "failed"
    )


@pytest.mark.parametrize(
    "values",
    [
        {"modules": ["../escape=PATH"]},
        {"files": ["state=PATH", "STATE=PATH"]},
        {"modules": ["support=PATH", "support=PATH"]},
    ],
)
def test_invalid_asset_names_fail_before_output_creation(mission, values):
    values = {
        key: [item.replace("PATH", str(mission["script"])) for item in items]
        for key, items in values.items()
    }
    with pytest.raises(ValueError):
        session.run_session(
            Emulator("raise AssertionError('must not run')"), **mission, **values
        )
    assert not mission["output"].exists()


def test_staging_failure_retains_failed_receipt(mission, monkeypatch):
    def fail(*args):
        raise ValueError("staging changed input")

    monkeypatch.setattr(session, "stage_assets", fail)
    with pytest.raises(ValueError, match="staging changed"):
        session.run_session(Emulator("raise AssertionError('must not run')"), **mission)
    receipt = json.loads((mission["output"] / "receipt.json").read_text())
    assert receipt["status"] == "failed"
    assert "staging changed" in receipt["error"]


@pytest.mark.parametrize(
    "kind,limit", [("modules", 1024 * 1024), ("files", 64 * 1024 * 1024)]
)
def test_oversized_assets_fail_before_launch(mission, tmp_path, kind, limit):
    asset = tmp_path / "oversized.bin"
    with asset.open("wb") as stream:
        stream.truncate(limit + 1)
    with pytest.raises(ValueError, match="bounds"):
        session.run_session(
            Emulator("raise AssertionError('must not run')"),
            **mission,
            **{kind: [f"asset={asset}"]},
        )
    assert not mission["output"].exists()


def test_bios_only_mission_transports_literal_arguments_and_clears_inherited_values(
    mission, monkeypatch
):
    mission.pop("executable")
    mission["bios"].write_bytes(bytes([0x5A]) * 524288)
    monkeypatch.setenv("PSX_RUNTIME_ENTRY", "stale")
    monkeypatch.setenv("PSX_RUNTIME_ARG_INHERITED", "stale")
    value = 'spaces; $(false) "quoted"'
    program = (
        "import os; "
        f"assert os.environ['PSX_RUNTIME_ARG_LABEL'] == {value!r}; "
        "assert 'PSX_RUNTIME_ENTRY' not in os.environ; "
        "assert 'PSX_RUNTIME_ARG_INHERITED' not in os.environ; " + completed_program()
    )
    emulator = Emulator(program)
    receipt = session.run_session(emulator, **mission, arguments=[f"label={value}"])
    assert receipt["status"] == "passed"
    assert receipt["schema"] == "psx.runtime-session/v1"
    assert receipt["arguments"] == {"LABEL": value}
    assert "executable" not in receipt["inputs"]
    assert "-exe" not in emulator.arguments
    assert (
        receipt["inputs"]["bios"]["sha256"]
        == hashlib.sha256(mission["bios"].read_bytes()).hexdigest()
    )


@pytest.mark.parametrize(
    "arguments",
    [
        ["missing-value"],
        ["target=1", "TARGET=2"],
        ["bad-name=3"],
        ["name=nul\x00byte"],
        ["name=" + "x" * 4097],
    ],
)
def test_invalid_mission_arguments_fail_before_launch(mission, arguments):
    with pytest.raises(ValueError, match="arguments"):
        session.run_session(
            Emulator("raise AssertionError('must not run')"),
            **mission,
            arguments=arguments,
        )
    assert not mission["output"].exists()


def test_cli_accepts_mission_arguments_without_an_executable():
    args = build_parser().parse_args(
        [
            "run",
            "--bios",
            "rom.bin",
            "--script",
            "mission.lua",
            "--output",
            "capture",
            "--argument",
            "target=0xbfc00000",
            "--argument",
            "label=reset",
        ]
    )
    assert args.executable is None
    assert args.argument == ["target=0xbfc00000", "label=reset"]


def test_success_pins_inputs_isolates_settings_and_validates_completion(mission):
    emulator = Emulator(completed_program() + "; print('command completed')")
    receipt = session.run_session(emulator, **mission)
    assert receipt["status"] == "passed"
    assert receipt["completion"]["captures"] == []
    assert receipt["inputs"]["bios"]["path"].endswith("renamed-bios.bin")
    assert emulator.arguments[emulator.arguments.index("-portable") + 1] == str(
        mission["output"]
    )
    assert {
        "-interpreter",
        "-no-gdb",
        "-no-webserver",
        "-no-pcdrv",
        "-no-fastboot",
    } <= set(emulator.arguments)
    assert (mission["output"] / "stdout.log").read_text() == "command completed\n"
    assert json.loads((mission["output"] / "receipt.json").read_text()) == receipt


@pytest.mark.parametrize(
    "program,failure",
    [
        ("import time; time.sleep(5)", "timeout"),
        ("print('x' * 4096)", "output limit"),
        ("raise SystemExit(7)", None),
    ],
)
def test_process_failure_cannot_publish_success(mission, program, failure):
    receipt = session.run_session(
        Emulator(program), **mission, timeout=0.3, output_limit=1024
    )
    assert receipt["status"] == "failed"
    assert receipt["process"]["failure"] == failure
    assert "completion" not in receipt


def test_success_without_marker_is_rejected_and_retained(mission):
    with pytest.raises(ValueError, match="completion.json"):
        session.run_session(Emulator("pass"), **mission)
    receipt = json.loads((mission["output"] / "receipt.json").read_text())
    assert receipt["status"] == "failed"
    assert receipt["process"]["exit_code"] == 0


@pytest.mark.parametrize(
    "captures",
    [
        [{"path": "../escape", "bytes": 0}],
        [{"path": "receipt.json", "bytes": 0}],
        [{"path": "missing.bin", "bytes": 3}],
        [{"path": "a", "bytes": True}],
        [{"path": "a", "bytes": 64 * 1024 * 1024 + 1}],
    ],
)
def test_invalid_capture_descriptors_reject(mission, captures):
    with pytest.raises(ValueError):
        session.run_session(Emulator(completed_program(captures)), **mission)
    assert (
        json.loads((mission["output"] / "receipt.json").read_text())["status"]
        == "failed"
    )


def test_capture_hashes_are_checked_after_process_exit(mission):
    program = (
        "from pathlib import Path; Path('data.bin').write_bytes(b'abc'); "
        + completed_program([{"path": "data.bin", "bytes": 3}])
    )
    receipt = session.run_session(Emulator(program), **mission)
    assert (
        receipt["completion"]["captures"][0]["sha256"]
        == hashlib.sha256(b"abc").hexdigest()
    )


def test_changed_input_is_not_accepted(mission):
    program = (
        completed_program()
        + f"; Path({str(mission['script'])!r}).write_text('changed')"
    )
    with pytest.raises(ValueError, match="inputs changed"):
        session.run_session(Emulator(program), **mission)


def test_existing_output_and_invalid_bios_are_not_overwritten(mission):
    mission["bios"].write_bytes(b"bad")
    with pytest.raises(ValueError, match="BIOS"):
        session.run_session(Emulator("raise AssertionError('must not run')"), **mission)
    assert not mission["output"].exists()
    mission["bios"].write_bytes(bytes(524288))
    mission["output"].mkdir()
    with pytest.raises(FileExistsError):
        session.run_session(Emulator("pass"), **mission)
