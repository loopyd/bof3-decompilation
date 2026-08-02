    report = build_report(root, args.targets, use_cache=not args.no_cache)
    parser.add_argument(
        "--no-cache",
        action="store_true",
        help="recompute every lift instead of reusing disposable audit summaries",
    )
