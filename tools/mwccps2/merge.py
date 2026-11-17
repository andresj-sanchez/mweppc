#!/usr/bin/env python3
"""Bring a PS2 3.0.1 build 198 source tree into this one, and check that it is in.

    merge.py SOURCE_TREE [--plan] [--threshold PERCENT]    merge, writing the files of this tree
    merge.py --check SOURCE_TREE                           fail unless every file resolves as it should

SOURCE_TREE is a checkout of the PS2 decompilation (src/ and include/ as it lays them out). Every file of it lands
here in one of four ways:

- this tree has no file of that name: it is added where this tree keeps that kind of file (src/compiler/frontend
  for src/frontend, include/libc for the C library's own headers, ...);
- this tree has the file and shares most of it: the two are merged a top-level block at a time (a function, a
  declaration, a group of preprocessor lines). Blocks both have are written once; the rest is chosen by
  `#if MWCCPS2_B198` (set on the PS2 b198 build's command line only), with this tree's text in the `#else`. A header's
  include guard is not a block: what it guards is compared a declaration at a time. Other builds' own conditionals
  here (`#if MWCCPS2_B52`) are read as false, as the PS2 b198 build reads them, and stay where they are;
- this tree has a file of that name with different code (the PowerPC back end's InstrSelection.c and the MIPS one's):
  the PS2 file goes under src/ps2/ or include/ps2/, which the PS2 build searches first;
- five C library files whose text is Metrowerks' own are not kept here: the build makes them from the library's
  sources and a patch of the decompilation's changes (tools/mwccps2/msl.py).

--check resolves every merged file both ways: for PS2 b198 it must give the source tree's file and for everything
else the file as it was before the merge (both token for token: clang-format may move white space). So the
GameCube, PSP and PS2 b210 builds read exactly the code they read before.
"""

import argparse
import difflib
import json
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
# set on the PS2 b198 build's command line only (tools/mwccps2/matching.py), as MWCCPSP is on the PSP build's
MARK = "MWCCPS2_B198"
OPEN_PS2 = f"#if {MARK}"
OPEN_OTHER = f"#if !{MARK}"
# other builds' own conditionals in this tree, false when PS2 b198 is built
OFF = ("MWCCPS2_B52",)
NATIVE = ("src/psp", "src/native", "src/host", "include/psp", "include/host")
DIRECTORIES = {
    "src/frontend": "src/compiler/frontend",
    "src/optimizer": "src/compiler/optimizer",
    "src/backend": "src/compiler/backend",
    "src/driver": "src/driver",
    "src/msl": "src/msl",
    "src/runtime": "src/runtime",
    "include": "include/libc",
}
LEDGER = ROOT / "config/PS2_3_0_1b198/merge.json"
# the source tree's C library files whose text is Metrowerks' own (see the module's description)
PATCHED = {"src/msl/alloc.c", "src/msl/time.c", "src/msl/scanf_core.c", "src/msl/strtoull.c", "src/msl/printf_g.c"}
SOURCE_VERSION = "3.0.1b198"  # the source tree's name for this version


def blank(text):
    """TEXT with comments and string and character literals turned into spaces (newlines kept)."""
    out, i, n = [], 0, len(text)
    while i < n:
        if text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
        elif text[i] in "\"'":
            j = i + 1
            while j < n and text[j] != text[i] and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            j = min(j + 1, n)
        else:
            out.append(text[i])
            i += 1
            continue
        out.append(re.sub(r"[^\n]", " ", text[i:j]))
        i = j
    return "".join(out)


def blocks(text):
    """TEXT's lines grouped into top-level blocks: a block ends at a blank line, a closing brace or a semicolon at file
    level, outside every preprocessor conditional; a comment or a directive stays with what follows it."""
    lines = text.split("\n")
    if lines and lines[-1] == "":
        lines.pop()
    plain = blank(text).split("\n")
    guard = include_guard(plain)
    out, current, braces, frames = [], [], 0, []
    for number, (line, code) in enumerate(zip(lines, plain)):
        current.append(line)
        stripped = code.strip()
        directive = re.match(r"#\s*(\w+)", stripped)
        if guard and number in guard:
            # an include guard's lines are blocks of their own, and what it guards is at top level
            out.append(current)
            current = []
            continue
        if directive:
            # braces count in the first branch of a conditional only: a function whose head is written twice, once
            # for each version, opens one block
            word = directive.group(1)
            if word in ("if", "ifdef", "ifndef"):
                frames.append([braces, None])
            elif word in ("elif", "else") and frames:
                if frames[-1][1] is None:
                    frames[-1][1] = braces
                braces = frames[-1][0]
            elif word == "endif" and frames:
                start, first = frames.pop()
                braces = braces if first is None else first
        else:
            braces += code.count("{") - code.count("}")
        conditionals = len(frames)
        if braces or conditionals or line.rstrip().endswith("\\"):
            continue
        if not stripped or (not directive and stripped.endswith(("}", ";", "};"))) or (directive and word == "endif"):
            out.append(current)
            current = []
    if current:
        out.append(current)
    return out


