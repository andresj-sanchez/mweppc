#!/usr/bin/env python3
"""PS2 b198's C library files that are Metrowerks' own text, rebuilt from the library's sources at build time.

    msl.py make SOURCE_TREE LIBRARY    write config/PS2_3_0_1b198/msl/*.patch from the decompilation's files
    msl.py apply LIBRARY               write the files (src/msl/ps2/..., not kept in git) from the patches

LIBRARY is the MSL sources of CodeWarrior 9.2 for Windows (tools/download_tool.py msl92 unpacks them from its
installer, as the GameCube versions' library comes out of their CodeWarrior discs). The build's own versions of
these files differ from Metrowerks' in a few lines. A patch holds only those lines: every line of the result is
either a range of the library file's lines, named by number, or a line of the decompilation's own:

    = 1-40      the library file's lines 1 to 40
    + text      a line of the decompilation's own

so the patches carry no text of Metrowerks'.
"""

import argparse
import difflib
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PATCHES = ROOT / "config/PS2_3_0_1b198/msl"
# the decompilation's file -> the library's file it is made from
FILES = {
    "alloc.c": "MSL_C/MSL_Common/Src/alloc.c",
    "time.c": "MSL_C/MSL_Common/Src/time.c",
    "scanf_core.c": "MSL_C/MSL_Common/Src/scanf.c",
    "strtoull.c": "MSL_C/MSL_Common/Src/strtoul.c",
    "printf_g.c": "MSL_C/MSL_Common/Src/printf.c",
}
OUTPUT = "src/msl/ps2"


def lines(path):
    return path.read_bytes().decode("latin-1").replace("\r\n", "\n").split("\n")


def make(source, library):
    PATCHES.mkdir(parents=True, exist_ok=True)
    for name, original in FILES.items():
        theirs = lines(library / original)
        ours = lines(source / "src/msl" / name)
        where = {}
        for i, line in enumerate(theirs):
            where.setdefault(line, i)
        out = [f"# {name}: from the MSL's {original}, CodeWarrior 9.2"]
        matcher = difflib.SequenceMatcher(None, theirs, ours, autojunk=False)
        for tag, i1, i2, j1, j2 in matcher.get_opcodes():
            if tag == "equal":
                out.append(f"= {i1 + 1}-{i2}")
                continue
            for line in ours[j1:j2]:
                # (a line the library has somewhere is named by its number, so no text of it is copied here)
                out.append(f"= {where[line] + 1}-{where[line] + 1}" if line in where else f"+ {line}")
        (PATCHES / f"{name}.patch").write_text("\n".join(out) + "\n", encoding="latin-1")
        if apply_one(name, library) != ours:
            sys.exit(f"{name}: the patch does not give the decompilation's file back")


def apply_one(name, library):
    theirs = lines(library / FILES[name])
    out = []
    for line in (PATCHES / f"{name}.patch").read_text(encoding="latin-1").split("\n")[1:-1]:
        if line.startswith("= "):
            first, last = (int(x) for x in line[2:].split("-"))
            out += theirs[first - 1 : last]
        elif line.startswith("+ "):
            out.append(line[2:])
        else:
            sys.exit(f"{name}.patch: cannot read {line!r}")
    return out


def apply(library):
    (ROOT / OUTPUT).mkdir(parents=True, exist_ok=True)
    for name in FILES:
        (ROOT / OUTPUT / name).write_bytes("\n".join(apply_one(name, library)).encode("latin-1"))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    m = sub.add_parser("make")
    m.add_argument("source", type=Path)
    m.add_argument("library", type=Path)
    a = sub.add_parser("apply")
    a.add_argument("library", type=Path)
    args = parser.parse_args()
    if args.command == "make":
        make(args.source, args.library)
    else:
        apply(args.library)


if __name__ == "__main__":
    main()
