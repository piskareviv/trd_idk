import os
import subprocess


def cached_compile(path):
    if not os.path.exists(path) or os.path.getmtime(path) < os.path.getmtime(f"{path}.cpp"):
        print(f"compiling {path}...", end=" ", flush=True)
        ret = subprocess.run(f"g++ {path}.cpp -o {path} -std=c++20 -O2", shell=True).returncode
        if ret != 0:
            print("fuck")
            exit(1)
        print("compiled", flush=True)
