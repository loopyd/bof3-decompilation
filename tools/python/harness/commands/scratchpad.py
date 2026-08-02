"""Create a public decomp.me scratch from one target-qualified lift."""

from ..domain import FUNCTION_ID_HELP, parse_function_id
from ..io import repo_layout
from ..toolchain.decompme import DecompMeScratchpadToolchain
from ._common import run_main


def run_share(args: argparse.Namespace) -> int:
    toolchain = DecompMeScratchpadToolchain(repo_layout())
    payload = toolchain.payload(
        parse_function_id(args.function), compiler=args.compiler
    )
    url = toolchain.publish(payload)
    print(url)
    return 0


def run_preview(args: argparse.Namespace) -> int:
    toolchain = DecompMeScratchpadToolchain(repo_layout())
    payload = toolchain.payload(
        parse_function_id(args.function), compiler=args.compiler
    )
    print(json.dumps(payload.as_api_data(), indent=2, sort_keys=True))
    parser = argparse.ArgumentParser(prog="scratchpad")
    subcommands = parser.add_subparsers(dest="command", required=True)
    for name, handler, help_text in (
        ("share", run_share, "create a public decomp.me scratch"),
        ("preview", run_preview, "print the decomp.me payload without publishing"),
    ):
        command = subcommands.add_parser(name, help=help_text)
        command.add_argument("function", help=FUNCTION_ID_HELP)
        command.add_argument(
            "--compiler",
            default="gcc-2.7.2-psx",
            help="local canonical/catalog GCC ID mapped to decomp.me",
        )
    return run_main(build_parser, sys.argv[1:] if argv is None else argv)


if __name__ == "__main__":
    raise SystemExit(main())
