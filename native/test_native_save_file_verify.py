"""Independent disk fixtures for the native game-written save verifier.

Does not touch the game's saves directory or start the runtime.
"""
from pathlib import Path
import struct
import subprocess
import tempfile

native = Path(__file__).resolve().parent
verifier = native / 'build' / 'native-save-file-verify.exe'

def fixture(flag=255, *, schema=0x2432da, extent=13, duplicate=False):
    chunk = struct.pack('>III', 0x434f4e46, schema, extent) + bytes([flag])
    payload = bytes([1, 2 if duplicate else 1, 0, 0]) + chunk * (2 if duplicate else 1)
    size = 32 + len(payload)
    disk = bytearray(struct.pack('>IIII', 0, 2, 1, size))
    disk += b'config1\0'.ljust(12, b'\0') + struct.pack('>I', 32) + payload
    words = [int.from_bytes(disk[i:i+2], 'big') for i in range(4, size - 1, 2)]
    checksum = ((sum(words) & 0xffff) << 16) | (sum(x ^ 0xffff for x in words) & 0xffff)
    disk[0:4] = struct.pack('>I', checksum)
    disk.extend(bytes((-len(disk)) % 32))
    return disk

with tempfile.TemporaryDirectory(prefix='save-verifier-fixture-', dir=native / 'build') as tmp:
    path = Path(tmp) / 'fixture.bin'
    def check(label, disk, success, needle, args=()):
        path.write_bytes(disk)
        result = subprocess.run([str(verifier), str(path), *args], capture_output=True, text=True)
        output = result.stdout + result.stderr
        assert (result.returncode == 0) == success, (label, result.returncode, output)
        assert needle in output, (label, output)
        print('PASS ' + label)
    check('created profile accepted', fixture(), True, 'created=1', ('--require-created',))
    check('empty profile readable', fixture(0), True, 'created=0')
    check('empty profile cannot count as created', fixture(0), False, 'Expected at least one', ('--require-created',))
    check('unknown option rejected', fixture(), False, 'Expected at least one', ('--unknown',))
    check('wrong CONF schema rejected', fixture(schema=0), False, 'Invalid CONF created-flag schema')
    check('out-of-bounds chunk rejected', fixture(extent=14), False, 'Invalid profile config chunk extent')
    check('invalid flag rejected', fixture(7), False, 'Invalid CONF created flag')
    check('duplicate CONF rejected', fixture(duplicate=True), False, 'Invalid CONF created-flag schema')
    corrupt = fixture()
    corrupt[0] ^= 1
    check('checksum corruption rejected', corrupt, False, 'checksum, bounds or layout')
print('PASS save-file verifier: 9 independent disk fixtures; no real game save modified')
