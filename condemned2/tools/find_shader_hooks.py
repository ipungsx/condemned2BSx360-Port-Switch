#!/usr/bin/env python3
"""Find D3DDevice_CreateVertexShader / D3DDevice_CreatePixelShader in a default.xex .text dump.

    find_shader_hooks.py <.text section dump> <text base hex> <generated/default>

The shader hooks of src/c2_shader_dump.cpp wrap these two functions, and their addresses differ from disc to disc
(Condemned 1 has them at 0x826D2780 / 0x826D25F0, Condemned 2 at 0x8265DF50 / 0x8265DE40). A wrong address means
no shader is ever dumped or registered, and the deko3d renderer draws nothing: a black screen with sound.

Shape searched (XDK 2007/2008 D3D):
  - r3 is the container: the function reads its flags (+0), virtualSize (+4) and physicalSize (+8) early;
  - it allocates twice: first with `lis r4,0x2480` (virtual object + copy), then with `lis r4,0xB580` (physical);
  - the vertex shader clears a large object (`li r5,0x368` or similar, then memset) and calls a fill routine;
    the pixel shader does not.
Needs: pip install capstone
"""
import bisect, glob, os, re, sys
import capstone

text_path, base, gen = sys.argv[1], int(sys.argv[2], 16), sys.argv[3]
t = open(text_path, "rb").read()
starts = []
for fn in glob.glob(os.path.join(gen, "*_register.cpp")):
    for line in open(fn):
        m = re.search(r"SetFunction\(0x([0-9A-F]+),", line)
        if m:
            starts.append(int(m.group(1), 16))
starts.sort()
md = capstone.Cs(capstone.CS_ARCH_PPC, capstone.CS_MODE_32 | capstone.CS_MODE_BIG_ENDIAN)
md.skipdata = True
found = []
for k, a in enumerate(starts[:-1]):
    e = starts[k + 1]
    if e - a > 0x400 or not (base <= a < base + len(t)):
        continue
    ins = list(md.disasm(t[a - base:e - base], a))
    alias, reads = {"r3"}, set()
    for i in ins[:30]:
        m = re.match(r"(r\d+), (r\d+)$", i.op_str)
        if i.mnemonic == "mr" and m and m.group(2) in alias:
            alias.add(m.group(1))
        m = re.match(r"(r\d+), (0x[0-9a-f]+|\d+)\((r\d+)\)$", i.op_str)
        if i.mnemonic == "lwz" and m and m.group(3) in alias:
            reads.add(int(m.group(2), 0))
    ops = [(i.mnemonic, i.op_str) for i in ins]
    virt = ("lis", "r4, 0x2480") in ops
    phys = ("lis", "r4, -0x4a80") in ops
    if not ({0, 4, 8} <= reads and virt and phys):
        continue
    clears = [o for m_, o in ops if m_ == "li" and o.startswith("r5, ") and int(o.split(", ")[1], 0) >= 0x100]
    found.append((a, e - a, "VS" if clears else "PS", clears))
for a, n, kind, clears in found:
    print(f"{kind}  sub_{a:08X}  size 0x{n:X}  {'memset ' + clears[0] if clears else ''}")
if [f[2] for f in found].count("VS") != 1 or [f[2] for f in found].count("PS") != 1:
    sys.exit("expected exactly one VS and one PS: check the candidates by hand")