def merge(ours, theirs):
    """The merged text, and the share of OURS's lines it has in common with THEIRS (0 to 1). THEIRS's blocks are
    compared as PS2 b198 reads them (other builds' conditionals off, see OFF); a block that is only another build's
    is kept as it is, and a block both have is written as THEIRS has it."""
    a, every = blocks(ours), blocks(theirs)

    def key(block):
        # an include guard is the same block whatever its name (the check makes sure nothing else uses the name)
        text = "\n".join(block)
        guard = re.fullmatch(r"\s*#\s*(ifndef|define)\s+\w+\s*", text)
        return f"#{guard.group(1)} guard" if guard else " ".join(tokens(off(text)))

    b = [j for j, block in enumerate(every) if key(block) or not tokens("\n".join(block))]
    matcher = difflib.SequenceMatcher(None, [key(x) for x in a], [key(every[j]) for j in b], autojunk=False)
    out, shared, done = [], 0, 0

    def upto(j):
        # THEIRS's blocks before its J-th that no build of PS2 b198 reads, kept where they are
        nonlocal out, done
        while done < j:
            if done not in b:
                out += every[done]
            done += 1

    for tag, i1, i2, j1, j2 in matcher.get_opcodes():
        upto(b[j1] if j1 < len(b) else len(every))
        if tag == "equal":
            for i, j in zip(range(i1, i2), b[j1:j2]):
                upto(j)
                out += every[j]
                done = j + 1
                shared += weight(a[i])
            continue
        mine = [line for block in a[i1:i2] for line in block]
        other = []
        for j in b[j1:j2]:
            # (another build's blocks inside the stretch stay in it)
            other += [line for k in range(done, j) for line in every[k]]
            other += every[j]
            done = j + 1
        if not tokens("\n".join(mine)):
            mine = []  # (blank lines only: nothing to choose)
        if not tokens("\n".join(other)):
            out += other
            other = []
        if mine and other:
            out += [OPEN_PS2, *mine, "#else", *other, "#endif"]
        elif mine:
            out += [OPEN_PS2, *mine, "#endif"]
        elif other:
            out += [OPEN_OTHER, *other, "#endif"]
    upto(len(every))
    total = sum(weight(block) for block in a)
    return "\n".join(out) + "\n", (shared / total if total else 1.0)


def weight(block):
    """How many of BLOCK's lines count towards the share: none for what every header has (its guard, the extern "C"
    wrapping), one for each other line with code."""
    code = [line for line in block if tokens(line)]
    wrapping = r'#\s*(ifn?def\s+\w+|define\s+\w+|endif|else)|extern\s+"C"\s*\{|\}'
    if all(re.fullmatch(wrapping, " ".join(tokens(line))) or re.fullmatch(wrapping, line.strip()) for line in code):
        return 0
    return len(code)


def include_guard(plain):
    """The line numbers of PLAIN's include guard (#ifndef, #define and the last #endif), or None."""
    code = [(n, line.strip()) for n, line in enumerate(plain) if line.strip()]
    if len(code) < 3:
        return None
    first, second, last = code[0], code[1], code[-1]
    name = re.fullmatch(r"#\s*ifndef\s+(\w+)", first[1])
    if not name or not re.fullmatch(rf"#\s*define\s+{name.group(1)}", second[1]) or not re.match(r"#\s*endif", last[1]):
        return None
    depth = 0
    for n, line in code[:-1]:
        word = re.match(r"#\s*(\w+)", line)
        word = word.group(1) if word else None
        depth += word in ("if", "ifdef", "ifndef")
        depth -= word == "endif"
        if depth == 0 and n != second[0]:
            return None  # the first #ifndef closes before the end: not a guard
    return {first[0], second[0], last[0]}


def unguarded(text):
    """TEXT without its include guard's lines (or as it is when it has none)."""
    lines = text.split("\n")
    guard = include_guard(blank(text).split("\n")) or set()
    return "\n".join(line for n, line in enumerate(lines) if n not in guard)


