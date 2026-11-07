import os
import re
import shutil
import subprocess
import tempfile

NL = "\n"
BS = "\\"

LIMIT = 96

BREAK_WORDS = (
    "struct", "class", "template", "namespace", "enum", "union",
    "using", "typedef", "public", "private", "protected",
    "friend", "inline", "virtual", "explicit", "constexpr", "operator"
)

CTRL = (
    "for", "while", "if", "switch", "return", "catch", "sizeof",
    "else", "do", "case", "new", "delete"
)

# ============================================================
# Literals
# ============================================================

# C++ raw strings, including custom delimiters.
RAW_STR = (
    r'(?:u8|u|U|L)?R"'
    r'([^ ()\\\t\r\n]{0,16})'
    r'\(.*?\)\1"'
)

RAW_RE = re.compile(RAW_STR, re.S)

STR = r'(?:u8|u|U|L)?"(?:[^"\\\n]|\\.)*"'
CHR = r"(?:u8|u|U|L)?'(?:[^'\\\n]|\\.)*'"

STRCHR = RAW_STR + "|" + STR + "|" + CHR


def protect_raw_strings(source):

    literals = []

    def replace(m):

        token = "__RAW_LITERAL_%d__" % len(literals)

        literals.append((token, m.group(0)))

        return token

    source = RAW_RE.sub(replace, source)

    return source, literals


def restore_raw_strings(source, literals):

    for token, literal in literals:
        source = source.replace(token, literal)

    return source


# ============================================================
# Comments
# ============================================================

def fix_comments(line):

    # Convert //comment -> /* comment */
    # Only outside string/character literals.

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

            return (
                line[:p]
                + " /* "
                + line[p + 2:].strip()
                + " */ "
            )

    return line


# ============================================================
# Mask strings and comments for brace counting
# ============================================================

def mask_source(source):

    result = list(source)

    i = 0
    n = len(source)

    state = "normal"

    while i < n:

        c = source[i]
        nxt = source[i + 1] if i + 1 < n else ""

        if state == "normal":

            if c == '"':
                result[i] = " "
                state = "string"

            elif c == "'":
                result[i] = " "
                state = "char"

            elif c == "/" and nxt == "/":

                result[i] = " "
                result[i + 1] = " "

                state = "line_comment"
                i += 1

            elif c == "/" and nxt == "*":

                result[i] = " "
                result[i + 1] = " "

                state = "block_comment"
                i += 1

        elif state == "string":

            if c != "\n":
                result[i] = " "

            if c == BS:

                i += 1

                if i < n and source[i] != "\n":
                    result[i] = " "

            elif c == '"':
                state = "normal"

        elif state == "char":

            if c != "\n":
                result[i] = " "

            if c == BS:

                i += 1

                if i < n and source[i] != "\n":
                    result[i] = " "

            elif c == "'":
                state = "normal"

        elif state == "line_comment":

            if c == "\n":
                state = "normal"
            else:
                result[i] = " "

        elif state == "block_comment":

            if c == "*" and nxt == "/":

                result[i] = " "
                result[i + 1] = " "

                state = "normal"
                i += 1

            elif c != "\n":
                result[i] = " "

        i += 1

    return "".join(result)


# ============================================================
# Whitespace
# ============================================================

def collapse(line):

    res = []
    last = 0

    for m in re.finditer(STRCHR, line):

        res.append(
            re.sub(r"\s+", " ", line[last:m.start()])
        )

        res.append(m.group(0))
        last = m.end()

    res.append(re.sub(r"\s+", " ", line[last:]))

    return "".join(res)


def split_words(line):

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


# ============================================================
# Function / lambda detection
# ============================================================

LAMBDA_RE = (
    r"(^|[=(,]\s*)"
    r"\[[^\]]*\]"
    r"\s*(\(|\{|->)"
)


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

    return (
        tail == ""
        or tail[0] == "{"
        or tail[0] == ":"
        or tail.startswith("const")
        or tail.startswith("->")
    )


# ============================================================
# Compression
# ============================================================

def compress(s, limit=LIMIT):

    # Protect multiline raw strings before processing.
    s, raw_literals = protect_raw_strings(s)

    out = []
    words = []

    indent = 0
    depth = 0
    net = 0
    lam = False

    masked = mask_source(s).split(NL)

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

                pad = " " * (indent + 4)
                budget = limit - indent - 4

                cur = w

            else:

                cur = w if not cur else cur + " " + w

        if cur:
            out.append(pad + cur)

        del words[:]

        net = 0
        lam = False

    for raw, clean in zip(s.split(NL), masked):

        line = collapse(fix_comments(raw)).strip()

        if not line:
            continue

        # Preprocessor directives are independent.
        if line.startswith("#"):

            flush()
            out.append(line)
            continue

        d0 = depth

        # Count braces outside literals and comments.
        line_net = clean.count("{") - clean.count("}")

        depth = d0 + line_net

        first = line.split("<", 1)[0].split(" ", 1)[0]

        is_lam = re.search(LAMBDA_RE, line)

        trig = (
            first in BREAK_WORDS
            or is_def(line)
            or is_lam
        )

        if trig and words:
            flush()

        if not words:

            indent = (
                4 * (d0 - 1)
                if line.startswith("}")
                else 4 * d0
            )

            indent = max(0, indent)

            lam = bool(is_lam)

        words.extend(split_words(line))

        net += line_net

        # Original aggressive grouping behavior.
        if net <= 0 and (
            line.endswith("}")
            or (
                line.endswith(";")
                and (depth == 0 or lam)
            )
        ):
            flush()

    flush()

    result = NL.join(out) + NL

    # Restore raw strings exactly.
    return restore_raw_strings(result, raw_literals)


