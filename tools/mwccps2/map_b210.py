#!/usr/bin/env python3
"""The functions of PS2 3.0.1 build 198 and where each one is in build 210 (the PS2_3_0_1 native build's).

    map_b210.py B210.exe B210_SYMBOLS [--report REPORT] [-o config/PS2_3_0_1b198/b210.csv]

For every function of build 198 (build/PS2_3_0_1b198/delink/symbols.json, the build's description of the original)
this finds the function of build 210 with the same code: the same bytes once the absolute addresses the base
relocations list and the targets of calls and jumps are left out. The two builds share almost all of their code at
shifted addresses, so the search follows the shift. Each row: build 198's address and name, build 210's address and
name (B210_SYMBOLS, config/PS2_3_0_1/symbols.txt), whether the code is the same, whether the function is exact
in build 198's decompilation (REPORT, build/PS2_3_0_1b198/report.json), whether the build's compiled function is
build 210's too (its bytes are build 210's at that address, but for the fields the compiled object relocates), and
where build 198's name comes from:
wii (the Wii 1.0RC1 compiler's debug information, build/compilers/Wii/1.0RC1/mwcceppc.exe), gamecube (the GameCube
versions' functions here), export (the original's export table), library (a function of the C library or runtime,
named by Metrowerks' library sources), reconstructed (named for what it does, and marked so in its declaration) or
address (not named yet).
"""

import argparse
import csv
import io
import json
import re
import struct
import sys
from pathlib import Path

import b198
from mw import config as mwconfig
from mw import project, wii
from mw.coff import Object
from mw.pe import EXPORTS, PEFile


