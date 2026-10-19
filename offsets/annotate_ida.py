"""Run with File > Script file in the IDB for the documented original EXE.
Imports fixed-width types, adds ac_ names and comments. Does not patch bytes.
This script has syntax validation; execution inside IDA has not been tested.
"""
import hashlib
import json
from pathlib import Path
import ida_bytes
import ida_name
import ida_nalt
import idc

folder = Path(__file__).resolve().parent
data = json.loads((folder / 'offsets.json').read_text(encoding='utf-8'))
exe = Path(ida_nalt.get_input_file_path())
if not exe.is_file():
    raise RuntimeError('Input executable unavailable: cannot verify its hash.')
if hashlib.sha256(exe.read_bytes()).hexdigest() != data['build']['sha256']:
    raise RuntimeError('Executable SHA256 differs: this map belongs to another build.')
base = ida_nalt.get_imagebase()
if ida_bytes.get_bytes(base + 0x94F2A, 7) != bytes.fromhex('89 84 8E 44 04 00 00'):
    raise RuntimeError('Database code bytes differ from the verified build.')
errors = idc.parse_decls(str(folder / 'ida_types.h'), idc.PT_FILE)
if errors:
    raise RuntimeError('Type import errors: %s' % errors)

def annotate(rva, name, text):
    ea = base + rva
    old = ida_name.get_name(ea)
    if not old or old.startswith(('sub_', 'dword_', 'word_', 'byte_', 'qword_', 'flt_', 'unk_', 'off_', 'ac_')):
        ida_name.set_name(ea, 'ac_' + name, ida_name.SN_CHECK)
    previous = idc.get_cmt(ea, 0) or ''
    comment = 'AC1302: ' + text
    if comment not in previous:
        idc.set_cmt(ea, previous + ('\n' if previous else '') + comment, 0)

for name, item in data['globals'].items():
    annotate(item['rva'], name, item['type'] + '; ' + item['evidence'])
for rva, name, text in [
    (0x7EF70, 'newplayerent', 'Allocates 0x408 bytes; constructs client playerent.'),
    (0x1C520, 'playerent_ctor', 'ECX = playerent*; two vtables at +0 and +0xE8.'),
    (0x570D0, 'readmatrices', 'Reads OpenGL modelview/projection; computes MVP and inverses.'),
    (0x948A0, 'server_processevents', 'Iterates server clients; ESI here is client*, not playerent*.'),
    (0x94EA0, 'server_process_shot', 'ECX = server client*, EDX = shotevent*.'),
    (0x94F2A, 'server_decrement_mag', 'ESI = server client*; ECX = weapon ID; writes mag[ECX] at +0x444.'),
]:
    annotate(rva, name, text)
idc.SetType(base + data['globals']['player1']['rva'], 'ACPlayerEnt32 *ac_player1;')
print('AC1302: imported types and annotated %d globals.' % len(data['globals']))
