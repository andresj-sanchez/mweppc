#!/usr/bin/env python3
"""Describe the original executable for `delink ida-split`, without IDA.

    describe.py ORIGINAL.exe SYMBOLS.txt SPLITS.txt OUT_DIR

delink splits a binary from a small JSON that IDA would normally export: segments, functions, names and
relocations. Everything in it can be read from the executable itself, so this writes the same JSON:

- segments: the PE section table;
- relocations: the PE base relocation table (every absolute address in the image);
- functions: found from the disassembly (targets of calls, and code that starts on a 16-byte boundary after
  the padding that follows a return or a jump, which is how this compiler lays functions out). It is a
  first pass, good enough to split; names and exact sizes come from symbols.txt as they are worked out;
- names: SYMBOLS.txt (config/PS2_3_0_1b198/symbols.txt), one `name = section:0xADDRESS; // type:function size:0xN` per line.

It also writes splits.json from SPLITS.txt, where a unit is a line `path/to/Unit.c:` followed by indented
`section start:0xA end:0xB` lines (decomp-toolkit's format).
"""

import hashlib
import json
import re
import struct
import subprocess
import sys
from bisect import bisect_left, bisect_right
from pathlib import Path

CLASSES = {
    ".text": "CODE",
    ".rdata": "CONST",
    ".exc": "CONST",
    ".edata": "CONST",
    ".data": "DATA",
    ".CRT": "DATA",
    ".bss": "BSS",
    ".idata": "XTRN",
}
KINDS = {".text": "function_ranges", ".rdata": "rdata", ".exc": "rdata", ".data": "data", ".CRT": "data", ".bss": "bss"}


def sections(data):
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    count = struct.unpack_from("<H", data, pe + 6)[0]
    optional = struct.unpack_from("<H", data, pe + 20)[0]
    base = struct.unpack_from("<I", data, pe + 24 + 28)[0]
    table = pe + 24 + optional
    found = []
    for i in range(count):
        name, vsize, vaddr, rsize, raw = struct.unpack_from("<8sIIII", data, table + 40 * i)
        flags = struct.unpack_from("<I", data, table + 40 * i + 36)[0]
        found.append(
            {
                "name": name.rstrip(b"\0").decode(),
                "start": base + vaddr,
                "end": base + vaddr + max(vsize, rsize if vsize == 0 else vsize),
                "raw": raw,
                "rsize": rsize,
                "flags": flags,
            }
        )
    return base, found


