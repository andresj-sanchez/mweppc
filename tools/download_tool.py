#!/usr/bin/env python3

import argparse
import hashlib
import io
import json
import os
import platform
import re
import shutil
import struct
import subprocess
import sys
import tarfile
import tempfile
import time
import urllib.error
import urllib.request
import zipfile
import zlib
from pathlib import Path

OBJDIFF_TAG = "v3.8.1"
WIBO_TAG = "1.2.0"
COMPILERS_TAG = "20250812"
UNSHIELDSHARP_TAG = "3.0.0"
DELINK_TAG = "v0.16.4"
# CodeWarrior 9.2 and 9.4 for Windows, for the PS2 compiler's version: 9.2's x86 compiler (3.2.2 build 396) builds
# its sources, 9.4's linker (3.2.4 build 418) links it back together
CW92_INSTALLER = (
    "https://archive.org/download/cwpro9/Metrowerks%20CodeWarrior%20for%20Windows%209.4.7z/"
    "Metrowerks%20CodeWarrior%20for%20Windows%209.4%2FCW_Windows_9.2.exe"
)
CW94_ARCHIVE = "https://archive.org/download/CodewarriorForWindows/CodeWarrior%209.4.7z"
# the PS2 compiler's build 198, from decomp.me's public image (fetched by its single layer's digest)
PS2_B198_IMAGE = "decompme/compilers/ps2/mwcps2-3.0.1b198-051011"
PS2_B198_LAYER = "sha256:627867f8704e14e2c8d428520f6de2668d7712903b9571e9aecf1ce628b4c60b"
PS2_B198_FILES = {
    "mwccps2.exe": "41983e892a7b4a34b3ba96c2dd9d45f6a52d96a5ac4120dc479246786b124184",
    "LMGR8C.DLL": "996041141858fd4724ec36add0913755a0067498b93271d7d26e4a1bfa04a561",
    "LMGR326B.DLL": "bd9415ab11f4aef42c372e8547a8303b02464b59549745be597cf09e460737e8",
}
PRO4_ISO = "https://archive.org/download/cwpro4/CW_PRO_R4.ISO"
PRO5_BIN = "https://archive.org/download/cwpro5/Tools%20-%20Windows%2095%20NT/CW_PRO5.bin"
PRO6_ISO = "https://archive.org/download/codewarrior-6.0/CW_Tools_6.0.iso"
PRO53_UPDATER = (
    "https://archive.org/download/ftp-metrowerks-updates-archive/ftp_metrowerks_updates.7z/"
    "Metrowerks/CWWindows5/Pro5.3_Factory_Upd_NoLibs.exe"
)
DOWNLOADS = Path(__file__).resolve().parent.parent / "build/downloads"

