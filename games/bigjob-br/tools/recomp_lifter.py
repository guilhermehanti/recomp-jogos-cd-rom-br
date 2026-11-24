"""Static x86-16/386 -> C lifter for BIGJOBNG.EXE (40 code segments)."""
import argparse
import json
import re
from collections import Counter, defaultdict, deque
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ANALYSIS = ROOT / "analysis"
DISASM = ANALYSIS / "disassembly"
OUT_DIR = ROOT / "build" / "recomp_gen"

LINE_RE = re.compile(r"^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2}[ \t])+)\s*(.*)$")
MEM_RE = re.compile(r"^(?:%(cs|ds|es|ss|fs|gs):)?(-?0x[0-9a-f]+|-?\d+)?(?:\(([^)]+)\))?$")

REG8 = {"%al": "AL", "%ah": "AH", "%bl": "BL", "%bh": "BH", "%cl": "CL", "%ch": "CH", "%dl": "DL", "%dh": "DH"}
REG16 = {"%ax": "AX", "%bx": "BX", "%cx": "CX", "%dx": "DX", "%si": "SI", "%di": "DI", "%bp": "BP", "%sp": "SP"}
REG32 = {"%eax": "EAX", "%ebx": "EBX", "%ecx": "ECX", "%edx": "EDX", "%esi": "ESI", "%edi": "EDI", "%ebp": "EBP", "%esp": "ESP"}
SREG = {"%es": "cpu->es", "%cs": "cpu->cs", "%ss": "cpu->ss", "%ds": "cpu->ds", "%fs": "cpu->fs", "%gs": "cpu->gs"}

JCC_MAP = {
    "jo": "CC_O(cpu)", "jno": "CC_NO(cpu)",
    "jb": "CC_B(cpu)", "jnae": "CC_B(cpu)", "jc": "CC_B(cpu)",
    "jae": "CC_AE(cpu)", "jnb": "CC_AE(cpu)", "jnc": "CC_AE(cpu)",
    "je": "CC_E(cpu)", "jz": "CC_E(cpu)",
    "jne": "CC_NE(cpu)", "jnz": "CC_NE(cpu)",
    "jbe": "CC_BE(cpu)", "jna": "CC_BE(cpu)",
    "ja": "CC_A(cpu)", "jnbe": "CC_A(cpu)",
    "js": "CC_S(cpu)", "jns": "CC_NS(cpu)",
    "jp": "CC_P(cpu)", "jpe": "CC_P(cpu)",
    "jnp": "CC_NP(cpu)", "jpo": "CC_NP(cpu)",
    "jl": "CC_L(cpu)", "jnge": "CC_L(cpu)",
    "jge": "CC_GE(cpu)", "jnl": "CC_GE(cpu)",
    "jle": "CC_LE(cpu)", "jng": "CC_LE(cpu)",
    "jg": "CC_G(cpu)", "jnle": "CC_G(cpu)",
}


def split_operands(ops_str):
    ops, cur, depth = [], [], 0
    for ch in ops_str:
        if ch == "(":
            depth += 1
            cur.append(ch)
        elif ch == ")":
            depth -= 1
            cur.append(ch)
        elif ch == "," and depth == 0:
            ops.append("".join(cur).strip())
            cur = []
        else:
            cur.append(ch)
    if cur:
        s = "".join(cur).strip()
        if s:
            ops.append(s)
    return ops


def parse_imm(s):
    if not s.startswith("$"):
        return None
    v = s[1:].strip()
    return int(v, 0)


def parse_mem(s):
    """Returns (seg_expr, ea_expr) for AT&T memory operand."""
    m = MEM_RE.match(s.strip())
    if not m:
        return None
    seg_tok, disp_tok, base_tok = m[1], m[2], m[3]
    if disp_tok is None and base_tok is None:
        return None
    parts = []
    default_seg = "cpu->ds"
    if base_tok:
        bparts = [p.strip() for p in base_tok.split(",")]
        for i, bp in enumerate(bparts):
            if not bp:
                continue
            if i == 0 and bp in ("%bp", "%sp", "%ebp", "%esp"):
                default_seg = "cpu->ss"
            if bp in REG16:
                parts.append(REG16[bp])
            elif bp in REG32:
                parts.append(f"(uint16_t){REG32[bp]}")
            elif bp.isdigit() and parts:
                parts[-1] = f"({parts[-1]} * {bp})"
            else:
                return None
    if disp_tok:
        dval = int(disp_tok, 0) & 0xFFFF
        parts.append(f"0x{dval:04X}u")
    ea = "(uint16_t)(" + " + ".join(parts) + ")" if parts else "0u"
    seg = f"cpu->{seg_tok}" if seg_tok else default_seg
    return seg, ea


def op_read(s, bits, reloc_imm=None):
    s = s.strip()
    if s.startswith("$"):
        if reloc_imm is not None:
            return reloc_imm
        v = parse_imm(s)
        mask = {8: 0xFF, 16: 0xFFFF, 32: 0xFFFFFFFF}[bits]
        return f"0x{(v & mask):X}u"
    if s in REG8:  return REG8[s]
    if s in REG16: return REG16[s]
    if s in REG32: return REG32[s]
    if s in SREG:  return SREG[s]
    mem = parse_mem(s)
    if mem:
        seg, ea = mem
        return f"mem_r{bits}({seg}, {ea})"
    return None


