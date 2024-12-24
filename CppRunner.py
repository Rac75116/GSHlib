import sys
import subprocess
import os
import time

compiler = "g++"
# compiler = "clang++"
gcc_option = [
    "-g",
    "-std=gnu++20",
    "-O2",
    "-Wall",
    "-Wextra",
    "-march=native",
    "-mtune=native",
    "-fconstexpr-depth=2147483647",
    "-fconstexpr-loop-limit=2147483647",
    "-fconstexpr-ops-limit=2147483647",
    "-I.",
]
clang_option = [
    "-g",
    "-std=c++20",
    "-O2",
    "-Wall",
    "-Wextra",
    "-march=native",
    "-mtune=native",
    "-fconstexpr-depth=2147483647",
    "-fconstexpr-steps=2147483647",
    "-fuse-ld=lld",
    "-I.",
]
includes = [r"C:\Users\tsout\Desktop\Projects\GSHlib\Test\lib\oldlib\oldlib.hpp"]
try:
    path = sys.argv[1]
    base, ext = os.path.splitext(path)
    if ext != ".cpp":
        gchpath = path + ".gch"
        option = [compiler]
        if compiler == "g++":
            option += gcc_option
        elif compiler == "clang++":
            option += clang_option
        else:
            assert False
        option += ["-x", "c++-header"]
        option += ["-o", gchpath, path]
        st = time.perf_counter()
        cp = subprocess.run(option)
        ed = time.perf_counter()
        if cp.returncode != 0:
            print("\033[31mCompilation failed\033[0m")
            sys.exit(1)
        print(
            "\033[32mCompilation succeeded  ["
            + str(max(0, round((ed - st) * 1000) - 10))
            + "ms]\033[0m"
        )
        sys.exit(0)
    exepath = base + ".exe"
    option = [compiler]
    if compiler == "g++":
        option += gcc_option
    elif compiler == "clang++":
        option += clang_option
    else:
        assert False
    for p in includes:
        option += ["-include", p]
    option += ["-o", exepath, path]
    st = time.perf_counter()
    cp = subprocess.run(option)
    ed = time.perf_counter()
    if cp.returncode != 0:
        print("\033[31mCompilation failed\033[0m")
        sys.exit(1)
    print(
        "\033[32mCompilation succeeded  ["
        + str(max(0, round((ed - st) * 1000) - 10))
        + "ms]\033[0m"
    )
    try:
        subprocess.run(exepath, timeout=0)
    except subprocess.TimeoutExpired:
        pass
    for i in range(10):
        st = time.perf_counter()
        cp = subprocess.run(exepath)
        ed = time.perf_counter()
        if cp.returncode != 0:
            print(f"\033[31m#{i+1} Execusion failed\033[0m")
            sys.exit(2)
        print(
            f"\033[32m#{i+1} Execusion succeeded  ["
            + str(max(0, round((ed - st) * 1000) - 10))
            + "ms]\033[0m"
        )
except KeyboardInterrupt:
    pass
