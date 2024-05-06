import sys, re, os
assert len(sys.argv) >= 2
try:
    os.chdir(os.path.dirname(__file__))
    files = ['gsh/' + f for f in os.listdir('gsh') if os.path.isfile(os.path.join('gsh', f))]
    lib = [re.sub(r'^#pragma once\n', '', open(f, 'r', encoding='utf-8').read()) for f in files]

    allpat = '('
    for f in files: allpat += rf'#include\s*?(<{f}>|"{f}")|'
    allpat = re.compile(allpat[:-1] + ')')
    comment1 = re.compile(r'//.*?\n')
    comment2 = re.compile(r'/\*[\s\S]*?\*/')

    fp = open(sys.argv[1], 'r', encoding="utf-8")
    code = fp.read()
    fp.close()

    while True:
        code = re.sub(comment1, '\n', code)
        code = re.sub(comment2, '', code)
        m = re.search(allpat, code)
        if m == None: break
        for i in range(len(files)):
            if files[i] in m[0]:
                code = code[:m.start(0)] + lib[i] + code[m.end(0):]
                lib[i] = ''
                break

    print(code)

except OSError as e:
    print(e)