def op_write(s, bits, val_expr):
    s = s.strip()
    if s in REG8:  return f"{REG8[s]} = (uint8_t)({val_expr});"
    if s in REG16: return f"{REG16[s]} = (uint16_t)({val_expr});"
    if s in REG32: return f"{REG32[s]} = (uint32_t)({val_expr});"
    if s in SREG:  return f"{SREG[s]} = (uint16_t)({val_expr});"
    mem = parse_mem(s)
    if mem:
        seg, ea = mem
        return f"mem_w{bits}({seg}, {ea}, (uint{bits}_t)({val_expr}));"
    return None


def parse_seg_asm(seg_id):
    insts = {}
    order = []
    for line in ((ANALYSIS / "disassembly-resync") / f"seg{seg_id:02d}.asm").read_text(encoding="utf8").splitlines():
        m = LINE_RE.match(line)
        if not m:
            continue
        off = int(m[1], 16)
        raw_bytes = m[2].strip().split()
        nbytes = len(raw_bytes)
        body = m[3]
        # Strip "# imm = ..." and "; NE ..."
        code_part = body.split(";")[0].split("#")[0].strip()
        insts[off] = (nbytes, code_part, body)
        order.append(off)
    return insts, order

_INIT_NEAR = None


def init_table_near_targets():
    """Near (type 0) records of the Borland _INIT_/_EXIT_ table in DGROUP (49:20B8..2118) carry no NE reloc."""
    global _INIT_NEAR
    if _INIT_NEAR is None:
        import struct
        inv = json.loads((ANALYSIS / "inventory.json").read_text(encoding="utf8"))
        segs = {s["id"]: s for s in inv["NE"]["BIGJOBNG.EXE"]["segments"]}
        exe = (ROOT / "build" / "BIGJOBNG.EXE").read_bytes()
        base = segs[49]["offset"]
        _INIT_NEAR = []
        for rec in range(0x20B8, 0x2118, 6):
            kind = exe[base + rec]
            off = struct.unpack_from("<H", exe, base + rec + 2)[0]
            if kind == 0 and off:
                _INIT_NEAR.append(off)
    return _INIT_NEAR

