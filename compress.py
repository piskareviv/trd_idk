import os, shutil, re

def compress(s, limit=96):
    s += '\n'
    s = re.sub(r"//([^\n]*)\n", r" /* \g<1> */ ", s)
    s = s.replace('\n', ' ')
    s = re.sub(r'\s+', ' ', s)
    t, u = "", ""
    for word in s.split(' '):
        if len(u) + len(word) > limit:
            t += u + '\n'
            u = ""
        u += word + ' '
    t += u + '\n'
    return t

os.makedirs("src_compressed", exist_ok=True)

for d in os.listdir("src"):
    if d.endswith(".cpp") or d.endswith(".hpp") or d.endswith(".h"):
        s = open(f"src/{d}").read()
        t = compress(s)
        open(f"src_compressed/{d}", "w").write(t)
        print(f"compressed {d} {s.count('\n')} lines -> {t.count('\n')} lines")
