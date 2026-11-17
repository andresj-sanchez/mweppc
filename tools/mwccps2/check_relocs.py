#!/usr/bin/env python3
"""Check that a unit's functions refer to the same things as the original's, address for address.

    python3 tools/mwccps2/check_relocs.py UNIT [FUNCTION ...]        UNIT like ps2/backend/InstrSelection

objdiff counts a function as exact when its instructions match, but it does not check where a relocation
points: a call to the wrong function or a load from the wrong global still scores 100%, and so does a
switch whose case table sends the cases to the wrong code. This compares, for every function both objects
define, the address each relocation resolves to, in order, and the case tables of switches.

In the original's split object every target is a function named fn_XXXXXXXX (or by symbols.txt) or an
offset from delink's __delink_ida_data_start or __delink_ida_bss_start. In ours, a target is a name: it is
resolved through config/PS2_3_0_1b198/symbols.txt. A string is compared by its text.
"""

import re
import struct
import sys
from pathlib import Path

import b198


def coff(path):
    data = Path(path).read_bytes()
    nsec, _, symptr, nsym, opt = struct.unpack_from("<HIIIH", data, 2)
    strtab = symptr + nsym * 18

    def name_at(raw):
        if raw[:4] == b"\0\0\0\0":
            off = struct.unpack_from("<I", raw, 4)[0]
            end = data.index(b"\0", strtab + off)
            return data[strtab + off : end].decode("latin1")
        return raw.rstrip(b"\0").decode("latin1")

    sections = []
    for i in range(nsec):
        base = 20 + opt + 40 * i
        name = name_at(data[base : base + 8])
        size, rawptr, relptr, _, nrel = struct.unpack_from("<IIIIH", data, base + 16)
        relocs = [struct.unpack_from("<IIH", data, relptr + 10 * k) for k in range(nrel)]
        sections.append(
            {"name": name, "data": data[rawptr : rawptr + size] if rawptr else b"\0" * size, "relocs": relocs}
        )
    symbols = {}
    i = 0
    while i < nsym:
        raw = data[symptr + 18 * i : symptr + 18 * i + 18]
        value, sec, typ, scl, aux = struct.unpack_from("<IhHBB", raw, 8)
        symbols[i] = (name_at(raw[:8]), sec, value, scl)
        i += 1 + aux
    return sections, symbols


def addresses():
    """Our names to original addresses."""
    found = {}
    for line in (b198.CONFIG / "symbols.txt").read_text().splitlines():
        m = re.match(r"\s*(\S+)\s*=\s*\.\w+:0x([0-9A-Fa-f]+);", line)
        if m:
            found[m.group(1)] = int(m.group(2), 16)
            found.setdefault(re.sub(r"@\d+$", "", m.group(1)), int(m.group(2), 16))  # callers see the undecorated name
    return found


def exe_offset(exe, sections, address):
    for base, raw, size in sections:
        if base <= address < base + size:
            return raw + address - base


def exe_string(exe, sections, address):
    for base, raw, size in sections:
        if base <= address < base + size:
            off = raw + address - base
            return exe[off : exe.index(b"\0", off)]


def import_table(exe, pe, image):
    """Original address of each import slot to the function name it holds."""
    rva, size = struct.unpack_from("<II", exe, pe + 24 + 96 + 8)
    n = struct.unpack_from("<H", exe, pe + 6)[0]
    opt = struct.unpack_from("<H", exe, pe + 20)[0]
    sections = [struct.unpack_from("<8sIIII", exe, pe + 24 + opt + 40 * i) for i in range(n)]

    def off(r):
        for _, vs, va, rs, raw in sections:
            if va <= r < va + max(vs, rs):
                return raw + r - va

    slots = {}
    d = off(rva)
    while struct.unpack_from("<I", exe, d + 12)[0]:
        lookup, _, _, _, first = struct.unpack_from("<IIIII", exe, d)
        k = 0
        thunks = off(lookup or first)
        while True:
            entry = struct.unpack_from("<I", exe, thunks + 4 * k)[0]
            if not entry:
                break
            if not entry & 0x80000000:
                name = exe[off(entry) + 2 : exe.index(b"\0", off(entry) + 2)].decode("latin1")
                slots[image + first + 4 * k] = name
            k += 1
        d += 20
    return slots