def lift_segment(seg_id, cfg, refs_by_seg, ne_entries, unsupported_counter):
    insts, order = parse_seg_asm(seg_id)
    jts_by_site = {int(jt["site"], 16): jt for jt in cfg["jump_tables"] if jt["segment"] == seg_id}

    # Index relocations by instruction offset
    relocs_in_seg = refs_by_seg[seg_id]
    reloc_by_inst = defaultdict(list)
    for off in order:
        nbytes = insts[off][0]
        for r in relocs_in_seg:
            if off <= r["site"] < off + nbytes:
                reloc_by_inst[off].append(r)

    # Seed entry points for BFS
    entries = set()
    for f in cfg["functions"]:
        if f["segment"] == seg_id:
            entries.add(int(f["start"], 16))
    for r_list in refs_by_seg.values():
        for r in r_list:
            if r["source_type"] == 3 and ":" in r["target"]:
                ts, to = r["target"].split(":", 1)
                if int(ts) == seg_id:
                    entries.add(int(to, 16))
    for jt in jts_by_site.values():
        for t in jt["targets"]:
            entries.add(int(t, 16))
    if seg_id == 1:
        entries.add(0)
        entries.update(init_table_near_targets())
    for ent in ne_entries:
        if ent[1] == seg_id:
            entries.add(ent[2])
    missing_entries = sorted(e for e in entries if e not in insts)
    if missing_entries:
        print(f'WARNING seg{seg_id:02d}: {len(missing_entries)} entry points not on an instruction boundary: ' + ', '.join(f'0x{e:04X}' for e in missing_entries[:8]))
    entries = {e for e in entries if e in insts}

    # BFS reachability inside segment
    reachable = set()
    label_targets = set(entries)
    queue = deque(sorted(entries))
    while queue:
        off = queue.popleft()
        if off in reachable or off not in insts:
            continue
        reachable.add(off)
        nbytes, code_part, _ = insts[off]
        next_off = off + nbytes

        # Parse opcode
        parts = code_part.split(None, 1)
        if not parts:
            continue
        op = parts[0]
        rest = parts[1].strip() if len(parts) > 1 else ""
        if op in ("rep", "repe", "repz", "repne", "repnz", "lock", "addr32", "data32"):
            sub = rest.split(None, 1)
            op = sub[0] if sub else op
            rest = sub[1].strip() if len(sub) > 1 else ""

        # Check jump table
        if off in jts_by_site:
            for t in jts_by_site[off]["targets"]:
                toff = int(t, 16)
                if toff in insts:
                    label_targets.add(toff)
                    if toff not in reachable:
                        queue.append(toff)
            continue

        # Control flow transfers
        if op in ("retw", "lretw", "ret", "lret", "iret", "iretw"):
            continue
        if op in ("ljmpw", "ljmp"):
            continue
        if op in ("jmp", "jmpw"):
            if not rest.startswith("*"):
                tm = re.match(r"0x([0-9a-f]+)", rest)
                if tm:
                    toff = int(tm[1], 16)
                    if toff in insts:
                        label_targets.add(toff)
                        if toff not in reachable:
                            queue.append(toff)
            continue
        if op in JCC_MAP or op in ("loop", "loopw", "loope", "loopne", "jcxz", "jecxz"):
            tm = re.match(r"0x([0-9a-f]+)", rest)
            if tm:
                toff = int(tm[1], 16)
                if toff in insts:
                    label_targets.add(toff)
                    if toff not in reachable:
                        queue.append(toff)
            if next_off in insts and next_off not in reachable:
                queue.append(next_off)
            continue
        if op in ("callw", "call") and not rest.startswith("*"):
            tm = re.match(r"0x([0-9a-f]+)", rest)
            if tm:
                toff = int(tm[1], 16)
                if toff in insts:
                    entries.add(toff)
                    label_targets.add(toff)
                    if toff not in reachable:
                        queue.append(toff)
            if next_off in insts and next_off not in reachable:
                queue.append(next_off)
            continue

        # Fallthrough
        if next_off in insts and next_off not in reachable:
            queue.append(next_off)

    # Emit C code
    lines = [
        '#include "../../runtime/cpu_ops.h"',
        '#include "../../runtime/win16_mem.h"',
        '#include "../../runtime/win16_api.h"',
        '#include <stdio.h>',
        '#include <stdlib.h>',
        '',
    ]
    for s in range(1, 41):
        lines.append(f'void seg{s:02d}_exec(CpuState *cpu, uint16_t entry_off);')
    lines.append('')
    lines.append(f'void seg{seg_id:02d}_exec(CpuState *cpu, uint16_t entry_off) {{')
    lines.append(f'    if (g_trace) fprintf(stderr, "[T] {seg_id:02d}:%04X SS:SP=%04X:%04X DS=%04X ES=%04X\\\\n", entry_off, cpu->ss, SP, cpu->ds, cpu->es);')
    lines.append('    switch (entry_off) {')
    if seg_id == 37:
        lines.append('    case 0x1ECA: win16_fiber_start(cpu); return;')
        lines.append('    case 0x1F13: win16_fiber_resume(cpu); return;')
        lines.append('    case 0x1F34: win16_fiber_yield(cpu); return;')
    for e in sorted(entries):
        if seg_id == 37 and e in (0x1ECA, 0x1F13, 0x1F34):
            continue
        if e in reachable:
            lines.append(f'    case 0x{e:04X}: goto L_{e:04X};')
    lines.append(f'    default: fprintf(stderr, "[recomp] seg{seg_id:02d} unknown entry 0x%04X (ret=%04X:%04X sp=%04X)\\n", entry_off, mem_r16(cpu->ss, (uint16_t)(SP + 2)), mem_r16(cpu->ss, SP), SP); fflush(stderr); exit(3);')
    lines.append('    }')
    lines.append('')

    sorted_reach = sorted(reachable)
    for off in sorted_reach:
        if off in label_targets:
            lines.append(f'L_{off:04X}:;')
        nbytes, code_part, orig_body = insts[off]
        next_off = off + nbytes
        inst_relocs = reloc_by_inst.get(off, [])

        # Handle prefixes (may combine rep + addr32, e.g. rep addr32 movsl)
        rep_mode = None
        is_addr32 = False
        parts = code_part.split(None, 1)
        op = parts[0] if parts else ""
        rest = parts[1].strip() if len(parts) > 1 else ""
        while op in ("rep", "repe", "repz", "repne", "repnz", "lock", "addr32", "data32"):
            if op in ("rep", "repe", "repz", "repne", "repnz"):
                rep_mode = op
            elif op == "addr32":
                is_addr32 = True
            sub = rest.split(None, 1)
            op = sub[0] if sub else ""
            rest = sub[1].strip() if len(sub) > 1 else ""

        # Check relocation-driven immediate override (source_type 2=sel, 5=off, or KERNEL!113)
        reloc_imm = None
        for r in inst_relocs:
            if r["target"] == "KERNEL!113":
                reloc_imm = "WIN16_AHSHIFT"
            elif r["source_type"] == 2 and ":" in r["target"]:
                tseg = int(r["target"].split(":")[0])
                reloc_imm = f"NE_SEG_TO_SEL({tseg})"
            elif r["source_type"] == 5 and ":" in r["target"]:
                toff = int(r["target"].split(":")[1], 16)
                reloc_imm = f"0x{toff:04X}u"

        ops = split_operands(rest) if rest else []
        emitted = None

        # ---- Control Flow ----
        if off in jts_by_site:
            jt = jts_by_site[off]
            m_ea = parse_mem(rest.lstrip("*"))
            if m_ea:
                seg_e, ea_e = m_ea
                sw = [f'    switch (mem_r16({seg_e}, {ea_e})) {{']
                for t in sorted(set(jt["targets"])):
                    tv = int(t, 16)
                    if tv in reachable:
                        sw.append(f'    case 0x{tv:04X}: goto L_{tv:04X};')
                sw.append(f'    default: fprintf(stderr, "[recomp] seg{seg_id:02d}:0x{off:04X} switch miss\\n"); return;')
                sw.append('    }')
                emitted = "\n".join(sw)

        elif op in ("nop", "wait", "fwait"):
            emitted = "    /* nop/wait */"

        elif op == "cld":
            emitted = "    cpu->df = 0;"
        elif op == "std":
            emitted = "    cpu->df = 1;"
        elif op == "clc":
            emitted = "    cpu->cf = 0;"
        elif op == "stc":
            emitted = "    cpu->cf = 1;"
        elif op == "cmc":
            emitted = "    cpu->cf ^= 1;"

        elif op in ("retw", "ret"):
            imm = parse_imm(ops[0]) if ops else 0
            if imm:
                emitted = f"    (void)pop16(cpu); SP = (uint16_t)(SP + 0x{imm:X}u); return;"
            else:
                emitted = "    (void)pop16(cpu); return;"
        elif op in ("lretw", "lret"):
            imm = parse_imm(ops[0]) if ops else 0
            if imm:
                emitted = f"    (void)pop16(cpu); cpu->cs = pop16(cpu); SP = (uint16_t)(SP + 0x{imm:X}u); return;"
            else:
                emitted = "    (void)pop16(cpu); cpu->cs = pop16(cpu); return;"

        elif op == "leave":
            emitted = "    SP = BP; BP = pop16(cpu);"
        elif op == "enter" and len(ops) >= 1:
            imm = parse_imm(ops[0]) or 0
            emitted = f"    push16(cpu, BP); BP = SP; SP = (uint16_t)(SP - 0x{imm:X}u);"

        elif op in ("jmp", "jmpw"):
            if not rest.startswith("*"):
                tm = re.match(r"0x([0-9a-f]+)", rest)
                if tm:
                    toff = int(tm[1], 16)
                    if toff in reachable:
                        emitted = f"    goto L_{toff:04X};"
            else:
                m_ea = parse_mem(rest[1:])
                if m_ea:
                    emitted = f"    recomp_dispatch_near(cpu, cpu->cs, mem_r16({m_ea[0]}, {m_ea[1]})); return;"

        elif op in JCC_MAP:
            tm = re.match(r"0x([0-9a-f]+)", rest)
            if tm:
                toff = int(tm[1], 16)
                if toff in reachable:
                    emitted = f"    if ({JCC_MAP[op]}) goto L_{toff:04X};"

        elif op in ("jcxz", "jecxz"):
            tm = re.match(r"0x([0-9a-f]+)", rest)
            if tm:
                toff = int(tm[1], 16)
                if toff in reachable:
                    emitted = f"    if (CX == 0) goto L_{toff:04X};"

        elif op in ("loop", "loopw"):
            tm = re.match(r"0x([0-9a-f]+)", rest)
            if tm:
                toff = int(tm[1], 16)
                if toff in reachable:
                    emitted = f"    CX = (uint16_t)(CX - 1); if (CX != 0) goto L_{toff:04X};"

        elif op in ("callw", "call"):
            if not rest.startswith("*"):
                tm = re.match(r"0x([0-9a-f]+)", rest)
                if tm:
                    toff = int(tm[1], 16)
                    emitted = f"    push16(cpu, 0x{next_off:04X}u); seg{seg_id:02d}_exec(cpu, 0x{toff:04X}u);"
            else:
                r_expr = op_read(rest[1:], 16)
                if r_expr:
                    emitted = f"    {{ uint16_t _t = {r_expr}; push16(cpu, 0x{next_off:04X}u); recomp_dispatch_near(cpu, cpu->cs, _t); }}"

        elif op in ("lcallw", "lcall"):
            if rest.startswith("*"):
                # QuickTime dispatcher shortcut (DS:1546 or DS:154E)
                if "0x1546" in rest and seg_id == 1:
                    emitted = "    win16_qt_dispatch(cpu, false, BX);"
                elif "0x154e" in rest and seg_id == 1:
                    emitted = "    win16_qt_dispatch(cpu, true, BX);"
                else:
                    m_ea = parse_mem(rest[1:])
                    if m_ea:
                        s, e = m_ea
                        emitted = (
                            f"    {{ uint16_t _ea = {e}; uint16_t _to = mem_r16({s}, _ea); "
                            f"uint16_t _ts = mem_r16({s}, (uint16_t)(_ea + 2)); "
                            f"push16(cpu, cpu->cs); push16(cpu, 0x{next_off:04X}u); "
                            f"recomp_dispatch_far(cpu, _ts, _to); }}"
                        )
            else:
                far_r = next((r for r in inst_relocs if r["source_type"] == 3), None)
                if far_r:
                    tgt = far_r["target"]
                    if "!" in tgt:
                        mod, ord_s = tgt.split("!", 1)
                        emitted = f'    win16_call_import(cpu, "{mod}", {int(ord_s)});'
                    elif ":" in tgt:
                        ts, to = tgt.split(":", 1)
                        tseg, toff = int(ts), int(to, 16)
                        emitted = (
                            f"    push16(cpu, cpu->cs); push16(cpu, 0x{next_off:04X}u); "
                            f"cpu->cs = NE_SEG_TO_SEL({tseg}); seg{tseg:02d}_exec(cpu, 0x{toff:04X}u);"
                        )

        elif op in ("ljmpw", "ljmp"):
            if rest.startswith("*"):
                m_ea = parse_mem(rest[1:])
                if m_ea:
                    s, e = m_ea
                    emitted = (
                        f"    {{ uint16_t _ea = {e}; uint16_t _to = mem_r16({s}, _ea); "
                        f"uint16_t _ts = mem_r16({s}, (uint16_t)(_ea + 2)); "
                        f"recomp_dispatch_far(cpu, _ts, _to); return; }}"
                    )
            else:
                far_r = next((r for r in inst_relocs if r["source_type"] == 3), None)
                if far_r and ":" in far_r["target"]:
                    ts, to = far_r["target"].split(":", 1)
                    tseg, toff = int(ts), int(to, 16)
                    emitted = f"    cpu->cs = NE_SEG_TO_SEL({tseg}); seg{tseg:02d}_exec(cpu, 0x{toff:04X}u); return;"

        elif op == "int":
            imm = parse_imm(ops[0]) if ops else None
            if imm is not None:
                emitted = f"    win16_int(cpu, 0x{imm:02X}u);"

        # ---- Stack ----
        elif op == "pushw":
            r = op_read(ops[0], 16, reloc_imm)
            if r: emitted = f"    push16(cpu, {r});"
        elif op == "popw":
            w = op_write(ops[0], 16, "pop16(cpu)")
            if w: emitted = f"    {w}"
        elif op == "pushl":
            r = op_read(ops[0], 32, reloc_imm)
            if r: emitted = f"    push32(cpu, {r});"
        elif op == "popl":
            w = op_write(ops[0], 32, "pop32(cpu)")
            if w: emitted = f"    {w}"
        elif op == "pushfw":
            emitted = "    push16(cpu, get_flags16(cpu));"
        elif op == "popfw":
            emitted = "    set_flags16(cpu, pop16(cpu));"

        # ---- Data movement ----
        elif op in ("movb", "movw", "movl") and len(ops) == 2:
            bits = {"movb": 8, "movw": 16, "movl": 32}[op]
            r = op_read(ops[0], bits, reloc_imm)
            w = op_write(ops[1], bits, r) if r else None
            if w: emitted = f"    {w}"
        elif op == "movsbw" and len(ops) == 2:
            r = op_read(ops[0], 8)
            w = op_write(ops[1], 16, f"(uint16_t)(int16_t)(int8_t)({r})") if r else None
            if w: emitted = f"    {w}"
        elif op == "movzbw" and len(ops) == 2:
            r = op_read(ops[0], 8)
            w = op_write(ops[1], 16, f"(uint16_t)(uint8_t)({r})") if r else None
            if w: emitted = f"    {w}"
        elif op == "movswl" and len(ops) == 2:
            r = op_read(ops[0], 16)
            w = op_write(ops[1], 32, f"(uint32_t)(int32_t)(int16_t)({r})") if r else None
            if w: emitted = f"    {w}"
        elif op == "movzwl" and len(ops) == 2:
            r = op_read(ops[0], 16)
            w = op_write(ops[1], 32, f"(uint32_t)(uint16_t)({r})") if r else None
            if w: emitted = f"    {w}"
        elif op == "movsbl" and len(ops) == 2:
            r = op_read(ops[0], 8)
            w = op_write(ops[1], 32, f"(uint32_t)(int32_t)(int8_t)({r})") if r else None
            if w: emitted = f"    {w}"
        elif op == "movzbl" and len(ops) == 2:
            r = op_read(ops[0], 8)
            w = op_write(ops[1], 32, f"(uint32_t)(uint8_t)({r})") if r else None
            if w: emitted = f"    {w}"
        elif op in ("leaw", "lea") and len(ops) == 2:
            m = parse_mem(ops[0])
            w = op_write(ops[1], 16, m[1]) if m else None
            if w: emitted = f"    {w}"
        elif op in ("lesw", "ldsw") and len(ops) == 2:
            m = parse_mem(ops[0])
            dst_reg = REG16.get(ops[1])
            sreg = "cpu->es" if op == "lesw" else "cpu->ds"
            if m and dst_reg:
                emitted = (
                    f"    {{ uint16_t _ea = {m[1]}; uint16_t _o = mem_r16({m[0]}, _ea); "
                    f"uint16_t _s = mem_r16({m[0]}, (uint16_t)(_ea + 2)); {dst_reg} = _o; {sreg} = _s; }}"
                )
        elif op in ("xchgw", "xchgb") and len(ops) == 2:
            bits = 16 if op == "xchgw" else 8
            r0, r1 = op_read(ops[0], bits), op_read(ops[1], bits)
            if r0 and r1:
                w0, w1 = op_write(ops[0], bits, "_t1"), op_write(ops[1], bits, "_t0")
                if w0 and w1:
                    emitted = f"    {{ uint{bits}_t _t0 = {r0}, _t1 = {r1}; {w0} {w1} }}"
        elif op == "cbtw":
            emitted = "    AX = (uint16_t)(int16_t)(int8_t)AL;"
        elif op == "cwtd":
            emitted = "    DX = ((int16_t)AX < 0) ? 0xFFFFu : 0u;"
        elif op == "cwtl":
            # In llvm-objdump 16-bit mode: 0x98 (1 byte) = CBW (AL->AX), 0x66 0x98 (2 bytes) = CWDE (AX->EAX)
            if nbytes == 1:
                emitted = "    AX = (uint16_t)(int16_t)(int8_t)AL;"
            else:
                emitted = "    EAX = (uint32_t)(int32_t)(int16_t)AX;"
        elif op == "cltd":
            # In llvm-objdump 16-bit mode: 0x99 (1 byte) = CWD (AX->DX:AX), 0x66 0x99 (2 bytes) = CDQ (EAX->EDX:EAX)
            if nbytes == 1:
                emitted = "    DX = ((int16_t)AX < 0) ? 0xFFFFu : 0u;"
            else:
                emitted = "    EDX = ((int32_t)EAX < 0) ? 0xFFFFFFFFu : 0u;"

        # ---- ALU 2-operand ----
        elif op in ("addb", "addw", "addl", "adcb", "adcw", "adcl",
                    "subb", "subw", "subl", "sbbb", "sbbw", "sbbl",
                    "andb", "andw", "andl", "orb", "orw", "orl",
                    "xorb", "xorw", "xorl") and len(ops) == 2:
            bits = {"b": 8, "w": 16, "l": 32}[op[-1]]
            base_op = op[:-1]
            r0 = op_read(ops[0], bits, reloc_imm)
            r1 = op_read(ops[1], bits)
            if r0 and r1:
                w = op_write(ops[1], bits, f"op_{base_op}{bits}(cpu, {r1}, {r0})")
                if w: emitted = f"    {w}"

        elif op in ("cmpb", "cmpw", "cmpl") and len(ops) == 2:
            bits = {"b": 8, "w": 16, "l": 32}[op[-1]]
            r0 = op_read(ops[0], bits, reloc_imm)
            r1 = op_read(ops[1], bits)
            if r0 and r1:
                emitted = f"    (void)op_sub{bits}(cpu, {r1}, {r0});"

        elif op in ("testb", "testw", "testl") and len(ops) == 2:
            bits = {"b": 8, "w": 16, "l": 32}[op[-1]]
            r0 = op_read(ops[0], bits)
            r1 = op_read(ops[1], bits)
            if r0 and r1:
                emitted = f"    (void)op_and{bits}(cpu, {r1}, {r0});"

        # ---- ALU 1-operand ----
        elif op in ("incb", "incw", "incl", "decb", "decw", "decl", "negb", "negw", "negl") and len(ops) == 1:
            bits = {"b": 8, "w": 16, "l": 32}[op[-1]]
            base_op = op[:-1]
            r = op_read(ops[0], bits)
            if r:
                w = op_write(ops[0], bits, f"op_{base_op}{bits}(cpu, {r})")
                if w: emitted = f"    {w}"

        elif op in ("notb", "notw", "notl") and len(ops) == 1:
            bits = {"b": 8, "w": 16, "l": 32}[op[-1]]
            r = op_read(ops[0], bits)
            if r:
                w = op_write(ops[0], bits, f"(~({r}))")
                if w: emitted = f"    {w}"

        # ---- Shifts & Rotates ----
        elif op in ("shlb", "shlw", "shll", "salb", "salw", "sall",
                    "shrb", "shrw", "shrl", "sarb", "sarw", "sarl",
                    "rolw", "rorw", "rclw", "rcrw"):
            bits = {"b": 8, "w": 16, "l": 32}[op[-1]]
            base_op = "shl" if op[:-1] == "sal" else op[:-1]
            if len(ops) == 1:
                cnt_e, dst_op = "1u", ops[0]
            else:
                cnt_e, dst_op = op_read(ops[0], 8), ops[1]
            r = op_read(dst_op, bits)
            if cnt_e and r:
                w = op_write(dst_op, bits, f"op_{base_op}{bits}(cpu, {r}, (uint8_t)({cnt_e}))")
                if w: emitted = f"    {w}"

        elif op in ("shldl", "shrdl") and len(ops) == 3:
            cnt_e = op_read(ops[0], 8)
            src_e = op_read(ops[1], 32)
            dst_e = op_read(ops[2], 32)
            fn = "op_shld32" if op == "shldl" else "op_shrd32"
            if cnt_e and src_e and dst_e:
                w = op_write(ops[2], 32, f"{fn}(cpu, {dst_e}, {src_e}, (uint8_t)({cnt_e}))")
                if w: emitted = f"    {w}"

        # ---- Mul / Div ----
        elif op == "mulw" and len(ops) == 1:
            r = op_read(ops[0], 16)
            if r: emitted = f"    op_mul16(cpu, {r});"
        elif op == "imulw":
            if len(ops) == 1:
                r = op_read(ops[0], 16)
                if r: emitted = f"    op_imul16_1(cpu, (int16_t)({r}));"
            elif len(ops) == 2:
                r0, r1 = op_read(ops[0], 16), op_read(ops[1], 16)
                if r0 and r1:
                    w = op_write(ops[1], 16, f"op_imul16_2(cpu, (int16_t)({r1}), (int16_t)({r0}))")
                    if w: emitted = f"    {w}"
            elif len(ops) == 3:
                r0, r1 = op_read(ops[0], 16), op_read(ops[1], 16)
                if r0 and r1:
                    w = op_write(ops[2], 16, f"op_imul16_2(cpu, (int16_t)({r1}), (int16_t)({r0}))")
                    if w: emitted = f"    {w}"
        elif op == "imull":
            if len(ops) == 2:
                r0, r1 = op_read(ops[0], 32), op_read(ops[1], 32)
                if r0 and r1:
                    w = op_write(ops[1], 32, f"op_imul32_2(cpu, (int32_t)({r1}), (int32_t)({r0}))")
                    if w: emitted = f"    {w}"
            elif len(ops) == 3:
                r0, r1 = op_read(ops[0], 32), op_read(ops[1], 32)
                if r0 and r1:
                    w = op_write(ops[2], 32, f"op_imul32_2(cpu, (int32_t)({r1}), (int32_t)({r0}))")
                    if w: emitted = f"    {w}"
        elif op == "divw" and len(ops) == 1:
            r = op_read(ops[0], 16)
            if r: emitted = f"    op_div16(cpu, {r});"
        elif op == "idivw" and len(ops) == 1:
            r = op_read(ops[0], 16)
            if r: emitted = f"    op_idiv16(cpu, (int16_t)({r}));"
        elif op == "divl" and len(ops) == 1:
            r = op_read(ops[0], 32)
            if r: emitted = f"    op_div32(cpu, {r});"
        elif op == "idivl" and len(ops) == 1:
            r = op_read(ops[0], 32)
            if r: emitted = f"    op_idiv32(cpu, (int32_t)({r}));"

        # ---- String ops ----
        elif op == "stosb":
            fn = "op_stosb32" if is_addr32 else "op_stosb"
            emitted = f"    {fn}(cpu, {'true' if rep_mode else 'false'});"
        elif op == "stosw":
            emitted = f"    op_stosw(cpu, {'true' if rep_mode else 'false'});"
        elif op == "stosl":
            fn = "op_stosl32" if is_addr32 else "op_stosl"
            emitted = f"    {fn}(cpu, {'true' if rep_mode else 'false'});"
        elif op == "movsb":
            s = "cpu->es" if "%es:(%si)" in rest or "%es:(%esi)" in rest else ("cpu->cs" if "%cs:(%si)" in rest or "%cs:(%esi)" in rest else ("cpu->ss" if "%ss:(%si)" in rest or "%ss:(%esi)" in rest else "cpu->ds"))
            fn = "op_movsb32" if is_addr32 else "op_movsb"
            emitted = f"    {fn}(cpu, {s}, {'true' if rep_mode else 'false'});"
        elif op == "movsw":
            s = "cpu->es" if "%es:(%si)" in rest else ("cpu->cs" if "%cs:(%si)" in rest else ("cpu->ss" if "%ss:(%si)" in rest else "cpu->ds"))
            emitted = f"    op_movsw(cpu, {s}, {'true' if rep_mode else 'false'});"
        elif op == "movsl":
            s = "cpu->es" if "%es:(%si)" in rest or "%es:(%esi)" in rest else ("cpu->cs" if "%cs:(%si)" in rest or "%cs:(%esi)" in rest else ("cpu->ss" if "%ss:(%si)" in rest or "%ss:(%esi)" in rest else "cpu->ds"))
            fn = "op_movsl32" if is_addr32 else "op_movsl"
            emitted = f"    {fn}(cpu, {s}, {'true' if rep_mode else 'false'});"
        elif op == "scasb":
            emitted = f"    op_scasb(cpu, {'true' if rep_mode else 'false'}, {'true' if rep_mode in ('repne','repnz') else 'false'});"
        elif op == "scasw":
            emitted = f"    op_scasw(cpu, {'true' if rep_mode else 'false'}, {'true' if rep_mode in ('repne','repnz') else 'false'});"
        elif op == "cmpsb":
            s = "cpu->es" if "%es:(%si)" in rest else ("cpu->cs" if "%cs:(%si)" in rest else ("cpu->ss" if "%ss:(%si)" in rest else "cpu->ds"))
            emitted = f"    op_cmpsb(cpu, {s}, {'true' if rep_mode else 'false'}, {'true' if rep_mode in ('repne','repnz') else 'false'});"
        elif op == "lodsb":
            s = "cpu->es" if "%es:" in rest else ("cpu->cs" if "%cs:" in rest else ("cpu->ss" if "%ss:" in rest else "cpu->ds"))
            emitted = f"    op_lodsb(cpu, {s});"
        elif op == "lodsw":
            s = "cpu->es" if "%es:" in rest else ("cpu->cs" if "%cs:" in rest else ("cpu->ss" if "%ss:" in rest else "cpu->ds"))
            emitted = f"    op_lodsw(cpu, {s});"

        if emitted is None:
            unsupported_counter[op] += 1
            emitted = f'    /* UNSUPPORTED {seg_id}:{off:04X}: {code_part} */'
        lines.append(emitted)

    lines.append('}')
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    out_file = OUT_DIR / f"seg{seg_id:02d}.c"
    new_text = "\n".join(lines) + "\n"
    if not out_file.exists() or out_file.read_text(encoding="utf8") != new_text:
        out_file.write_text(new_text, encoding="utf8")
    return len(sorted_reach), len(entries)