def image(path):
    """(text address, text bytes, the addresses the base relocations list) of a PE executable."""
    data = Path(path).read_bytes()
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    count, optional = struct.unpack_from("<H", data, pe + 6)[0], struct.unpack_from("<H", data, pe + 20)[0]
    base = struct.unpack_from("<I", data, pe + 24 + 28)[0]
    sections = {}
    for i in range(count):
        at = pe + 24 + optional + 40 * i
        name = data[at : at + 8].rstrip(b"\0").decode()
        sections[name] = struct.unpack_from("<IIII", data, at + 8)
    vsize, vaddr, _, raw = sections[".text"]
    text = data[raw : raw + vsize]
    relocated = set()
    rsize, raddr, _, rraw = sections[".reloc"]
    at = rraw
    while at < rraw + rsize:
        page, size = struct.unpack_from("<II", data, at)
        if size < 8:
            break
        for k in range((size - 8) // 2):
            entry = struct.unpack_from("<H", data, at + 8 + 2 * k)[0]
            if entry >> 12 == 3:
                relocated.add(base + page + (entry & 0xFFF))
        at += size
    return base + vaddr, text, relocated


def masked(start, text, relocated, address, size):
    """SIZE bytes of code at ADDRESS with relocated words and call and jump targets zeroed."""
    out = bytearray(text[address - start : address - start + size])
    for a in range(address - 3, address + size):
        if a in relocated:
            for k in range(4):
                if 0 <= a - address + k < size:
                    out[a - address + k] = 0
    i = 0
    while i < len(out) - 4:
        if out[i] in (0xE8, 0xE9):
            out[i + 1 : i + 5] = b"\0\0\0\0"
            i += 5
        else:
            i += 1
    return bytes(out)


def compiled_functions():
    """Linker name -> (code, the offsets the object relocates) of every function the build compiled. Each function is
    in a section of its own."""
    out = {}
    for path in (b198.BUILD / "src").rglob("*.obj"):
        obj = Object.read(path)
        for symbol in obj.symbols:
            if symbol.section <= 0 or symbol.section > len(obj.sections):
                continue
            section = obj.sections[symbol.section - 1]
            if not section.name.startswith(".text") or symbol.value != 0 or not symbol.name.startswith(("_", "?")):
                continue
            relocated = {r.offset + k for r in section.relocations for k in range(4)}
            out.setdefault(symbol.name, (section.data, relocated))
    return out


def names_evidence():
    """name -> where it comes from (see the module's description)."""
    official = set(wii.read(wii.DEFAULT).names().values()) if wii.DEFAULT.exists() else set()
    gamecube = {row.name for v in project.VERSIONS for row in mwconfig.functions(v)}
    pe = PEFile(b198.ORIGINAL)
    exported = set()
    table, _ = pe.directory(EXPORTS)
    if table:
        count, names = pe.word(table + 24), pe.word(table + 32)
        for i in range(count):
            name = pe.string(pe.image_base + pe.word(pe.image_base + names + 4 * i))
            exported.add(re.sub(r"@\d+$", "", name.lstrip("_")))

    library = []
    unit = None
    for line in (b198.CONFIG / "splits.txt").read_text().splitlines():
        if re.match(r"^\S+:$", line):
            unit = line[:-1]
        m = re.match(r"\s+\.text\s+start:0x([0-9A-Fa-f]+)\s+end:0x([0-9A-Fa-f]+)", line)
        if m and unit and unit.startswith(("src/msl/", "src/runtime/")):
            library.append((int(m.group(1), 16), int(m.group(2), 16)))

    def evidence(name, address):
        plain = re.sub(r"@\d+$", "", name)
        if re.fullmatch(r"fn_[0-9A-F]{8}", plain) or re.search(r"_[0-9A-F]{6}$", plain):
            return "address"
        if plain in official:
            return "wii"
        if plain in gamecube:
            return "gamecube"
        if plain in exported:
            return "export"
        if any(start <= address < end for start, end in library):
            return "library"
        return "reconstructed"

    return evidence


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("b210", type=Path)
    parser.add_argument("b210_symbols", type=Path)
    parser.add_argument("--report", type=Path, default=b198.BUILD / "report.json")
    parser.add_argument("-o", "--output", type=Path, default=b198.CONFIG / "b210.csv")
    args = parser.parse_args()
    described = json.loads((b198.BUILD / "delink/symbols.json").read_text())
    functions = sorted((int(f["start"], 16), int(f["size"], 16), f["name"]) for f in described["functions"])
    theirs = {}
    for line in args.b210_symbols.read_text().splitlines():
        m = re.match(r"(\S+) = 0x([0-9A-Fa-f]+);", line)
        if m:
            theirs.setdefault(int(m.group(2), 16), m.group(1))
    exact = set()
    if args.report.exists():
        for unit in json.loads(args.report.read_text()).get("units", []):
            for f in unit.get("functions", []):
                if f.get("fuzzy_match_percent") == 100.0 and "virtual_address" in f.get("metadata", {}):
                    exact.add(int(f["metadata"]["virtual_address"]))
    evidence = names_evidence()
    compiled = compiled_functions()
    a_start, a_text, a_relocated = image(b198.ORIGINAL)
    b_start, b_text, b_relocated = image(args.b210)
    rows, shift = [], 0
    for address, size, name in functions:
        mine = masked(a_start, a_text, a_relocated, address, size)
        found = None
        for d in [shift] + [shift + k for k in range(-0x2000, 0x2001) if k]:
            there = address + d
            if not b_start <= there < b_start + len(b_text) - size:
                continue
            if b_text[there - b_start] != a_text[address - a_start]:
                continue
            if masked(b_start, b_text, b_relocated, there, size) == mine:
                found, shift = there, d
                break
        in_b210 = ""
        if address in exact and found:
            code, relocated = compiled.get(name, compiled.get("_" + name.lstrip("_"), (None, None)))
            if code is not None:
                there = b_text[found - b_start : found - b_start + len(code)]
                same = len(there) == len(code) and all(
                    a == b or k in relocated for k, (a, b) in enumerate(zip(code, there))
                )
                in_b210 = "yes" if same else "no"
        rows.append(
            [
                f"0x{address:08X}",
                name.lstrip("_"),
                f"0x{found:08X}" if found else "",
                theirs.get(found, "") if found else "",
                "yes" if found else "no",
                "yes" if address in exact else "no",
                in_b210,
                evidence(name.lstrip("_"), address),
            ]
        )
    out = io.StringIO()
    writer = csv.writer(out, lineterminator="\n")
    writer.writerow(
        [
            "b198_address",
            "b198_name",
            "b210_address",
            "b210_name",
            "same_code",
            "exact_in_b198",
            "exact_in_b210",
            "b198_name_from",
        ]
    )
    writer.writerows(rows)
    project.write_atomic(args.output, out.getvalue())
    same = sum(1 for r in rows if r[4] == "yes")
    both = sum(1 for r in rows if r[4] == "yes" and r[5] == "yes")
    in_b210 = sum(1 for r in rows if r[6] == "yes")
    checked = sum(1 for r in rows if r[6])
    print(f"{len(rows)} functions, {same} with the same code in b210; {len(exact)} exact, {both} of them in b210 too")
    print(f"compiled against b210: {in_b210} of the {checked} exact functions found there are exact in b210")


if __name__ == "__main__":
    sys.exit(main())
