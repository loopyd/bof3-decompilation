"""Validate that the local reverse-engineering setup is ready to use."""
import subprocess
import tomllib
from collections.abc import Callable
from ..compiler_config import load_object_compilers
from ..domain import load_target_manifests
from ..io import repo_layout
from ..toolchain import managed_toolchains
from ..toolchain.disc import DiscToolchain
from ..toolchain.psyq import PsyqToolchain
from ._common import run_main
from .setup import REQUIRED_TOOLS, _psyq_47_members


Task = Callable[[Path], str]


@dataclass(frozen=True)
class DoctorTask:
    label: str
    run: Task


TASKS: list[DoctorTask] = []


def doctor_task(label: str) -> Callable[[Task], Task]:
    def register(run: Task) -> Task:
        TASKS.append(DoctorTask(label, run))
        return run

    return register


def _require(root: Path, paths: tuple[Path, ...]) -> str:
    missing = [str(path.relative_to(root)) for path in paths if not path.exists()]
    if missing:
        raise FileNotFoundError(", ".join(missing))
    return f"{len(paths)} present"


@doctor_task("toolchain")
def _toolchain(root: Path) -> str:
    from ..toolchain.gcc_variants import lookup_variant

    layout = repo_layout(root)
    labels = [toolchain.verify() for toolchain in managed_toolchains(root, layout)]

    # Inspect compiler variants only when BOF3_OBJCOMPILER_ selections exist.
    selections = load_object_compilers(root)
    if selections:
        verified_ids = set()
        for key, cid in selections.items():
            if cid in verified_ids:
                continue
            verified_ids.add(cid)
            variant = lookup_variant(layout, cid)
            variant.verify(layout)
            labels.append(f"compiler={variant.label} ({variant.id})")

    return ", ".join(labels)


@doctor_task("PsyQ 4.7")
def _psyq(root: Path) -> str:
    layout = repo_layout(root)
    PsyqToolchain(layout).verify()
    members = _psyq_47_members(root)
    _require(root, tuple(members))
    return f"headers, libraries, {len(members)} reviewed members"


@doctor_task("disc media")
def _disc(root: Path) -> str:
    return DiscToolchain(root).verify()


@doctor_task("target images")
def _target_images(root: Path) -> str:
    manifests = load_target_manifests(root)
    missing = [
        str(manifest.binary)
        for manifest in manifests.values()
        if not (root / manifest.binary).is_file()
    ]
    if missing:
        raise FileNotFoundError(", ".join(missing))
    return f"{len(manifests)} images"


@doctor_task("tool wrappers")
def _tools(root: Path) -> str:
    layout = repo_layout(root)
    commands = (
        (root / "bin" / "cc", "-x", "c", "-E", "-"),
        *((root / tool, "--version") for tool in REQUIRED_TOOLS),
        (root / "bin" / "rizin", "-V"),
        (root / "bin" / "maspsx", "--help"),
        (root / "bin" / "spimdisasm", "--version"),
        (layout.harness_disk_bin, "--help"),
        (layout.emi_ex_bin, "--help"),
    )
    for command in commands:
        result = subprocess.run(
            [str(part) for part in command],
            cwd=root,
            stdin=None if command[0] == root / "bin" / "cc" else subprocess.DEVNULL,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=False,
        )
        if result.returncode:
            raise RuntimeError(f"{' '.join(map(str, command))} exited {result.returncode}")
    return f"{len(commands)} commands"


def _render(status: str, label: str, detail: str) -> None:
    print(f"[{status}] {label:<{max(len(task.label) for task in TASKS)}}  {detail}")
    failed = 0
    for task in TASKS:
        try:
            _render("PASS", task.label, task.run(root))
        except (FileNotFoundError, RuntimeError, ValueError, tomllib.TOMLDecodeError) as exc:
            failed += 1
            _render("FAIL", task.label, str(exc).replace("\n", "; "))
    print(f"doctor: {len(TASKS) - failed}/{len(TASKS)} checks passed")
    return 2 if failed else 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="doctor")
