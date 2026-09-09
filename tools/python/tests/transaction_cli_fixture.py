"""Real owner CLI handlers over synthetic native-gate fixtures, never stub transport."""

import contextlib
import io
import json
from unittest.mock import patch

from harness.macros import cli as macro_audit
from harness.types import cli as type_audit


class OwnerCLI:
    """Adapt existing transaction fixtures to argparse and retained CLI outputs."""

    def __init__(self, root, owner, transactions, review):
        self.root = root
        self.cli = type_audit if owner == "type" else macro_audit
        self.transactions = transactions
        self.review = review
        self.count = 0
        self.commands = []

    def __getattr__(self, name):
        return getattr(self.transactions, name)

    def invoke(self, command, values, flags=None, runner=None, output=None):
        self.count += 1
        directory = self.root / f"out/reviews/evidence/cli-{self.count}"
        directory.mkdir(parents=True)
        argv = ["--root", str(self.root), command]
        for number, value in enumerate(values):
            path = directory / f"input-{number}.json"
            path.write_text(json.dumps(value))
            argv.append(str(path))
        produces = command in {
            "prepare",
            "run",
            "review",
            "revalidate",
            "review-revalidation",
        }
        if produces:
            output = output or str((directory / "output.json").relative_to(self.root))
            argv.append(output)
        for name, value in (flags or {}).items():
            if value is None:
                continue
            argv.append("--" + name.replace("_", "-"))
            if name in {"parent_attestation", "intervening"}:
                path = directory / f"{name}.json"
                path.write_text(json.dumps(value))
                argv.append(str(path))
            else:
                argv.extend(value if isinstance(value, list) else [value])
        args = self.cli.build_parser().parse_args(argv)
        stream = io.StringIO()
        with contextlib.ExitStack() as stack:
            if runner is not None:
                module, name = (
                    (self.cli, "run_transaction")
                    if command == "run"
                    else (self.review, "revalidate_application")
                )
                original = getattr(module, name)
                stack.enter_context(
                    patch.object(
                        module, name, lambda *a, **kw: original(*a, runner=runner, **kw)
                    )
                )
            stack.enter_context(contextlib.redirect_stdout(stream))
            self.commands.append(command)
            assert args.handler(args) == 0
        return (
            json.loads((self.root / output).read_text())
            if produces
            else json.loads(stream.getvalue())
        )

    def prepare_transaction(self, root, request):
        assert root == self.root
        return self.invoke("prepare", [request])

    def run_transaction(
        self, root, manifest, changes, *, runner=None, output=None, **flags
    ):
        assert root == self.root
        return self.invoke("run", [manifest, changes], flags, runner, output)

    def review_application(self, root, application, parent, pin):
        assert root == self.root
        return self.invoke(
            "review",
            [application],
            {"parent_attestation": parent, "expected_application_digest": pin},
        )

    def verify_reviewed_application(self, root, envelope, pin):
        assert root == self.root
        return self.invoke(
            "final-verify", [envelope], {"expected_envelope_digest": pin}
        )

    def revalidate_application(
        self, root, envelope, pin, *, runner=None, output=None, **flags
    ):
        assert root == self.root
        return self.invoke(
            "revalidate",
            [envelope],
            {"expected_envelope_digest": pin, **flags},
            runner,
            output,
        )

    def review_revalidation(self, root, record, parent, pin):
        assert root == self.root
        self.invoke(
            "verify-revalidation", [record], {"expected_revalidation_digest": pin}
        )
        return self.invoke(
            "review-revalidation",
            [record],
            {"parent_attestation": parent, "expected_revalidation_digest": pin},
        )

    def verify_reviewed_revalidation(self, root, envelope, pin):
        assert root == self.root
        return self.invoke(
            "final-verify-revalidation", [envelope], {"expected_envelope_digest": pin}
        )

    def verify_revalidation(self, root, record, pin):
        assert root == self.root
        return self.invoke(
            "verify-revalidation", [record], {"expected_revalidation_digest": pin}
        )
