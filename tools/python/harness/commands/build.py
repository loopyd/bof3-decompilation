from ..domain import lookup_target_manifest, normalize_target_id
        manifest = lookup_target_manifest(root, args.selector)
            raise ValueError(
                f"unknown target: {normalize_target_id(args.selector).value}"
            )
            print(f"{manifest.id.value}: no authored sources")
