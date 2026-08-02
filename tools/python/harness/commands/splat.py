from ..domain import lookup_target_manifest
from ..toolchain.splat import SplatToolchain
    manifest = lookup_target_manifest(root, args.target)
    toolchain = SplatToolchain(root)
    if not toolchain.executable.is_file():
        raise FileNotFoundError(f"missing Splat executable: {toolchain.executable}; run just setup")
    result = toolchain.execute(
        ["split", "--make-full-disasm-for-code", str(root / manifest.splat)],
            print(f"{manifest.id.value}: splat OK")
