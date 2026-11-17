#!/usr/bin/env python3
"""link_config.py: the link's complete configuration for PS2 3.0.1 build 198, in build/PS2_3_0_1b198/link.

config/PS2_3_0_1b198/symbols.txt and splits.txt name only part of the original. The link needs every reference to point at a
named symbol and every byte of every section to belong to a unit, so this does what decomp-toolkit does for its projects'
unassigned parts:

- functions are config/PS2_3_0_1b198/symbols.txt's, then the ones delink's description found (build/PS2_3_0_1b198/delink), each as
  long as the next one's start allows;
- every address a base relocation points at in data gets a label (lbl_ADDRESS), and every import slot a name
  (__imp__NAME); the exports keep theirs;
- each unit takes its ranges from splits.txt, plus its functions' entries in the exception table (.exc) and their action
  data (.rdata); every range no unit holds becomes a unit of its own (auto_SECTION_ADDRESS);
- the units are put in an order that keeps each section in address order, since the linker lays out each section in the
  order of the objects.

Written: symbols.txt, splits.txt (with each data range's alignment and grouped section name), exports.txt, and notes.txt,
the mistakes in config/PS2_3_0_1b198 found on the way.
"""

import bisect
import heapq
import itertools
import json
import re
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import b198  # noqa: I001
from mw import project
from mw.pe import EXPORTS, original

SECTIONS = {
    ".text": "type:code align:16",
    ".exc": "type:rodata align:4",
    ".rdata": "type:rodata align:4",
    ".data": "type:data align:8",
    ".rsrc": "type:data align:4",
    ".CRT": "type:data align:4",
    ".idata": "type:data align:4",
    ".bss": "type:bss align:8",
}
# (the sections the units hold; the linker makes .rsrc, .idata, .edata and .reloc)
SPLIT = [".text", ".exc", ".rdata", ".data", ".CRT", ".bss"]
SYMBOL = re.compile(r"(\S+) = ([^:\s]+):0x([0-9A-Fa-f]+);(?: // (.*))?")