def main():
    unit = sys.argv[1]
    wanted = set(sys.argv[2:])
    exe = b198.ORIGINAL.read_bytes()
    pe = struct.unpack_from("<I", exe, 0x3C)[0]
    n = struct.unpack_from("<H", exe, pe + 6)[0]
    opt = struct.unpack_from("<H", exe, pe + 20)[0]
    image = struct.unpack_from("<I", exe, pe + 24 + 28)[0]
    pesec = [struct.unpack_from("<8sIIII", exe, pe + 24 + opt + 40 * i) for i in range(n)]
    pesec = [(image + va, raw, rs) for _, vs, va, rs, raw in pesec]
    iat = import_table(exe, pe, image)
    bases = {"__delink_ida_data_start": 0x5C8000, "__delink_ida_bss_start": 0x5FC000}
    names = addresses()

    splits = (b198.CONFIG / "splits.txt").read_text()
    m = re.search(
        re.escape(f"src/{unit}.") + r"\w+:\s*\n\s*\.text\s+start:0x([0-9A-Fa-f]+)\s+end:0x([0-9A-Fa-f]+)", splits
    )
    if m is None:
        match = re.search(
            re.escape(f"src/{unit}.")
            + r"\w+:\s*\n(?P<ranges>(?:\s*\.(?:rdata|data|bss)\s+start:0x[0-9A-Fa-f]+\s+end:0x[0-9A-Fa-f]+\s*\n)+)",
            splits,
        )
        if match is None:
            raise ValueError(f"{unit}: missing .text and no data-only split ranges")
        print("0 function(s) checked, 0 with differences (data-only unit)")
        return
    unit_start, unit_end = int(m.group(1), 16), int(m.group(2), 16)
    osec, osym = coff(b198.BUILD / "obj" / "src" / f"{unit}.obj")
    msec, msym = coff(b198.BUILD / "src" / f"{unit}.obj")
    text = osec[0]
    ofuncs = sorted((v, nm) for nm, sec, v, scl in osym.values() if sec == 1 and scl == 2)
    ofunc_at = {
        nm: (v, (ofuncs[k + 1][0] if k + 1 < len(ofuncs) else len(text["data"]))) for k, (v, nm) in enumerate(ofuncs)
    }
    ofunc_addr = {}
    for nm, (v, e) in ofunc_at.items():
        ofunc_addr[nm] = names.get(nm)

    def orig_target(sym, addend):
        nm = osym[sym][0]
        m = re.match(r"__delink_ida_(?:data|bss|rdata)_([0-9A-Fa-f]+)$", nm)  # a data range delink names itself
        if m:
            return int(m.group(1), 16) + addend
        if nm in bases:
            return bases[nm] + addend
        m = re.match(r"fn_([0-9A-F]{8})$", nm)
        if m:
            return int(m.group(1), 16) + addend
        if nm in names:
            return names[nm] + addend
        if osym[sym][1] == 1:
            return ("local", osym[sym][2] + addend)
        return ("?", nm, addend)

    def ours_target(sym, addend, sections):
        nm, sec, value, scl = msym[sym]
        nm = re.sub(r"@\d+$", "", nm)  # the __stdcall decoration: _OS_Open@12 is _OS_Open
        if nm.startswith("__imp__"):
            return ("imp", re.sub(r"@\d+$", "", nm[7:]))  # an import: __imp__GetLastError@0 is GetLastError
        if nm in names:
            return names[nm] + addend
        if sec > 0 and sections[sec - 1]["name"].startswith(".rdata"):
            if sections[sec - 1]["relocs"]:  # a switch's case table
                entries = []
                for off, tsym, typ in sorted(sections[sec - 1]["relocs"]):
                    target = struct.unpack_from("<i", sections[sec - 1]["data"], off)[0] + msym[tsym][2]
                    entries.append(target)
                return ("table", entries)
            blob = sections[sec - 1]["data"][value + addend :]
            return ("string", blob[: blob.index(b"\0")] if b"\0" in blob else blob)
        if sec > 0:
            return ("local", sections[sec - 1]["name"], value + addend)
        return ("?", nm, addend)

    problems = 0
    checked = 0
    for nm, sec, value, scl in msym.values():
        if scl != 2 or sec <= 0 or not msec[sec - 1]["name"].startswith(".text") or nm not in ofunc_at:
            continue
        if wanted and nm not in wanted:
            continue
        ostart, oend = ofunc_at[nm]
        orel = sorted((off - ostart, sym) for off, sym, typ in text["relocs"] if ostart <= off < oend)
        size = len(msec[sec - 1]["data"])
        mrel = sorted((off - value, sym) for off, sym, typ in msec[sec - 1]["relocs"] if value <= off < value + size)
        ot = []
        for off, sym in orel:
            addend = struct.unpack_from("<i", text["data"], ostart + off)[0]
            if osym[sym][0].startswith("__imp__"):
                continue  # an import: the loop over ours reads the original's slot at this call
            k = ostart + off
            if (
                k >= 3
                and text["data"][k - 3] == 0xFF
                and text["data"][k - 2] == 0x24
                and text["data"][k - 1] & 0xC7 == 0x85
            ):
                continue  # a switch's case table (jmp [reg*4+table]): compared apart, as ours are
            ot.append(orig_target(sym, addend))
        mt = []
        for off, sym in mrel:
            addend = struct.unpack_from("<i", msec[sec - 1]["data"], value + off)[0]
            target = ours_target(sym, addend, msec)
            if isinstance(target, tuple) and target[0] == "imp":
                at = exe_offset(exe, pesec, unit_start + ostart + off)
                slot = struct.unpack_from("<I", exe, at)[0] if at is not None else 0
                if iat.get(slot) != target[1]:
                    mt.append(("import", target[1], iat.get(slot)))
                continue
            if isinstance(target, int) and unit_start <= target - addend < unit_end and off not in {o for o, _ in orel}:
                # a call within the unit: the original's split object has it resolved, without a relocation
                there = unit_start + ostart + off + 4 + struct.unpack_from("<i", text["data"], ostart + off)[0]
                if there != target - addend:
                    print(f"{nm}: the call at +{off:#x} goes to {there:#x} in the original, {target - addend:#x} here")
                    problems += 1
                continue
            mt.append(target)
        checked += 1
        tables = [b for b in mt if isinstance(b, tuple) and b[0] == "table"]  # case tables, compared apart
        mt = [b for b in mt if not (isinstance(b, tuple) and b[0] == "table")]
        code = text["data"][ostart:oend]
        otables = []
        for k in range(len(code) - 6):  # jmp [reg*4+table] carries the table's address
            if code[k] == 0xFF and code[k + 1] == 0x24 and code[k + 2] & 0xC7 == 0x85:
                offset = exe_offset(exe, pesec, unit_start + ostart + k + 3)
                otables.append(
                    struct.unpack_from("<I", exe, offset)[0]
                    if offset is not None
                    else struct.unpack_from("<I", code, k + 3)[0]
                )
        for i, b in enumerate(tables):
            ours = tuple(e - value for e in b[1])
            if i >= len(otables):
                print(f"{nm}: switch {i} has no counterpart in the original")
                problems += 1
                continue
            offsets = [exe_offset(exe, pesec, otables[i] + 4 * j) for j in range(len(ours))]
            if any(offset is None for offset in offsets):
                print(f"{nm}: switch {i} has an unmapped original table at {otables[i]:#x}")
                problems += 1
                continue
            entries = [struct.unpack_from("<I", exe, offset)[0] for offset in offsets]
            orig = tuple(e - (unit_start + ostart) for e in entries)
            if orig != ours:
                print(
                    f"{nm}: switch {i} sends its cases elsewhere: original {[hex(x) for x in orig]}, ours {[hex(x) for x in ours]}"
                )
                problems += 1
        bad = []
        for k in range(max(len(ot), len(mt))):
            a = ot[k] if k < len(ot) else None
            b = mt[k] if k < len(mt) else None
            if isinstance(b, tuple) and b[0] == "local" and isinstance(a, int):
                continue  # jumps to our own labels
            if isinstance(a, tuple) and a[0] == "local":
                continue
            if a != b and not (isinstance(a, int) and isinstance(b, int) and a == b):
                if isinstance(a, int) and isinstance(b, tuple) and b[0] == "string":
                    if exe_string(exe, pesec, a) == b[1]:
                        continue
                bad.append((k, a if not isinstance(a, int) else hex(a), b if not isinstance(b, int) else hex(b)))
        if bad:
            problems += 1
            print(f"{nm}: {len(bad)} relocation(s) differ")
            for k, a, b in bad[:8]:
                print(f"    #{k}: original {a}, ours {b}")
    print(f"{checked} function(s) checked, {problems} with differences")
    sys.exit(1 if problems else 0)


if __name__ == "__main__":
    main()