SHA256 = {
    "objdiff-cli-linux-aarch64": "a84dc05de65eb981c97ca85890bbf2cf8e1def8a4ce062144ecc4d0d686a993a",
    "objdiff-cli-linux-i686": "fcde0f3029b4564d7ffb612904232cc2f912dbf1dbea251969c186dbcf204fec",
    "objdiff-cli-linux-x86_64": "c8290281e82114bcc1a06ff73061110d3902a177822e750337de2537188e358f",
    "objdiff-cli-macos-arm64": "98f8275c27900c4fe2248fce3af37617658be49648fa7dbb5b376371f046dfdb",
    "objdiff-cli-macos-x86_64": "13e2535c252b8ecb6dba76b5aaef0bc03c6e36460c739fe5ac35af16d898501e",
    "objdiff-cli-windows-arm64.exe": "ef51c4bab8aeded0ccaa039921266babf26e67f84e406f64e77d92f4076932d1",
    "objdiff-cli-windows-x86.exe": "c5faf4225a700ef6f4f1b1c511b3ab7029162af32b008183c001d1b0a5498c16",
    "objdiff-cli-windows-x86_64.exe": "f2a8865f3b928dc412e4f633626fe138edc79077d6fcf23e63b7cfe649c154b4",
    "wibo-i686": "2575d3b0a2f408b2c2b0850db56f1af5d005a138394a6774eba77b6708ecc304",
    "wibo-macos": "2b3000ef6a7a490c24ccd71967735ae0005e218922e51806cca1b8d77fd3cf7c",
    "wibo-x86_64": "13f86a2d618f0dbe67179d349625345eabf9b46450295cb4c904e49f6aff85af",
    "UnshieldSharp_3.0.0_net10.0_linux-arm64_release.zip": "068612055c46c669230adc46dfdd0db2e8bacda38a3507cc793065321ded0a2f",
    "UnshieldSharp_3.0.0_net10.0_linux-x64_release.zip": "e2fb8fd7f83f59fa9543ca50532a9929735790a728c1d445baa4524cee933760",
    "UnshieldSharp_3.0.0_net10.0_osx-arm64_release.zip": "d916e19287eb57cde9a531a3224ae627f87b5f3898b439c6651802334124fe83",
    "UnshieldSharp_3.0.0_net10.0_osx-x64_release.zip": "6f698e7f18db74978c60f74977dc44de108c0b888fc9ef169f97fe1a64775f76",
    "UnshieldSharp_3.0.0_net10.0_win-arm64_release.zip": "b19e14a8f7669a29f57df8be0525a52a2f1ffb0406ea4158cc563ac2877cf8e4",
    "UnshieldSharp_3.0.0_net10.0_win-x64_release.zip": "d6eac922c9ce094fc69b4546ae182f39b4995e61f3ac5409f5b4e24d553232a4",
    "UnshieldSharp_3.0.0_net10.0_win-x86_release.zip": "185d4a1ebdc56a30a13afe22ce7455405d8dc1fdb0755103d9eb0c337afd6afe",
    "delink-linux-x86_64": "31f221ddaf9e12e01a2b7a8f71fc6e62d35605e5e331138dc2e59de4f22f39e4",
    "delink-macos-arm64": "7cf0182a559731188447d97c429870ab7ece49204471fdd6a3e7ce99323a5c5b",
    "delink-macos-x86_64": "9bb721bcdec74ab4b79edcd9c6ec9c69bc05e5d035ce026b5244a68fab47d3cd",
    "delink-windows-x86_64.exe": "06e6d5189c39c54794b9d3af4f02df6af09830aaa3caaa7d45ef3f20aa2faf1c",
    "cw92": "6836e6a03c23b730fa106b04223548ac6cdff28112b5882ef6cb5f9abe34f193",
    "cw92_installer": "86608e30a15f501065fc0e0aab55b0ac3372aa02818f7f37f2aa20e47dccd6dc",
    "cw94_mwld": "badeeefa1a175bbf2ebc273d67eeda9b414002327bf2102d3db9242e74fc6bd1",
    "compilers": "f7fdc6f47d61f2e1728ba6b8dd28f13b0d510405b300d2b107e7cedd4242e706",
    "pro53_updater": "0919270219d1495764169ca190d0f341389d5aac41e509977007b8808034ec95",
    "pro4": "82d8a3e219217bea7dedd544aeeec623f01cc5ac6d6101b399a0df4c5c826b72",
    "pro5": "738dd623e16597922d2784facf5738377370b45cf2e055a6006093d142570c03",
    "pro6": "41ce15fd2c716f8d0ee5c8e8bf14aa3ad8d3eb715eace8c61cfdec70d3ff418c",
    "lib": "0214ffb064993884b397f5459107c7ba6595effe9d9c6823735eadee7dd70e81",
}


def checked(name, data):
    digest = hashlib.sha256(data).hexdigest()
    if digest != SHA256[name]:
        raise SystemExit(f"{name}: the download's SHA-256 is {digest}, not {SHA256[name]}: it changed upstream")
    return data


def tree_digest(files):
    return hashlib.sha256(
        "".join(f"{path} {hashlib.sha256(data).hexdigest()}\n" for path, data in sorted(files.items())).encode()
    ).hexdigest()


def get(url, headers=None):
    for attempt in range(6):
        try:
            with urllib.request.urlopen(urllib.request.Request(url, headers=headers or {}), timeout=300) as response:
                return response.read()
        except (urllib.error.URLError, TimeoutError) as error:
            if attempt == 5 or isinstance(error, urllib.error.HTTPError) and error.code < 500:
                raise
            print(f"{url}: {error}; retrying", file=sys.stderr)
            time.sleep(2**attempt)


SERVERS = {}