def main():
    if len(sys.argv) != 1:
        sys.exit(__doc__.strip())
    v = b198.VERSION
    pe = original(v)
    sec = {s.name: s for s in pe.sections}
    end_of = lambda name: sec[name].virtual_address + sec[name].virtual_size
    notes = []

    # ------------------------------------------------------------------ symbols: address -> (name, section, options)
    symbols = {}
    for line in (v.config / "symbols.txt").read_text().splitlines():
        m = SYMBOL.match(line.strip())
        if not m:
            continue
        address = int(m[3], 16)
        options = dict(o.split(":", 1) for o in (m[4] or "").split() if ":" in o)
        section = pe.section_for_address(address).name
        if section != m[2]:
            notes.append(f"symbols.txt: {m[1]} at 0x{address:08X} is in {section}, not {m[2]}")
        symbols[address] = (m[1], section, options)
    manual_functions = [
        (address, address + int(options["size"], 16))
        for address, (_, section, options) in symbols.items()
        if section == ".text" and options.get("type") == "function" and options.get("size")
    ]
    manual_functions.sort()
    manual_starts = [start for start, _ in manual_functions]
    manual_reach = list(itertools.accumulate((end for _, end in manual_functions), max))
    for function in json.loads((v.build / "delink" / "symbols.json").read_text())["functions"]:
        address = int(function["start"], 16)
        before = bisect.bisect_left(manual_starts, address)
        if before and manual_reach[before - 1] > address:
            continue
        symbols.setdefault(address, (function["name"], ".text", {"type": "function", "size": function["size"]}))
    functions = sorted(a for a, (_, s, _) in symbols.items() if s == ".text")
    for i, address in enumerate(functions):
        following = functions[i + 1] if i + 1 < len(functions) else end_of(".text")
        options = symbols[address][2]
        size = int(options.get("size", "0"), 16)
        if not size or address + size > following:
            options["size"] = f"0x{following - address:X}"
        options["type"] = "function"
    for item in pe.imports:
        name = item.name or f"{item.dll.split('.')[0]}_{item.number}"
        symbols[item.slot] = (f"__imp__{name}", ".idata", {"type": "object", "size": "0x4"})
    exports = []
    directory, _ = pe.directory(EXPORTS)
    if directory:
        _, count, functions_at, names_at, ordinals_at = struct.unpack_from("<IIIII", pe.read(directory + 20, 20))
        for i in range(count):
            name = pe.string(pe.image_base + pe.word(pe.image_base + names_at + 4 * i))
            ordinal = struct.unpack_from("<H", pe.read(pe.image_base + ordinals_at + 2 * i, 2))[0]
            address = pe.image_base + pe.word(pe.image_base + functions_at + 4 * ordinal)
            old = symbols.get(address)
            symbols[address] = (name, pe.section_for_address(address).name, dict(old[2]) if old else {"type": "object"})
            exports.append(name)
    for at in pe.base_relocations:
        value = pe.word(at)
        try:
            section = pe.section_for_address(value).name
        except ValueError:
            continue
        # (a reference into code is to the function holding it, with an addend)
        if section != ".text":
            symbols.setdefault(value, (f"lbl_{value:08X}", section, {"type": "object"}))
    for name in SPLIT:
        start = sec[name].virtual_address
        symbols.setdefault(start, (f"lbl_{start:08X}", name, {"type": "object"}))
    by_section = {}
    for address in sorted(symbols):
        by_section.setdefault(symbols[address][1], []).append(address)
    for name, addresses in by_section.items():
        if name == ".text":
            continue
        for i, address in enumerate(addresses):
            following = addresses[i + 1] if i + 1 < len(addresses) else end_of(name)
            symbols[address][2]["size"] = f"0x{following - address:X}"
            symbols[address][2].setdefault("type", "object")
    seen = {}
    for address in sorted(symbols):
        name, section, options = symbols[address]
        options["scope"] = "global"
        if name in seen:
            notes.append(
                f"symbols.txt: {name} names both 0x{seen[name]:08X} and 0x{address:08X} (the second is "
                f"renamed here; a static of two units is scope:local)"
            )
            symbols[address] = (f"{name}_{address:08X}", section, options)
        seen.setdefault(name, address)

    # ------------------------------------------------------------------ units: [name, [(section, start, end)]]
    units, current = [], None
    for line in (v.config / "splits.txt").read_text().splitlines():
        if line and not line[0].isspace() and line.endswith(":") and not line.startswith("#"):
            current = [line[:-1], []]
            units.append(current)
        elif line.strip().startswith(".") and current:
            fields = line.split()
            options = dict(x.split(":", 1) for x in fields[1:])
            current[1].append((fields[0], int(options["start"], 16), int(options["end"], 16)))
    # (the exception table: an 8-byte entry (function, its action data in .rdata) per function compiled with exceptions,
    # then a 0xFFFFFFFF terminator. A unit's functions bring their entries and action data with them.)
    exc = sec[".exc"]
    words = struct.unpack_from(f"<{exc.virtual_size // 4}I", pe.read(exc.virtual_address, exc.virtual_size))
    entries = [(exc.virtual_address + 8 * k, words[2 * k], words[2 * k + 1]) for k in range(len(words) // 2)]
    rdata = by_section[".rdata"]
    for k, (at, function, data) in enumerate(entries):
        owner = next((u for u in units if any(s == ".text" and a <= function < b for s, a, b in u[1])), None)
        if owner is None:
            continue
        following = entries[k + 1][2] if k + 1 < len(entries) else next(a for a in rdata if a > data)
        ranges = owner[1]
        # A unit may specify the exact action-data range in splits.txt. In that case
        # do not widen it to the next exception record: unrelated switch tables can
        # lie between two exception-data pointers (LongLongx86.c does this).
        covered = any(s == ".rdata" and a <= data < b for s, a, b in ranges)
        if not any(s == ".exc" for s, _, _ in ranges):
            ranges += [(".exc", at, at + 8)]
            if not covered:
                ranges.append((".rdata", data, following))
        else:
            owner[1] = [
                (s, a, at + 8)
                if s == ".exc"
                else (s, a, following)
                if s == ".rdata" and b == data and not covered
                else (s, a, b)
                for s, a, b in ranges
            ]
    for name in SPLIT:
        taken = sorted((a, b) for _, ranges in units for s, a, b in ranges if s == name)
        points = [sec[name].virtual_address] + [x for r in taken for x in r] + [end_of(name)]
        for i in range(0, len(points), 2):
            start, end = points[i], points[i + 1]
            if name == ".text":
                # (a unit that ends where its last function's code ends, before the padding: the function decides)
                for function in functions:
                    stop = function + int(symbols[function][2]["size"], 16)
                    if function < start < stop:
                        notes.append(
                            f"splits.txt: the .text range ending at 0x{start:08X} ends inside "
                            f"{symbols[function][0]} (0x{function:08X}-0x{stop:08X})"
                        )
                        start = min(stop, end)
            if end > start:
                units.append([f"auto_{name[1:]}_{start:08X}", [(name, start, end)]])
    # (the exception table's terminator and the .CRT table's ends are sections of their own: see grouped())
    crt = sec[".CRT"]
    for unit in list(units):
        if not unit[0].startswith("auto_"):
            continue
        for i, (s, a, b) in enumerate(unit[1]):
            if s == ".exc" and b == end_of(".exc") and b - a > 4:
                unit[1][i] = (s, a, b - 4)
                units.append([f"auto_exc_{b - 4:08X}", [(s, b - 4, b)]])
            if s == ".CRT":
                cuts = sorted({a, b, crt.virtual_address + 4, end_of(".CRT") - 4} & set(range(a, b + 1)))
                unit[1][i] = (s, cuts[0], cuts[1])
                units += [[f"auto_CRT_{x:08X}", [(s, x, y)]] for x, y in zip(cuts[1:], cuts[2:])]
    # (.bss ends where the original's last variables end, with no alignment after them: they were left to the linker's
    # COMMON allocation, which starts on an 8-byte boundary. How far back the original's COMMON variables reach is not
    # known; the tail from the last 8-aligned variable is made COMMON.)
    common = None
    if end_of(".bss") % 8:
        tail = max(a for a in by_section[".bss"] if a % 8 == 0)
        for unit in units:
            unit[1] = [
                (s, a, tail)
                if s == ".bss" and a < tail < b == end_of(".bss") and unit[0].startswith("auto_")
                else (s, a, b)
                for s, a, b in unit[1]
            ]
        common = "auto_bss_common"
        units.append([common, [(".bss", tail, end_of(".bss"))]])

    # ------------------------------------------------------------------ link order
    rank = {name: i for i, name in enumerate(SPLIT)}
    after = {u[0]: set() for u in units}
    needs = {u[0]: 0 for u in units}
    for name in SPLIT:
        sequence = sorted((a, u[0]) for u in units for s, a, _ in u[1] if s == name)
        for (_, x), (_, y) in zip(sequence, sequence[1:]):
            if x != y and y not in after[x]:
                after[x].add(y)
                needs[y] += 1
    key = {u[0]: min((rank[s], a) for s, a, _ in u[1]) for u in units}
    ready = [(key[n], n) for n, count in needs.items() if count == 0]
    heapq.heapify(ready)
    ordered = []
    while ready:
        _, name = heapq.heappop(ready)
        ordered.append(name)
        for following in after[name]:
            needs[following] -= 1
            if needs[following] == 0:
                heapq.heappush(ready, (key[following], following))
    if len(ordered) != len(units):
        raise project.Error("no link order keeps every section in address order: two units' ranges cross")
    position = {name: i for i, name in enumerate(ordered)}
    units.sort(key=lambda u: position[u[0]])

    def grouped(s, a, b):
        """The grouped section a range is written in, as the compiler and the runtime write them; the linker sorts
        .exc$BBB (each function's entry) before $CCC (the terminator), and .CRT$XCA, $XCU, $XCZ."""
        if s == ".exc":
            return " rename:.exc$CCC" if b == end_of(".exc") and b - a == 4 else " rename:.exc$BBB"
        if s == ".CRT":
            if a == crt.virtual_address:
                return " rename:.CRT$XCA"
            return " rename:.CRT$XCZ" if b == end_of(".CRT") and b - a == 4 else " rename:.CRT$XCU"
        return ""

    lines = ["Sections:"] + [f"\t{n:11} {o}" for n, o in SECTIONS.items()] + [""]
    for name, ranges in units:
        lines.append(f"{name}:")
        for s, a, b in ranges:
            # (a data range: the alignment its address allows, at most 8, so the linker adds no gap before it)
            extra = "" if s == ".text" else " common:true" if name == common else f" align:{min(8, a & -a)}"
            lines.append(f"\t{s:11} start:0x{a:08X} end:0x{b:08X}{extra}{grouped(s, a, b)}")
        lines.append("")
    project.write_atomic(b198.LINK / "splits.txt", "\n".join(lines))
    project.write_atomic(
        b198.LINK / "symbols.txt",
        "".join(
            f"{n} = {s}:0x{a:08X}; // " + " ".join(f"{k}:{x}" for k, x in o.items()) + "\n"
            for a, (n, s, o) in sorted(symbols.items())
        ),
    )
    project.write_atomic(b198.LINK / "exports.txt", "".join(f"{name}\n" for name in exports))
    project.write_atomic(b198.LINK / "notes.txt", "".join(f"{note}\n" for note in notes))
    print(
        f"{len(symbols)} symbols, {len(units)} units, {len(exports)} exports; {len(notes)} notes in "
        f"{project.relative(b198.LINK / 'notes.txt')}"
    )


if __name__ == "__main__":
    try:
        main()
    except project.Error as e:
        sys.exit(f"error: {e}")
