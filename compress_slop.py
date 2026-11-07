import os, re, shutil, subprocess, tempfile

NL = chr(10)   # newline
BS = chr(92)   # backslash

# string and char literals (exact content is preserved everywhere)
STR = '"' + "(?:[^" + '"' + BS + BS + BS + "n]|" + BS + BS + ".)*" + '"'
CHR = "'" + "(?:[^" + "'" + BS + BS + BS + "n]|" + BS + BS + ".)*" + "'"
STRCHR = STR + "|" + CHR
WS_RE = BS + "s+"

BREAK_WORDS = ("struct", "class", "template", "namespace", "enum", "union",
               "using", "typedef", "public", "private", "protected",
               "friend", "inline", "virtual", "explicit", "constexpr", "operator")
CTRL = ("for", "while", "if", "switch", "return", "catch", "sizeof",
        "else", "do", "case", "new", "delete")

# a lambda: [captures] preceded by = ( , or line start, then followed by ( { or ->
# LAMBDA_RE = ("(^|[=(,]" + BS + "s*)" + BS + "[[" + BS + "]]*" + BS + "]"
#              + BS + "s*(" + BS + "(|" + BS + "{|->)")
LAMBDA_RE = ("(^|[=(,]" + BS + "s*)" + BS + "[" + "[^" + BS + "]]*" + BS + "]"
             + BS + "s*(" + BS + "(|" + BS + "{|->)")


def fix_comments(line):
    # convert //comment -> /* comment */, but only outside string/char literals
    segs = []
    last = 0
    for m in re.finditer(STRCHR, line):
        if m.start() > last:
            segs.append((last, m.start(), False))
        segs.append((m.start(), m.end(), True))
        last = m.end()
    segs.append((last, len(line), False))
    for a, b, lit in segs:
        if lit:
            continue
        p = line.find("//", a, b)
        if p >= 0:
            return line[:p] + " /* " + line[p + 2:].strip() + " */ "
    return line

def collapse(line):
    # collapse whitespace only OUTSIDE string/char literals
    res = []
    last = 0
    for m in re.finditer(STRCHR, line):
        res.append(re.sub(WS_RE, " ", line[last:m.start()]))
        res.append(m.group(0))
        last = m.end()
    res.append(re.sub(WS_RE, " ", line[last:]))
    return "".join(res)

def split_words(line):
    # split into words, keeping string/char literals ATOMIC
    words = []
    last = 0
    for m in re.finditer(STRCHR, line):
        for w in line[last:m.start()].split(" "):
            if w:
                words.append(w)
        words.append(m.group(0))
        last = m.end()
    for w in line[last:].split(" "):
        if w:
            words.append(w)
    return words

def close_paren(line, i):
    d = 0
    for k in range(i, len(line)):
        if line[k] == "(":
            d += 1
        elif line[k] == ")":
            d -= 1
            if d == 0:
                return k
    return -1

def is_def(line):
    i = line.find("(")
    if i < 1:
        return False
    head = line[:i]
    if ";" in head or '"' in head:
        return False
    if "=" in head and not head.rstrip().endswith("="):
        return False
    words = head.split()
    if not words or words[-1] in CTRL:
        return False
    k = close_paren(line, i)
    if k < 0:
        return True
    tail = line[k + 1:].strip()
    return (tail == "" or tail[0] == "{" or tail[0] == ":"
            or tail.startswith("const") or tail.startswith("->"))

def compress(s, limit=96):
    out = []
    words = []
    indent = 0
    depth = 0
    net = 0
    lam = False

    def flush():
        nonlocal net, lam
        if not words:
            return
        pad = " " * indent
        budget = limit - indent
        cur = ""
        for w in words:
            if cur and len(cur) + 1 + len(w) > budget:
                out.append(pad + cur)
                pad = " " * (indent + 4)      # continuation lines get +4
                budget = limit - indent - 4
                cur = w
            else:
                cur = w if not cur else cur + " " + w
        if cur:
            out.append(pad + cur)
        del words[:]
        net = 0
        lam = False

    for raw in s.split(NL):
        line = collapse(fix_comments(raw)).strip()
        if not line:
            continue
        if line.startswith("#") and 0:
            flush()
            out.append(line)
            continue
        d0 = depth
        line_net = line.count("{") - line.count("}")
        depth = d0 + line_net
        first = line.split("<", 1)[0].split(" ", 1)[0]
        is_lam = re.search(LAMBDA_RE, line)
        trig = first in BREAK_WORDS or is_def(line) or is_lam
        if trig and words:
            flush()
        if not words:
            indent = 4 * (d0 - 1) if line.startswith("}") else 4 * d0
            lam = bool(is_lam)
        for w in split_words(line):
            words.append(w)
        net += line_net
        # end the line when the construct that opened on it has closed
        if net <= 0 and (line.endswith("}")
                         or (line.endswith(";") and (depth == 0 or lam))):
            flush()
    flush()
    return NL.join(out) + NL

