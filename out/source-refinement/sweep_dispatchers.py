import json, re, subprocess
from pathlib import Path
root = Path('/mnt/PROJECTS/repos/rebof3-simple')
scan = json.load(open(root/'out/source-refinement/dispatcher-scan.json'))
grouped = {}
for tf in (root/'config/targets/emi').rglob('target.toml'):
    t = str(tf.parent.relative_to(root/'config/targets'))
    if re.search(r'(DispatchTables|DispatchSlotOne)\.c', tf.read_text()) and t in scan:
        grouped[t] = 1
out = root/'out/source-refinement/dispatcher-sweep.json'
res = {'ok': [], 'fail': []}
for t in sorted(grouped):
    load = int(next(l.split('=')[1].strip() for l in (root/f'config/targets/{t}/target.toml').read_text().splitlines() if l.startswith('load_address')), 0)
    b = subprocess.run(['.venv/bin/python','-m','harness.cli','build',t], cwd=root, capture_output=True, text=True)
    if b.returncode:
        res['fail'].append([t,'build']); out.write_text(json.dumps(res)); continue
    for off, _ in scan[t]:
        va = load + off
        r = subprocess.run(['.venv/bin/python','-m','harness.cli','lift','gate','--target',t,f'{t}@0x{va:08X}'], cwd=root, capture_output=True, text=True)
        (res['ok'] if f'@0x{va:08X}: PASS' in r.stdout else res['fail']).append([t, hex(va)])
    out.write_text(json.dumps(res))
print('done', len(res['ok']), 'fail', len(res['fail']))