def fetch(url, headers=None):
    item = re.match(r"https://archive\.org/download/([^/]+)/(.+)", url)
    if not item or ".7z/" in item[2]:
        return get(url, headers)
    if item[1] not in SERVERS:
        metadata = json.loads(get(f"https://archive.org/metadata/{item[1]}"))
        SERVERS[item[1]] = [f"https://{server}{metadata['dir']}/" for server in metadata["workable_servers"]]
    for i, server in enumerate(SERVERS[item[1]]):
        try:
            return get(server + item[2], headers)
        except urllib.error.URLError as error:
            if i == len(SERVERS[item[1]]) - 1:
                raise
            print(f"{server}: {error}; trying the next server", file=sys.stderr)


def release(repository, tag, asset):
    if asset not in SHA256:
        raise SystemExit(f"{repository} {tag} has no build {asset} for this machine")
    return checked(asset, fetch(f"https://github.com/{repository}/releases/download/{tag}/{asset}"))


def objdiff():
    system = {"Darwin": "macos", "Linux": "linux", "Windows": "windows"}[platform.system()]
    machine = platform.machine().lower()
    machine = {"amd64": "x86_64", "arm64": "arm64" if system == "macos" else "aarch64"}.get(machine, machine)
    suffix = ".exe" if system == "windows" else ""
    return release("encounter/objdiff", OBJDIFF_TAG, f"objdiff-cli-{system}-{machine}{suffix}")


def wibo():
    arch = "macos" if platform.system() == "Darwin" else platform.machine().lower()
    return release("decompals/wibo", WIBO_TAG, f"wibo-{arch}")