def emit_dispatch_table():
    lines = [
        '#include "../../runtime/cpu.h"',
        '#include "../../runtime/win16_mem.h"',
        '#include "../../runtime/win16_api.h"',
        '#include <stdio.h>',
        '#include <stdlib.h>',
        '',
    ]
    for s in range(1, 41):
        lines.append(f'void seg{s:02d}_exec(CpuState *cpu, uint16_t entry_off);')
    lines.extend([
        '',
        'void recomp_dispatch_far(CpuState *cpu, uint16_t seg, uint16_t off) {',
        '    if (seg == 0xFF01) { (void)pop16(cpu); cpu->cs = pop16(cpu); win16_qt_dispatch(cpu, false, BX); return; }',
        '    if (seg == 0xFF02) { (void)pop16(cpu); cpu->cs = pop16(cpu); if (off == 2 || off == 3) { AX = 0; DX = 0; } else win16_qt_dispatch(cpu, true, BX); return; }',
        '    if (seg == 0xFF03) { win16_ver_dispatch(cpu, off); return; }',
        '    uint16_t seg_id = NE_SEL_TO_SEG(seg);',
        '    cpu->cs = seg;',
        '    switch (seg_id) {',
    ])
    for s in range(1, 41):
        lines.append(f'    case {s}: seg{s:02d}_exec(cpu, off); return;')
    lines.extend([
        '    default: {',
        '        static int g_disp_err_cnt = 0;',
        '        if (g_disp_err_cnt < 60) {',
        '            g_disp_err_cnt++;',
        '            uint16_t ret_ip = mem_r16(cpu->ss, SP);',
        '            uint16_t ret_cs = mem_r16(cpu->ss, (uint16_t)(SP + 2));',
        '            fprintf(stderr, "[recomp] dispatch_far invalid %04X:%04X (seg_id=%u) caller=%04X:%04X (seg%u:%04X) SS:SP=%04X:%04X\\n",',
        '                    seg, off, seg_id, ret_cs, ret_ip, NE_SEL_TO_SEG(ret_cs), ret_ip, cpu->ss, SP);',
        '        }',
        '        if (seg == 0 && off == 0) {',
        '            (void)pop16(cpu);',
        '            cpu->cs = pop16(cpu);',
        '            AX = 0; DX = 0;',
        '            return;',
        '        }',
        '        fflush(stderr); exit(3);',
        '    }',
        '    }',
        '}',
        '',
        'void recomp_dispatch_near(CpuState *cpu, uint16_t seg, uint16_t off) {',
        '    uint16_t seg_id = NE_SEL_TO_SEG(seg);',
        '    switch (seg_id) {',
    ])
    for s in range(1, 41):
        lines.append(f'    case {s}: seg{s:02d}_exec(cpu, off); return;')
    lines.extend([
        '    default:',
        '        fprintf(stderr, "[recomp] dispatch_near invalid %04X:%04X\\n", seg, off);',
        '        fflush(stderr); exit(3);',
        '    }',
        '}',
    ])
    dt_file = OUT_DIR / "dispatch_table.c"
    new_dt = "\n".join(lines) + "\n"
    if not dt_file.exists() or dt_file.read_text(encoding="utf8") != new_dt:
        dt_file.write_text(new_dt, encoding="utf8")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--seg", type=int, default=0, help="Lift only one segment (0 = all 40)")
    args = ap.parse_args()

    cfg = json.loads((ANALYSIS / "recomp-cfg.json").read_text(encoding="utf8"))
    refs = json.loads((DISASM / "references.json").read_text(encoding="utf8"))
    inv = json.loads((ANALYSIS / "inventory.json").read_text(encoding="utf8"))
    ne_entries = inv["NE"]["BIGJOBNG.EXE"].get("entries", [])

    refs_by_seg = defaultdict(list)
    for r in refs:
        refs_by_seg[r["segment"]].append(r)

    unsupported = Counter()
    segs_to_lift = [args.seg] if args.seg else list(range(1, 41))
    total_insts = 0
    total_entries = 0
    for s in segs_to_lift:
        ni, ne = lift_segment(s, cfg, refs_by_seg, ne_entries, unsupported)
        total_insts += ni
        total_entries += ne

    emit_dispatch_table()
    total_unsup = sum(unsupported.values())
    pct = 100.0 * (total_insts - total_unsup) / max(1, total_insts)
    print(f"Lifted {len(segs_to_lift)} segments: {total_insts} reachable instructions, {total_entries} entry points")
    print(f"Supported: {total_insts - total_unsup}/{total_insts} ({pct:.2f}%), unsupported: {total_unsup}")
    if unsupported:
        print("Top unsupported:", unsupported.most_common(25))


if __name__ == "__main__":
    main()