def guard_name(text):
    guard = include_guard(blank(text).split("\n"))
    return re.search(r"ifndef\s+(\w+)", text).group(1) if guard else None


def off(text):
    """TEXT as PS2 b198 reads it: other builds' own conditionals (OFF) resolved false, everything else kept."""
    out, stack = [], []
    for line in text.split("\n"):
        stripped = line.strip()
        word = re.match(r"#\s*(\w+)\s*(!?)\s*(\w*)", stripped)
        kind = word.group(1) if word else None
        if kind == "if" and word.group(3) in OFF:
            stack.append(["off", word.group(2) == "!"])
            continue
        if kind in ("if", "ifdef", "ifndef"):
            stack.append(["other", True])
        elif kind == "else" and stack and stack[-1][0] == "off":
            stack[-1][1] = not stack[-1][1]
            continue
        elif kind == "endif" and stack:
            if stack.pop()[0] == "off":
                continue
        if all(taking for _, taking in stack):
            out.append(line)
    return "\n".join(out)


def resolve(text, ps2):
    """TEXT as the preprocessor sees it for PS2 b198 (PS2 true) or for every other version: the merge's own
    conditionals are taken or dropped, every other line is kept."""
    out, stack = [], []
    for line in text.split("\n"):
        stripped = line.strip()
        if stripped in (OPEN_PS2, OPEN_OTHER):
            stack.append(["merge", (stripped == OPEN_PS2) == ps2])
            continue
        word = re.match(r"#\s*(\w+)", stripped)
        word = word.group(1) if word else None
        if word in ("if", "ifdef", "ifndef"):
            stack.append(["other", True])
        elif word == "else" and stack and stack[-1][0] == "merge":
            stack[-1][1] = not stack[-1][1]
            continue
        elif word == "endif" and stack:
            if stack.pop()[0] == "merge":
                continue
        if all(taking for _, taking in stack):
            out.append(line)
    return "\n".join(out)


def tokens(text):
    """C tokens, comments and white space ignored: what the compiler reads."""
    return re.findall(
        r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|\w+|\S', re.sub(r"/\*.*?\*/|//[^\n]*", " ", text, flags=re.S)
    )


def his_files():
    """This tree's own C files (as the commit before the merge has them), by lowercase name."""
    listed = subprocess.run(
        ["git", "ls-tree", "-r", "--name-only", base(), "--", "src", "include"],
        cwd=ROOT,
        capture_output=True,
        text=True,
        check=True,
    ).stdout.split()
    found = {}
    for relative in listed:
        if not relative.startswith(NATIVE):
            found.setdefault(relative.rsplit("/", 1)[-1].lower(), []).append(relative)
    return found


def destination(relative):
    parent, name = relative.rsplit("/", 1)
    return f"{DIRECTORIES.get(parent, parent)}/{name}"


def separate(relative):
    """Where a PS2 file goes beside this tree's different file of the same name: src/ps2 and include/ps2, or for the
    C library and the runtime (which keep the libraries' own layout) a ps2 folder of their own."""
    top, rest = relative.split("/", 1)
    if top == "src" and rest.startswith(("msl/", "runtime/")):
        library, name = rest.split("/", 1)
        return f"src/{library}/ps2/{name}"
    return f"{top}/ps2/{rest}"


def plan(source, threshold):
    """[(source file, file here, how, share)] for every C file of SOURCE: how is new, merged or separate, share the
    part of the source file's lines a merge would have in common with this tree's file."""
    theirs = his_files()
    out = []
    for top in ("src", "include"):
        for path in sorted((source / top).rglob("*")):
            if not path.is_file() or path.suffix not in (".c", ".cpp", ".h", ".inc", ".r"):
                continue
            relative = path.relative_to(source).as_posix()
            if relative in PATCHED:
                out.append((relative, f"src/msl/ps2/{path.name}", "patched", 0.0))
                continue
            folder = destination(relative).rsplit("/", 1)[0]
            # a header is the same file only in the same folder; a source may sit in src/driver or src/compiler/driver
            candidates = [
                c
                for c in theirs.get(path.name.lower(), [])
                if (c.rsplit("/", 1)[0] == folder if top == "include" else c.startswith("src/"))
            ]
            if not candidates:
                out.append((relative, destination(relative), "new", 0.0))
                continue
            target = sorted(candidates, key=lambda c: (not c.startswith(folder), c))[0]
            _, share = merge(path.read_text(errors="replace"), upstream(target))
            if share * 100 >= threshold:
                out.append((relative, target, "merged", share))
            else:
                out.append((relative, separate(relative), "separate", share))
    return out