class Disc:
    BLOCK = 128

    def __init__(self, url, raw):
        self.url, self.blocks = url, {}
        self.sector, self.skip = (2352, 24) if raw else (2048, 0)

    def block(self, index):
        if index not in self.blocks:
            first = index * self.BLOCK * self.sector
            data = fetch(self.url, {"Range": f"bytes={first}-{first + self.BLOCK * self.sector - 1}"})
            self.blocks[index] = b"".join(
                data[i * self.sector + self.skip : i * self.sector + self.skip + 2048]
                for i in range(len(data) // self.sector)
            )
        return self.blocks[index]

    def read(self, lba, size):
        data = b""
        while len(data) < size:
            index, offset = divmod(lba, self.BLOCK)
            chunk = self.block(index)[offset * 2048 :]
            if not chunk:
                break
            data += chunk
            lba += self.BLOCK - offset
        return data[:size]

    def file(self, path):
        record = self.read(16, 2048)[156:190]
        for part in path.split("/"):
            directory = self.read(struct.unpack_from("<I", record, 2)[0], struct.unpack_from("<I", record, 10)[0])
            cursor, record = 0, None
            while cursor < len(directory):
                size = directory[cursor]
                if not size:
                    cursor = (cursor // 2048 + 1) * 2048
                    continue
                if directory[cursor + 33 : cursor + 33 + directory[cursor + 32]] == part.encode():
                    record = directory[cursor : cursor + size]
                    break
                cursor += size
            if record is None:
                raise SystemExit(f"{self.url}: no {path}")
        return DiscFile(self, struct.unpack_from("<I", record, 2)[0], struct.unpack_from("<I", record, 10)[0])


class DiscFile(io.RawIOBase):
    def __init__(self, disc, lba, size):
        self.disc, self.lba, self.size, self.pos = disc, lba, size, 0

    def seekable(self):
        return True

    def tell(self):
        return self.pos

    def seek(self, offset, whence=0):
        self.pos = (offset, self.pos + offset, self.size + offset)[whence]
        return self.pos

    def read(self, size=-1):
        size = min(self.size - self.pos if size < 0 else size, self.size - self.pos)
        if size <= 0:
            return b""
        sector, skip = divmod(self.pos, 2048)
        data = self.disc.read(self.lba + sector, size + skip)[skip:]
        self.pos += len(data)
        return data


def tools_zip_member(name, url, raw, archive, member):
    return checked(name, zipfile.ZipFile(Disc(url, raw).file(archive)).read(member))


def updater():
    path = DOWNLOADS / "Pro5.3_Factory_Upd_NoLibs.exe"
    if path.exists():
        return checked("pro53_updater", path.read_bytes())
    data = checked("pro53_updater", fetch(PRO53_UPDATER))
    DOWNLOADS.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(".part")
    temporary.write_bytes(data)
    temporary.replace(path)
    return data


def unshieldsharp():
    system = {"Darwin": "osx", "Linux": "linux", "Windows": "win"}[platform.system()]
    machine = platform.machine().lower()
    machine = {"x86_64": "x64", "amd64": "x64", "aarch64": "arm64", "i386": "x86", "i686": "x86"}.get(machine, machine)
    path = (
        DOWNLOADS / f"UnshieldSharp-{UNSHIELDSHARP_TAG}" / ("UnshieldSharp.exe" if system == "win" else "UnshieldSharp")
    )
    if not path.exists():
        asset = f"UnshieldSharp_{UNSHIELDSHARP_TAG}_net10.0_{system}-{machine}_release.zip"
        data = release("mnadareski/UnshieldSharp", UNSHIELDSHARP_TAG, asset)
        with zipfile.ZipFile(io.BytesIO(data)) as archive:
            member = next(n for n in archive.namelist() if Path(n).name == path.name)
            path.parent.mkdir(parents=True, exist_ok=True)
            temporary = path.with_suffix(".part")
            temporary.write_bytes(archive.read(member))
            temporary.chmod(0o755)
            temporary.replace(path)
    return path


def pro53_tree(root):
    data = updater()
    for offset in (100746, 40487531):
        _, _, _, _, _, _, packed, _, length, extra = struct.unpack_from("<5H3I2H", data, offset + 4)
        name = data[offset + 30 : offset + 30 + length].decode()
        begin = offset + 30 + length + extra
        (root / name).write_bytes(zlib.decompress(data[begin : begin + packed], -15))
    subprocess.run(
        [unshieldsharp(), f"-o={root / 'files'}", root / "data1.hdr"],
        check=True,
        stdout=subprocess.DEVNULL,
        env={**os.environ, "DOTNET_SYSTEM_GLOBALIZATION_INVARIANT": "1"},
    )
    return root / "files"


def pro53(tool="mwcc.exe"):
    with tempfile.TemporaryDirectory() as temporary:
        files = pro53_tree(Path(temporary))
        return next(files.rglob(tool)).read_bytes()


def delink():
    system = {"Darwin": "macos", "Linux": "linux", "Windows": "windows"}[platform.system()]
    machine = {"amd64": "x86_64"}.get(platform.machine().lower(), platform.machine().lower())
    suffix = ".exe" if system == "windows" else ""
    return release("dbalatoni13/delink", DELINK_TAG, f"delink-{system}-{machine}{suffix}")


def ps2_b198(output):
    """mwccps2.exe 3.0.1 build 198 and the two license manager libraries it loads, into OUTPUT's folder."""
    token = json.loads(get(f"https://ghcr.io/token?scope=repository:{PS2_B198_IMAGE}:pull"))["token"]
    blob = get(f"https://ghcr.io/v2/{PS2_B198_IMAGE}/blobs/{PS2_B198_LAYER}", {"Authorization": f"Bearer {token}"})
    if "sha256:" + hashlib.sha256(blob).hexdigest() != PS2_B198_LAYER:
        raise SystemExit(f"{PS2_B198_IMAGE}: the layer is not {PS2_B198_LAYER}")
    prefix = "compilers/ps2/mwcps2-3.0.1b198-051011/"
    with tarfile.open(fileobj=io.BytesIO(blob), mode="r:gz") as archive:
        for name, digest in PS2_B198_FILES.items():
            data = archive.extractfile(prefix + name).read()
            if hashlib.sha256(data).hexdigest() != digest:
                raise SystemExit(f"{name}: the SHA-256 is not {digest}")
            temporary = output.parent / (name + ".part")
            temporary.write_bytes(data)
            temporary.replace(output.parent / name)


def cw92_installer():
    """CodeWarrior 9.2's installer, kept in build/downloads: the compiler and the C library come out of it."""
    path = DOWNLOADS / "CW_Windows_9.2.exe"
    if path.exists():
        return checked("cw92_installer", path.read_bytes())
    data = checked("cw92_installer", fetch(CW92_INSTALLER))
    DOWNLOADS.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(".part")
    temporary.write_bytes(data)
    temporary.replace(path)
    return data


def cw92_files(root):
    """The installed tree of CodeWarrior 9.2, under ROOT/files. The installer is a cabinet holding InstallShield's:
    cabextract (or 7z) opens the first, unshield (or UnshieldSharp) the second."""
    (root / "installer.exe").write_bytes(cw92_installer())
    if shutil.which("cabextract"):
        command = ["cabextract", "-q", "-d", root / "cab", root / "installer.exe"]
    elif shutil.which("7z"):
        command = ["7z", "x", f"-o{root / 'cab'}", root / "installer.exe"]
    else:
        raise SystemExit("cabextract or 7z is needed to unpack CodeWarrior 9.2's installer")
    subprocess.run(command, check=True, stdout=subprocess.DEVNULL)
    if shutil.which("unshield"):
        command = ["unshield", "-d", root / "files", "x", root / "cab" / "data1.cab"]
    else:
        command = [unshieldsharp(), f"-o={root / 'files'}", root / "cab" / "data1.hdr"]
    subprocess.run(
        command,
        check=True,
        stdout=subprocess.DEVNULL,
        env={**os.environ, "DOTNET_SYSTEM_GLOBALIZATION_INVARIANT": "1"},
    )
    return root / "files"


def cw92():
    """The command-line compiler, Tools/Other_Metrowerks_Tools/Command_Line_Tools/mwcc.exe."""
    with tempfile.TemporaryDirectory() as temporary:
        files = cw92_files(Path(temporary))
        return checked("cw92", next(files.rglob("Command_Line_Tools/mwcc.exe")).read_bytes())


def msl92(output):
    """The C library's sources of CodeWarrior 9.2 (Tools/MSL), under OUTPUT's folder as MSL/."""
    with tempfile.TemporaryDirectory() as temporary:
        files = cw92_files(Path(temporary))
        target = output.parent / "MSL"
        shutil.rmtree(target, ignore_errors=True)
        shutil.copytree(files / "Tools/MSL", target)
    output.touch()


def cw94_mwld():
    """CodeWarrior 9.4's archive holds a ThinApp package: one large file with the installed tree, every file stored as
    it is. The linker is found in it by its banner and cut out by its own section table. Needs bsdtar or 7z."""
    with tempfile.TemporaryDirectory() as temporary:
        root = Path(temporary)
        (root / "cw94.7z").write_bytes(fetch(CW94_ARCHIVE))
        if shutil.which("bsdtar"):
            command = ["bsdtar", "-xf", root / "cw94.7z", "-C", root]
        elif shutil.which("7z"):
            command = ["7z", "x", f"-o{root}", root / "cw94.7z"]
        else:
            raise SystemExit("bsdtar or 7z is needed to unpack CodeWarrior 9.4")
        subprocess.run(command, check=True, stdout=subprocess.DEVNULL)
        image = next(root.rglob("*.dat")).read_bytes()
        for found in re.finditer(b"MZ", image):
            start = found.start()
            if start % 0x1000 or start + 0x40 > len(image):
                continue
            pe = struct.unpack_from("<I", image, start + 0x3C)[0]
            if not 0x40 <= pe < 0x400 or image[start + pe : start + pe + 4] != b"PE\0\0":
                continue
            count, optional = (
                struct.unpack_from("<H", image, start + pe + 6)[0],
                struct.unpack_from("<H", image, start + pe + 20)[0],
            )
            table = start + pe + 24 + optional
            end = max(sum(struct.unpack_from("<II", image, table + 40 * i + 16)) for i in range(count))
            body = image[start : start + end]
            if b"Metrowerks Linker for Windows/x86" in body and hashlib.sha256(body).hexdigest() == SHA256["cw94_mwld"]:
                return body
    raise SystemExit("CodeWarrior 9.4's linker was not found in its package")


LIB = [
    ("msl/MSL_Common/Include", "MSL/MSL_C/MSL_Common/Include", None),
    ("msl/MSL_Common/Src", "MSL/MSL_C/MSL_Common/Src", None),
    ("msl/MSL_Cpp/MSL_Common/Include", "MSL/MSL_C++/MSL_Common/Include", None),
    ("msl/MSL_Cpp/MSL_Common/Src", "MSL/MSL_C++/MSL_Common/Src", None),
    ("msl/MSL_Win32/Include", "MSL/MSL_C/MSL_Win32/Include", None),
    ("msl/MSL_Win32/Src", "MSL/MSL_C/MSL_Win32/Src", None),
    ("msl/MSL_X86", "MSL/MSL_C/MSL_X86", None),
    (
        "msl/win32sdk",
        "Win32-x86 Support/Headers/Win32 SDK",
        [
            "BaseTsd.h",
            "POPPACK.H",
            "PSHPACK2.H",
            "PSHPACK4.H",
            "PSHPACK8.H",
            "TCHAR.H",
            "WINBASE.H",
            "WINDEF.H",
            "WINERROR.H",
            "WINNT.H",
            "WINUSER.H",
            "sys/TYPES.H",
        ],
    ),
    ("msl/extra", "MSL/MSL_C/MSL_Common/Include", ["os_enum.h"]),
    ("msl/extra", "Win32-x86 Support/Headers/Win32 SDK", ["x86_prefix.h"]),
    ("msl/extra", "MSL/MSL_C/MSL_Common/Src", ["printf.c"]),
    ("runtime", "Win32-x86 Support/Libraries/Runtime/(Sources)", None),
]


LIB53 = [
    ("msl/MSL_Common/Include", "MSL/MSL_C/MSL_Common/Include", None),
    ("msl/MSL_Common/Src", "MSL/MSL_C/MSL_Common/Src", None),
    ("msl/MSL_Win32/Include", "MSL/MSL_C/MSL_Win32/Include", None),
    ("msl/MSL_Win32/Src", "MSL/MSL_C/MSL_Win32/Src", None),
    ("msl/MSL_X86", "MSL/MSL_C/MSL_X86", None),
    ("msl/MSL_Cpp/MSL_Common/Include", "MSL/MSL_C++/MSL_Common/Include", None),
    (
        "msl/win32sdk",
        "Win32-x86_Support/Headers/Win32_SDK",
        next(e[2] for e in LIB if e[0] == "msl/win32sdk"),
    ),
    ("msl/extra", "MSL/MSL_C/MSL_Common/Include", ["os_enum.h"]),
    ("msl/extra", "MSL/MSL_C/MSL_Common/Src", ["printf.c"]),
    ("runtime", "Win32-x86_Support/Libraries/Runtime/(Sources)", None),
]


TIME_PATCHES = [
    (
        b"\ttime_t\ttime = __get_time();\r\n\r\n\tif (timer)",
        b"\ttime_t\ttime = __get_time();\r\n\r\n\t__to_gm_time(&time);\r\n\r\n\tif (timer)",
    ),
    (b"\tif (!__to_gm_time(&time))\r\n\t\treturn(NULL);\r\n", b""),
    (
        b"\tstatic struct tm\ttm;\r\n\r\n\tif (!timer)\r\n\t\tclear_tm(&tm);\r\n\telse\r\n\t\t__time2tm(*timer, &tm);",
        b"\tstatic struct tm\ttm;\r\n\ttime_t\ttime;\r\n\r\n\tif (!timer)\r\n\t\tclear_tm(&tm);\r\n\telse\r\n\t{\r\n"
        b"\t\ttime = *timer;\r\n\t\tsubtract_time_zone_bias((int *)&time);\r\n\t\t__time2tm(time, &tm);\r\n\t}",
    ),
    (
        b"#ifndef _No_Time_OS_Support              /* mea 970720 */",
        b"extern int subtract_time_zone_bias(int *);\r\n\r\n#ifndef _No_Time_OS_Support              /* mea 970720 */",
    ),
]


def patch(files, path, replacements, count=1):
    data = files[path]
    for old, new in replacements:
        if data.count(old) != count:
            raise SystemExit(f"{path}: unexpected contents ({old[:40]!r})")
        data = data.replace(old, new)
    return data


def lib(output):
    root = output.parent
    files = {}
    with zipfile.ZipFile(Disc(PRO5_BIN, True).file("CODEWA~1.ZIP;1")) as archive:
        names = [n for n in archive.namelist() if not n.endswith("/")]
        for directory, source, wanted in LIB:
            for name in names:
                relative = name[len(source) + 1 :] if name.startswith(source + "/") else None
                if relative is None or wanted is not None and relative not in wanted:
                    continue
                files[f"{directory}/{relative}"] = archive.read(name)
    with tempfile.TemporaryDirectory() as temporary:
        tree = pro53_tree(Path(temporary))
        for directory, source, wanted in LIB53:
            base = tree / source
            for path in sorted(base.rglob("*")) if base.exists() else ():
                relative = path.relative_to(base).as_posix()
                if not path.is_file() or wanted is not None and relative.lower() not in {f.lower() for f in wanted}:
                    continue
                target = f"{directory}/{relative}"
                target = next((f for f in files if f.lower() == target.lower()), target)
                files[target] = path.read_bytes()
    files["msl/extra/printf.c"] = patch(
        files,
        "msl/extra/printf.c",
        [(b'buff_ptr = "";      /*97010mani@be */', b'buff_ptr = "(null)";      /*97010mani@be */')],
    )
    files["msl/extra/time.c"] = patch(files, "msl/MSL_Common/Src/time.c", TIME_PATCHES)
    files["msl/extra/startup.win32.c"] = patch(
        files, "msl/MSL_Win32/Src/startup.win32.c", [(b"->translate = 1;\t/* 961208 KO */", b"->translate = 0;")], 3
    )
    digest = tree_digest(files)
    if digest != SHA256["lib"]:
        raise SystemExit(f"lib: the files' SHA-256 is {digest}, not {SHA256['lib']}: a download changed upstream")
    for path, data in files.items():
        (root / path).parent.mkdir(parents=True, exist_ok=True)
        (root / path).write_bytes(data)
    output.touch()


TOOLS = (
    "objdiff-cli",
    "wibo",
    "compilers",
    "pro4",
    "pro5",
    "pro53",
    "pro6",
    "lib",
    "pro53_mwld",
    "delink",
    "cw92",
    "cw94_mwld",
    "msl92",
    "ps2_b198",
)


def main():
    parser = argparse.ArgumentParser(description="Fetch a build tool, checked against its SHA-256.")
    parser.add_argument("tool", choices=TOOLS, metavar="TOOL", help=", ".join(TOOLS))
    parser.add_argument(
        "outputs", nargs="+", type=Path, metavar="OUTPUT", help="what it makes (ninja names every file a tool yields)"
    )
    args = parser.parse_args()
    tool, output = args.tool, args.outputs[0]
    output.parent.mkdir(parents=True, exist_ok=True)
    if sys.platform == "win32" and tool in ("pro5", "pro53", "pro6"):
        shutil.copy(Path(__file__).parent / "lmgr_stub/lmgr326b.dll", output.parent)
    if (
        tool in ("compilers", "pro4", "pro5", "pro53", "pro6", "lib", "pro53_mwld", "cw92", "cw94_mwld")
        and output.exists()
    ):
        output.touch()
        return
    if tool == "lib":
        lib(output)
        return
    if tool == "ps2_b198":
        if not output.exists():
            ps2_b198(output)
        return
    if tool == "msl92":
        if not output.exists():
            msl92(output)
        return
    if tool == "compilers":
        data = checked("compilers", fetch(f"https://files.decomp.dev/compilers_{COMPILERS_TAG}.zip"))
        with zipfile.ZipFile(io.BytesIO(data)) as archive:
            archive.extractall(output.parent)
        output.touch()
        return
    data = {
        "objdiff-cli": objdiff,
        "wibo": wibo,
        "pro4": lambda: tools_zip_member(
            "pro4", PRO4_ISO, False, "CODEWA~1/CODEWA~1.ZIP;1", "Tools/Command Line Tools/mwcc.exe"
        ),
        "pro5": lambda: tools_zip_member("pro5", PRO5_BIN, True, "CODEWA~1.ZIP;1", "Tools/Command Line Tools/mwcc.exe"),
        "pro53": pro53,
        "pro53_mwld": lambda: pro53("mwld.exe"),
        "delink": delink,
        "cw92": cw92,
        "cw94_mwld": cw94_mwld,
        "pro6": lambda: tools_zip_member(
            "pro6", PRO6_ISO, True, "CODEWA~1.ZIP;1", "Other Metrowerks Tools/Command Line Tools/mwcc.exe"
        ),
    }[tool]()
    temporary = output.with_name(output.name + ".part")
    temporary.write_bytes(data)
    temporary.chmod(0o755)
    temporary.replace(output)


if __name__ == "__main__":
    main()