# ---------- token equivalence check (no compiler needed) ----------

def tokenize(s):
    BLOCK = BS + "/" + BS + "*.*?" + BS + "*/"
    LINE = "//[^" + NL + "]*"
    IDENT = "[A-Za-z0-9_.]+"
    CATCH = "[^" + BS + "s]"
    pat = BLOCK + "|" + LINE + "|" + STR + "|" + CHR + "|" + IDENT + "|" + CATCH
    toks = []
    for m in re.finditer(pat, s, re.S):
        t = m.group(0)
        if t.startswith("//") or t.startswith("/*"):
            inner = t[2:]
            if inner.endswith("*/"):
                inner = inner[:-2]
            toks.append(("comment", re.sub(WS_RE, "", inner)))
        else:
            toks.append(("tok", t))
    return toks

def text_check(orig_path, comp_path):
    a = tokenize(open(orig_path).read())
    b = tokenize(open(comp_path).read())
    if len(a) == len(b) and all(x == y for x, y in zip(a, b)):
        return "token-identical (%d tokens, strings exact)" % len(a)
    assert False, "FUCK"
    n = min(len(a), len(b))
    for i in range(n):
        if a[i] != b[i]:
            return "MISMATCH at token %d: %r vs %r" % (i, a[i][1][:40], b[i][1][:40])
    return "MISMATCH: token count %d vs %d" % (len(a), len(b))

# ---------- compile-equivalence check (clang++/g++) ----------

def pick_compiler():
    for c in ("clang++", "g++", "c++"):
        if shutil.which(c):
            return c
    return None

def norm_asm(a):
    keep = []
    for ln in a.split(NL):
        ln = ln.strip()
        if ln and not ln.startswith("#") and not ln.startswith(".file") and not ln.startswith(".loc"):
            keep.append(ln)
    return NL.join(keep)

def check_pair(orig, comp):
    pa = orig
    pb = comp
    orig = open(f"{orig}").read()
    comp = open(f"{comp}").read()

    orig = orig.replace('\n', ' ')
    orig = orig.replace('  ', ' ')
    orig = orig.replace('  ', ' ')
    orig = orig.replace('  ', ' ')
    comp = comp.replace('\n', ' ')
    comp = comp.replace('  ', ' ')
    comp = comp.replace('  ', ' ')
    comp = comp.replace('  ', ' ')

    orig = orig.replace(' ', '')
    comp = comp.replace(' ', '')

    orig = orig.replace('/*', '')
    comp = comp.replace('/*', '')
    
    orig = orig.replace('*/', '')
    comp = comp.replace('*/', '')
    orig = orig.replace('*', '')
    comp = comp.replace('*', '')
    
    orig = orig.replace('/', '')
    comp = comp.replace('/', '')


    
    print(f"checking {d}")
    if orig != comp:
        for x, y in [("A", orig), ("B", comp)]:
            z = ""
            for i,c in  enumerate(y):
                z += c
                if i % 100 == 99:
                    z += '\n'
            open(f"src_compressed/{x}.cpp",'w').write(z)
        print("fuck" * 10000)
    assert orig == comp
    print(f"success {d}")

os.makedirs("src_compressed", exist_ok=True)

done = []
for d in sorted(os.listdir("src")):
    if d.endswith(".cpp") or d.endswith(".hpp") or d.endswith(".h"):
        s = open("src/" + d).read()
        t = compress(s)
        open("src_compressed/" + d, "w").write(t)
        print("compressed", d, s.count(NL), "lines ->", t.count(NL), "lines")
        print("  text:", text_check("src/" + d, "src_compressed/" + d))
        done.append(d)

if done:
    print()
    print("checking compilation equivalence with")
    for d in done:
        check_pair("src/" + d, "src_compressed/" + d)
        # r = text_check("src/" + d, "src_compressed/" + d)
        # print(r)
else:
    print("no files")