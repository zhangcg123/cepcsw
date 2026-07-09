#!/usr/bin/env python3
"""Create or verify a lossless file manifest for a history directory."""

import argparse
import hashlib
from pathlib import Path
import sys


def entries(root: Path):
    if not root.is_dir():
        raise ValueError(f"not a directory: {root}")
    for path in sorted(p for p in root.rglob("*") if p.is_file()):
        digest = hashlib.sha256(path.read_bytes()).hexdigest()
        yield path.relative_to(root).as_posix(), path.stat().st_size, digest


def write_manifest(root: Path, manifest: Path):
    rows = list(entries(root))
    manifest.write_text(
        "".join(f"{digest}\t{size}\t{name}\n" for name, size, digest in rows),
        encoding="utf-8",
    )
    print(f"created {manifest}: {len(rows)} files")


def read_manifest(manifest: Path):
    result = {}
    for line_no, line in enumerate(manifest.read_text(encoding="utf-8").splitlines(), 1):
        try:
            digest, size, name = line.split("\t", 2)
        except ValueError as exc:
            raise ValueError(f"invalid manifest line {line_no}") from exc
        result[name] = (int(size), digest)
    return result


def check_manifest(root: Path, manifest: Path):
    expected = read_manifest(manifest)
    actual = {name: (size, digest) for name, size, digest in entries(root)}
    missing = sorted(set(expected) - set(actual))
    added = sorted(set(actual) - set(expected))
    changed = sorted(name for name in expected.keys() & actual.keys() if expected[name] != actual[name])
    if missing or added or changed:
        for label, values in (("missing", missing), ("added", added), ("changed", changed)):
            for value in values:
                print(f"{label}: {value}", file=sys.stderr)
        return 1
    print(f"verified {root}: {len(actual)} files unchanged")
    return 0


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("mode", choices=("create", "check"))
    parser.add_argument("history_dir", type=Path)
    parser.add_argument("manifest", type=Path)
    args = parser.parse_args()
    try:
        if args.mode == "create":
            write_manifest(args.history_dir, args.manifest)
            return 0
        return check_manifest(args.history_dir, args.manifest)
    except (OSError, ValueError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