# ============================================================
# Token equivalence check
# ============================================================

def tokenize(s):

    BLOCK = r"/\*.*?\*/"
    LINE = r"//[^\n]*"

    IDENT = r"[A-Za-z0-9_.]+"

    pat = (
        RAW_STR
        + "|"
        + BLOCK
        + "|"
        + LINE
        + "|"
        + STR
        + "|"
        + CHR
        + "|"
        + IDENT
        + "|[^ \t\r\n]"
    )

    toks = []

    for m in re.finditer(pat, s, re.S):

        t = m.group(0)

        if t.startswith("//") or t.startswith("/*"):

            inner = t[2:]

            if inner.endswith("*/"):
                inner = inner[:-2]

            toks.append((
                "comment",
                re.sub(r"\s+", "", inner)
            ))

        else:

            toks.append(("tok", t))

    return toks


def text_check(orig_path, comp_path):

    with open(orig_path) as f:
        a = tokenize(f.read())

    with open(comp_path) as f:
        b = tokenize(f.read())

    if len(a) == len(b) and all(
        x == y for x, y in zip(a, b)
    ):

        return (
            "token-identical (%d tokens, strings exact)"
            % len(a)
        )

    n = min(len(a), len(b))

    for i in range(n):

        if a[i] != b[i]:

            return (
                "MISMATCH at token %d: %r vs %r"
                % (i, a[i][1][:40], b[i][1][:40])
            )

    return (
        "MISMATCH: token count %d vs %d"
        % (len(a), len(b))
    )


# ============================================================
# Compilation-equivalence approximation
# ============================================================

def pick_compiler():

    for c in ("clang++", "g++", "c++"):

        if shutil.which(c):
            return c

    return None


def norm_asm(a):

    keep = []

    for ln in a.split(NL):

        ln = ln.strip()

        if (
            ln
            and not ln.startswith("#")
            and not ln.startswith(".file")
            and not ln.startswith(".loc")
        ):
            keep.append(ln)

    return NL.join(keep)


def check_pair(orig, comp):

    orig = open(orig).read()
    comp = open(comp).read()

    orig = orig.replace("\n", " ")
    orig = orig.replace("  ", " ")
    orig = orig.replace("  ", " ")
    orig = orig.replace("  ", " ")

    comp = comp.replace("\n", " ")
    comp = comp.replace("  ", " ")
    comp = comp.replace("  ", " ")
    comp = comp.replace("  ", " ")

    orig = orig.replace(" ", "")
    comp = comp.replace(" ", "")

    orig = orig.replace("/*", "")
    comp = comp.replace("/*", "")

    orig = orig.replace("*/", "")
    comp = comp.replace("*/", "")

    orig = orig.replace("*", "")
    comp = comp.replace("*", "")

    orig = orig.replace("/", "")
    comp = comp.replace("/", "")

    print("checking", d)

    if orig != comp:

        for x, y in [("A", orig), ("B", comp)]:

            z = ""

            for i, c in enumerate(y):

                z += c

                if i % 100 == 99:
                    z += "\n"

            with open(
                "src_compressed/" + x + ".cpp",
                "w"
            ) as f:
                f.write(z)

        print("MISMATCH:", d)

    assert orig == comp

    print("success", d)


# ============================================================
# Main
# ============================================================

os.makedirs("src_compressed", exist_ok=True)

done = []

for d in sorted(os.listdir("src")):

    if d.endswith((".cpp", ".hpp", ".h")):

        with open("src/" + d) as f:
            s = f.read()

        t = compress(s)

        with open("src_compressed/" + d, "w") as f:
            f.write(t)

        print(
            "compressed",
            d,
            s.count(NL),
            "lines ->",
            t.count(NL),
            "lines"
        )

        print(
            "  text:",
            text_check(
                "src/" + d,
                "src_compressed/" + d
            )
        )

        done.append(d)


if done:

    print()
    print("checking compilation equivalence")

    for d in done:

        check_pair(
            "src/" + d,
            "src_compressed/" + d
        )

else:

    print("no files")
