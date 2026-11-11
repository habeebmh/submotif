#!/usr/bin/env python3
"""Release provenance, integrity and portable ZIP preservation checks."""
import importlib.util
import json
import os
from pathlib import Path
import stat
import struct
import tempfile
import unittest
import zipfile

SCRIPT = Path(__file__).resolve().parents[1] / 'scripts/package-release.py'
spec = importlib.util.spec_from_file_location('packager', SCRIPT)
package = importlib.util.module_from_spec(spec)
spec.loader.exec_module(package)


class ReleaseTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.stage = Path(self.temp.name) / 'release'
        self.stage.mkdir()
        paths = ['LICENSE', 'JUCE-LICENSE.md', 'THIRD_PARTY_NOTICES.md',
                 'source/.deps/JUCE/CMakeLists.txt', 'source/plugin/CMakeLists.txt',
                 'max/Submotif.amxd']
        for name in paths:
            p = self.stage / name
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_text('fixture\n')
        pe = bytearray(70)
        pe[:2] = b'MZ'
        struct.pack_into('<I', pe, 0x3c, 64)
        pe[64:68] = b'PE\0\0'
        struct.pack_into('<H', pe, 68, 0x8664)
        binaries = {}
        for name, kind in [('Submotif.vst3/Contents/x86_64-win/Submotif.vst3', 'VST3'), ('Submotif.exe', 'Standalone')]:
            p = self.stage / name
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_bytes(pe)
            binaries['Undertow_artefacts/Release/' + kind + '/' + name] = package.build.sha(p)
        self.fingerprint = package.build.source_fingerprint(self.stage / 'source/plugin')
        self.manifest = {'target': 'windows-x64', 'sourceCommit': 'a' * 40, 'workingTreeDirty': False,
                         'tests': 'passed', 'coreOnly': False, 'sourceFiles': self.fingerprint,
                         'architectures': ['x64'], 'binaries': binaries}
        self.seal()

    def seal(self):
        (self.stage / 'BUILD.json').write_text(json.dumps(self.manifest))
        (self.stage / 'SHA256SUMS.txt').write_text(''.join(
            f'{package.build.sha(p)}  {p.relative_to(self.stage).as_posix()}\n'
            for p in sorted(self.stage.rglob('*')) if p.is_file() and p.name != 'SHA256SUMS.txt'))

    def verify(self):
        return package.verify_stage(self.stage, 'windows-x64', 'a' * 40, self.fingerprint)

    def test_complete_build_passes(self):
        self.assertEqual(self.verify()['sourceCommit'], 'a' * 40)

    def test_ineligible_builds_rejected(self):
        for key, value in [('sourceCommit', 'b' * 40), ('workingTreeDirty', True),
                           ('tests', 'skipped'), ('coreOnly', True), ('target', 'macos-arm64')]:
            with self.subTest(key=key):
                original = self.manifest[key]
                self.manifest[key] = value
                self.seal()
                with self.assertRaises(ValueError): self.verify()
                self.manifest[key] = original

    def test_tampered_binary_rejected(self):
        (self.stage / 'Submotif.exe').write_bytes(b'altered')
        with self.assertRaisesRegex(ValueError, 'checksum'): self.verify()

    def test_unlisted_file_rejected(self):
        (self.stage / 'unexpected').write_text('extra')
        with self.assertRaisesRegex(ValueError, 'unchecksummed'): self.verify()

    def test_source_mismatch_rejected(self):
        (self.stage / 'source/plugin/CMakeLists.txt').write_text('altered')
        self.seal()
        with self.assertRaisesRegex(ValueError, 'fingerprint'): self.verify()

    def test_missing_binary_metadata_rejected(self):
        self.manifest['binaries'] = {}
        self.seal()
        with self.assertRaisesRegex(ValueError, 'both expected'): self.verify()

    def test_checksum_cannot_escape(self):
        (self.stage / 'SHA256SUMS.txt').write_text('0' * 64 + '  ../outside\n')
        with self.assertRaisesRegex(ValueError, 'checksum path'): self.verify()

    def test_zip_content_and_modes(self):
        executable = self.stage / 'tool'
        executable.write_text('executable')
        executable.chmod(0o755)
        if os.name != 'nt': (self.stage / 'tool-link').symlink_to('tool')
        output = Path(self.temp.name) / 'test.zip'
        package.write_zip(self.stage, output)
        with zipfile.ZipFile(output) as archive:
            self.assertEqual(archive.read('release/tool'), b'executable')
            if os.name != 'nt':
                self.assertTrue(archive.getinfo('release/tool').external_attr >> 16 & stat.S_IXUSR)
                self.assertTrue(stat.S_ISLNK(archive.getinfo('release/tool-link').external_attr >> 16))
                self.assertEqual(archive.read('release/tool-link'), b'tool')


if __name__ == '__main__': unittest.main()
