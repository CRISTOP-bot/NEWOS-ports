#!/usr/bin/env python3
"""Validate that every declared submodule matches a checked-out gitlink."""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def git(*args: str, cwd: Path = ROOT) -> str:
    result = subprocess.run(
        ["git", *args], cwd=cwd, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    if result.returncode:
        raise RuntimeError(result.stderr.strip() or f"git {' '.join(args)} failed")
    return result.stdout


def main() -> int:
    if not (ROOT / ".gitmodules").is_file():
        raise RuntimeError(".gitmodules is missing")

    declared: dict[str, tuple[str, str]] = {}
    entries = git("config", "-f", ".gitmodules", "--get-regexp", r"^submodule\..*\.path$").splitlines()
    for entry in entries:
        key, path = entry.split(None, 1)
        section = key.removesuffix(".path")
        url = git("config", "-f", ".gitmodules", "--get", f"{section}.url").strip()
        if path in declared:
            raise RuntimeError(f"duplicate submodule path in .gitmodules: {path}")
        declared[path] = (section, url)

    gitlinks: dict[str, str] = {}
    for record in git("ls-files", "--stage", "-z").split("\0"):
        if not record:
            continue
        metadata, path = record.split("\t", 1)
        mode, sha, stage = metadata.split()
        if mode == "160000":
            if stage != "0":
                raise RuntimeError(f"unmerged submodule gitlink: {path} (stage {stage})")
            gitlinks[path] = sha

    declared_paths = set(declared)
    gitlink_paths = set(gitlinks)
    if declared_paths != gitlink_paths:
        missing_gitlinks = sorted(declared_paths - gitlink_paths)
        missing_declarations = sorted(gitlink_paths - declared_paths)
        raise RuntimeError(
            f".gitmodules/gitlink mismatch; missing gitlinks={missing_gitlinks}, "
            f"missing declarations={missing_declarations}"
        )

    for path, (section, url) in sorted(declared.items()):
        if not url:
            raise RuntimeError(f"submodule {path} has an empty URL")
        configured_url = git("config", "--get", f"{section}.url").strip()
        if configured_url != url:
            raise RuntimeError(
                f"checked-out URL for {path} is {configured_url!r}, expected {url!r}"
            )
        submodule_dir = ROOT / path
        if not submodule_dir.is_dir():
            raise RuntimeError(f"submodule is not initialized: {path}")
        head = git("rev-parse", "--verify", "HEAD", cwd=submodule_dir).strip()
        if head != gitlinks[path]:
            raise RuntimeError(f"{path} HEAD {head} does not match gitlink {gitlinks[path]}")

    # `submodule update --init --recursive` in CI initializes the complete tree.
    # Git prefixes each status line with a space for an exact checkout; '-', '+',
    # or 'U' means an uninitialized, mismatched, or conflicted nested gitlink.
    status = git("submodule", "status", "--recursive")
    bad = [line for line in status.splitlines() if line and line[0] != " "]
    if bad:
        raise RuntimeError("submodule checkout status is not exact:\n" + "\n".join(bad))

    print(f"Validated {len(declared)} declared gitlink(s), with recursive submodule status clean.")
    for path, sha in sorted(gitlinks.items()):
        print(f"  {path}: {sha}")
    print("No top-level build or test target is defined in NEWOS-ports; see CI notes for upstream requirements.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (RuntimeError, ValueError) as error:
        print(f"submodule validation failed: {error}", file=sys.stderr)
        raise SystemExit(1)