def relocations(data, base, found):
    reloc = next((s for s in found if s["name"] == ".reloc"), None)
    out = []
    if not reloc:
        return out

    def offset(address):
        for s in found:
            if s["start"] <= address < s["start"] + s["rsize"]:
                return s["raw"] + address - s["start"]

    pos = reloc["raw"]
    end = pos + reloc["rsize"]
    while pos + 8 <= end:
        page, size = struct.unpack_from("<II", data, pos)
        if size < 8:
            break
        for i in range((size - 8) // 2):
            entry = struct.unpack_from("<H", data, pos + 8 + 2 * i)[0]
            if entry >> 12 != 3:
                continue  # only HIGHLOW carries an address
            address = base + page + (entry & 0xFFF)
            at = offset(address)
            if at is not None:
                out.append(
                    {
                        "addr": f"0x{address:X}",
                        "type": "OFF32",
                        "size": 4,
                        "target": f"0x{struct.unpack_from('<I', data, at)[0]:X}",
                    }
                )
        pos += size
    return out


def functions(exe, text):
    """Function starts from the disassembly; see the module docstring."""
    listing = subprocess.run(
        ["objdump", "-d", "-M", "intel", "--no-show-raw-insn", str(exe)], capture_output=True, text=True
    ).stdout
    rows = [(int(m.group(1), 16), m.group(2).strip()) for m in re.finditer(r"^\s*([0-9a-f]+):\s+(.*)$", listing, re.M)]
    rows = [(a, t) for a, t in rows if text["start"] <= a < text["end"]]
    starts = {text["start"]}
    padding = re.compile(r"nop|int3|lea\s+e(\w\w),\[e\1\+0x0\]|mov\s+e(\w\w),e\2|xchg\s+e(\w\w),e\3")
    for index, (address, instruction) in enumerate(rows):
        call = re.match(r"call\s+0x([0-9a-f]+)", instruction)
        if call and text["start"] <= int(call.group(1), 16) < text["end"]:
            starts.add(int(call.group(1), 16))
        if address % 16 == 0 and index and not padding.match(instruction):
            back = index - 1
            while back > 0 and padding.match(rows[back][1]):
                back -= 1
            if back != index - 1 and re.match(r"ret|jmp", rows[back][1]):
                starts.add(address)
            elif back == index - 1 and re.match(r"ret", rows[back][1]):
                starts.add(address)
    ordered = sorted(starts)
    result = []
    where = {a: i for i, (a, _) in enumerate(rows)}
    for start, following in zip(ordered, ordered[1:] + [text["end"]]):
        if start not in where:
            continue
        last = where[start]
        while last + 1 < len(rows) and rows[last + 1][0] < following:
            last += 1
        while last > where[start] and padding.match(rows[last][1]):
            last -= 1  # the padding is not the function's
        end = rows[last + 1][0] if last + 1 < len(rows) else following
        result.append((start, max(1, min(end, following) - start)))
    return result


def read_symbols(path):
    named = {}
    for line in Path(path).read_text().splitlines():
        m = re.match(r"\s*(\S+)\s*=\s*(\S+):0x([0-9A-Fa-f]+);(?:\s*//\s*(.*))?", line)
        if not m:
            continue
        options = dict(o.split(":", 1) for o in (m.group(4) or "").split() if ":" in o)
        named[int(m.group(3), 16)] = (m.group(1), options)
    return named


def read_splits(path, found):
    units = {}
    current = None
    for line in Path(path).read_text().splitlines():
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        if not line[0].isspace():
            current = re.sub(r"\.(c|cpp|cp|s)$", ".obj", line.strip().rstrip(":"))
            units[current] = {"functions": [], "function_ranges": [], "rdata": [], "data": [], "bss": []}
            continue
        m = re.match(r"\s+(\S+)\s+start:0x([0-9A-Fa-f]+)\s+end:0x([0-9A-Fa-f]+)", line)
        if m and current:
            units[current][KINDS[m.group(1)]].append([f"0x{int(m.group(2), 16):X}", f"0x{int(m.group(3), 16):X}"])
    return units


def main():
    exe, symbols, splits, out = sys.argv[1:5]
    data = Path(exe).read_bytes()
    base, found = sections(data)
    named = read_symbols(symbols)
    text = next(s for s in found if s["name"] == ".text")
    out = Path(out)
    out.mkdir(parents=True, exist_ok=True)
    # (the disassembly and the relocations depend only on the executable: kept from the last run while it is the same)
    stat = Path(exe).stat()
    key = [stat.st_size, stat.st_mtime_ns, hashlib.sha1(Path(__file__).read_bytes()).hexdigest()]
    cache = out / "executable.json"
    try:
        kept = json.loads(cache.read_text())
        if kept["key"] != key:
            raise ValueError
        listed, fixups = [tuple(f) for f in kept["functions"]], kept["relocations"]
    except (OSError, ValueError, KeyError):
        listed, fixups = functions(exe, text), relocations(data, base, found)
        cache.write_text(json.dumps({"key": key, "functions": listed, "relocations": fixups}))
    funcs = []
    names = []
    sizes = dict(listed)
    for address, (name, options) in named.items():
        if options.get("type") == "function" and "size" in options:
            sizes[address] = int(options["size"], 16)
    starts = sorted(sizes)
    inside = set()
    for address, (name, options) in named.items():  # a given size overrides the starts found inside it
        if options.get("type") == "function" and "size" in options and address in sizes:
            within = starts[bisect_right(starts, address) : bisect_left(starts, address + sizes[address])]
            inside.update(a for a in within if a not in named)
    for a in inside:
        del sizes[a]
    for start in sorted(sizes):
        name = named.get(start, (f"fn_{start:08X}", {}))[0]
        funcs.append(
            {
                "start": f"0x{start:X}",
                "size": f"0x{sizes[start]:X}",
                "name": name,
                "thunk": False,
                "lib": False,
                "static": False,
                "public": True,
                "thunk_target": None,
            }
        )
        names.append({"addr": f"0x{start:X}", "name": name, "public": True, "weak": False, "is_func": True})
    for address, (name, options) in sorted(named.items()):
        if options.get("type") != "function":
            names.append({"addr": f"0x{address:X}", "name": name, "public": True, "weak": False, "is_func": False})
    segments = [
        {
            "name": s["name"],
            "start": f"0x{s['start']:X}",
            "end": f"0x{s['end']:X}",
            "perm_r": True,
            "perm_w": bool(s["flags"] & 0x80000000),
            "perm_x": bool(s["flags"] & 0x20000000),
            "class": CLASSES[s["name"]],
            "bitness": 32,
        }
        for s in found
        if s["name"] in CLASSES
    ]
    (out / "symbols.json").write_text(
        json.dumps(
            {
                "delink_ida_version": 1,
                "meta": {
                    "arch": "x86",
                    "procname": "metapc",
                    "bits": 32,
                    "endian": "little",
                    "image_base": f"0x{base:X}",
                    "min_ea": segments[0]["start"],
                    "max_ea": f"0x{max(s['end'] for s in found if s['name'] in CLASSES):X}",
                    "filetype": "PE",
                    "input_file": Path(exe).name,
                },
                "segments": segments,
                "functions": funcs,
                "names": names,
                "relocations": fixups,
            }
        )
    )
    (out / "splits.json").write_text(json.dumps(read_splits(splits, found), indent=2))
    print(f"{len(funcs)} functions, {len(names)} names, {len(segments)} segments -> {out}")


if __name__ == "__main__":
    main()