def upstream(path):
    """PATH as this tree had it before any PS2 b198 file came in."""
    return subprocess.run(
        ["git", "show", f"{base()}:{path}"], cwd=ROOT, capture_output=True, text=True, check=True
    ).stdout


def apply(source, rows):
    # a merge done before is undone first: its own files go, the merged ones are put back as they were
    if LEDGER.exists():
        for row in json.loads(LEDGER.read_text()):
            if row["how"] == "merged":
                (ROOT / row["file"]).write_text(upstream(row["file"]))
            else:
                (ROOT / row["file"]).unlink(missing_ok=True)
    ledger, moved = [], {}
    for relative, target, how, share in rows:
        ours = (source / relative).read_text(errors="replace")
        path = ROOT / target
        if how == "merged":
            text, _ = merge(ours, upstream(target))
        else:
            text = ours
        if how != "patched":
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(text)
        ledger.append({"source": relative, "file": target, "how": how, "shared": round(share, 2)})
        moved[relative] = target
    LEDGER.write_text(json.dumps(ledger, indent=1) + "\n")
    # formatted as the rest of this tree is (the libraries' own sources are left as they are)
    formatter = shutil.which("clang-format", path=str(ROOT / ".venv/bin")) or shutil.which("clang-format")
    written = [
        row["file"]
        for row in ledger
        if not row["file"].startswith(("src/msl/", "src/runtime/")) and row["file"].endswith((".c", ".cpp", ".h"))
    ]
    for i in range(0, len(written), 100):
        for _ in range(3):  # (a few constructs settle only on a second pass)
            subprocess.run([formatter, "-i", *written[i : i + 100]], cwd=ROOT, check=True)
    # the source tree's configuration, with its sources where they are here
    config = source / "config" / SOURCE_VERSION
    units = json.loads((config / "sources.json").read_text())
    kept = {moved[s]: u for s, u in units.items()}
    (LEDGER.parent / "sources.json").write_text(json.dumps(kept, indent=2) + "\n")
    splits = re.sub(
        r"^(src/\S+):$",
        lambda m: f"{moved.get(m.group(1), m.group(1))}:",
        (config / "splits.txt").read_text(),
        flags=re.M,
    )
    (LEDGER.parent / "splits.txt").write_text(splits)
    (LEDGER.parent / "symbols.txt").write_text((config / "symbols.txt").read_text())


def check(source):
    problems = 0
    for row in json.loads(LEDGER.read_text()):
        if row["how"] == "patched":
            continue  # (made by the build; msl.py make checks its patches)
        text = (ROOT / row["file"]).read_text(errors="replace")
        ours = (source / row["source"]).read_text(errors="replace")
        if tokens(off(unguarded(resolve(text, True)))) != tokens(off(unguarded(ours))):
            print(f"{row['file']}: for PS2 b198 it is not {row['source']}")
            problems += 1
        mine = guard_name(ours)
        if row["how"] == "merged" and mine and mine != guard_name(text):
            users = subprocess.run(["grep", "-rlw", mine, "src", "include"], cwd=source, capture_output=True, text=True)
            if [f for f in users.stdout.split() if f != row["source"]]:
                print(f"{row['file']}: its include guard is not {mine}, which other files test")
                problems += 1
        if row["how"] == "merged":
            if tokens(resolve(text, False)) != tokens(upstream(row["file"])):
                print(f"{row['file']}: for the other versions it is not what it was")
                problems += 1
    print(f"{problems} problem(s)")
    return 1 if problems else 0


def base():
    """The commit before the one that added the merge's ledger, or this one while no commit has it."""
    added = subprocess.run(
        ["git", "log", "-1", "--format=%H", "--diff-filter=A", "--", LEDGER.relative_to(ROOT).as_posix()],
        cwd=ROOT,
        capture_output=True,
        text=True,
        check=True,
    ).stdout.strip()
    return f"{added}^" if added else "HEAD"


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("source", type=Path)
    parser.add_argument("--plan", action="store_true", help="print where each file would go, and change nothing")
    parser.add_argument("--check", action="store_true", help="check the merge already made")
    parser.add_argument("--threshold", type=float, default=20, help="the share of lines in common to merge (%%)")
    args = parser.parse_args()
    if args.check:
        return check(args.source)
    rows = plan(args.source, args.threshold)
    if args.plan:
        for relative, target, how, share in rows:
            print(f"{how:9} {share:4.0%} {relative} -> {target}")
        return 0
    apply(args.source, rows)
    return 0


if __name__ == "__main__":
    sys.exit(main())
