import sys, subprocess, os, time
compiler = "g++"
#compiler = "clang++"
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
    "-I."
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
    "-I."
]
path = sys.argv[1]
exepath = os.path.splitext(path)[0] + ".exe"
option = [compiler]
if compiler == "g++": option += gcc_option
elif compiler == "clang++": option += clang_option
else: assert False
option += ["-o", exepath, path]
st = time.perf_counter()
cp = subprocess.run(option)
ed = time.perf_counter()
if cp.returncode != 0:
    print("\033[31mCompilation failed\033[0m")
    sys.exit(1)
print("\033[32mCompilation succeeded  [" + str(max(0, round((ed - st) * 1000) - 10)) + "ms]\033[0m")
try: subprocess.run(exepath, timeout=0)
except: pass
st = time.perf_counter()
cp = subprocess.run(exepath, universal_newlines=True)
ed = time.perf_counter()
if cp.returncode != 0:
    print("\033[31mExecusion failed\033[0m")
    sys.exit(2)
print("\033[32mExecusion succeeded  [" + str(max(0, round((ed - st) * 1000) - 10)) + "ms]\033[0m")
