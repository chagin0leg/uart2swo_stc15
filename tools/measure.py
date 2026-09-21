from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent.parent
BUILD = ROOT / ".pio/build/STC15F104W"

asm = (BUILD / "src/main.asm").read_text(errors="replace")
idx = asm.find("_main:")
part = asm[idx:]
m = re.search(r"if \(sbi\)", part)
start = m.start() if m else 0
before = part[:start].splitlines()
loop_lab = None
for line in reversed(before[-50:]):
    mm = re.match(r"(001\d+)\$:", line.strip())
    if mm:
        loop_lab = mm.group(1)
        break
print("LOOP", loop_lab)

body_lines = []
for line in part[start:].splitlines():
    body_lines.append(line)
    s = line.strip()
    if loop_lab and s.startswith("sjmp") and loop_lab in s:
        break

cost = {
    "nop": 1, "ret": 4, "lcall": 4, "acall": 3, "ljmp": 4, "ajmp": 3, "sjmp": 3,
    "mov": 2, "clr": 2, "setb": 2, "cpl": 1, "inc": 2, "dec": 2, "add": 2, "subb": 2,
    "anl": 2, "orl": 2, "xrl": 2, "rrc": 1, "rlc": 1, "rr": 1, "rl": 1,
    "jz": 3, "jnz": 3, "jc": 3, "jnc": 3, "jb": 4, "jnb": 4, "cjne": 4, "djnz": 3,
    "push": 2, "pop": 2, "xch": 2,
}


def insn_cost(s):
    if not s or s.startswith(";") or s.endswith("$:") or s.startswith("."):
        return 0
    op = s.split("\t")[0].split()[0]
    c = cost.get(op, 2)
    if op == "mov":
        if re.match(r"mov\ta, r\d", s) or re.match(r"mov\tr\d, a", s):
            c = 1
        elif re.match(r"mov\tr\d, #", s):
            c = 1
        elif "#" in s:
            c = 3
    if op == "dec" and re.match(r"dec\tr\d", s):
        c = 1
    if op == "inc" and re.match(r"inc\tr\d", s):
        c = 1
    return c


print("--- LOOP ASM ---")
total = 0
for line in body_lines:
    s = line.strip()
    c = insn_cost(s)
    if s:
        print(f"{s}" + (f"  ; {c}" if c else ""))
        total += c
print("SUM_ALL_LINES_ROUGH", total)
print("(not path-split; see manual path analysis below)")

text = "\n".join(body_lines)
out = BUILD / "_loop_dump.asm"
BUILD.mkdir(parents=True, exist_ok=True)
out.write_text(text, encoding="utf-8")
print(f"wrote {out.relative_to(ROOT)}")
