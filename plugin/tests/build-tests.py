#!/usr/bin/env python3
"""Build orchestration contracts; real compiler coverage runs in the OS matrix."""
import importlib.util
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch
import json
import subprocess

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('submotif_build', ROOT / 'scripts/build.py')
build = importlib.util.module_from_spec(spec)
spec.loader.exec_module(build)

class BuildTests(unittest.TestCase):
    def test_mac_targets_have_isolated_explicit_architectures(self):
        expected = {'macos-arm64':'arm64', 'macos-x86_64':'x86_64', 'macos-universal':'arm64;x86_64'}
        directories = set()
        for target, arch in expected.items():
            plan = build.build_plan(target, 'Darwin', ninja=True)
            self.assertIn('-DCMAKE_OSX_ARCHITECTURES='+arch, plan['configure'])
            self.assertNotIn('-A', plan['configure'])
            directories.add(plan['directory'])
        self.assertEqual(len(directories), 3)

    def test_windows_uses_msvc_x64_and_separate_config_executable(self):
        plan = build.build_plan('windows-x64', 'Windows', ninja=True)
        self.assertIn('Visual Studio 17 2022', plan['configure'])
        self.assertEqual(plan['configure'][plan['configure'].index('-A')+1], 'x64')
        self.assertFalse(any('OSX' in x for x in plan['configure']))
        self.assertEqual(plan['probe'].name, 'UndertowGenerationProbe.exe')
        self.assertEqual(plan['probe'].parent.name, 'Release')

    def test_wrong_host_refused_before_building(self):
        for target, host in [('windows-x64','Darwin'), ('macos-arm64','Windows'), ('macos-universal','Linux')]:
            with self.assertRaisesRegex(ValueError, 'requires'):
                build.build_plan(target, host)

    def test_old_build_cache_cannot_silently_change_architecture(self):
        with tempfile.TemporaryDirectory() as folder:
            directory = Path(folder)
            (directory/'CMakeCache.txt').write_text('CMAKE_OSX_ARCHITECTURES:STRING=arm64\n')
            with self.assertRaisesRegex(ValueError,'architecture'):
                build.check_cache(directory, 'macos-x86_64')
            build.check_cache(directory, 'macos-arm64')

    def test_windows_binary_machine_is_checked_not_just_filename(self):
        with tempfile.TemporaryDirectory() as folder:
            p=Path(folder)/'Submotif.vst3'
            raw=bytearray(256);raw[:2]=b'MZ';struct.pack_into('<I',raw,0x3c,128)
            raw[128:132]=b'PE\0\0';struct.pack_into('<H',raw,132,0x8664);p.write_bytes(raw)
            build.verify_windows_binary(p)
            struct.pack_into('<H',raw,132,0xaa64);p.write_bytes(raw)
            with self.assertRaisesRegex(ValueError,'x64'):
                build.verify_windows_binary(p)
            p.write_bytes(b'not a binary')
            with self.assertRaises(ValueError): build.verify_windows_binary(p)

    def test_zero_jobs_rejected(self):
        with self.assertRaises(ValueError): build.build_plan('macos-arm64','Darwin',jobs=0)

    def test_rebuilding_an_unversioned_source_copy_does_not_require_git_history(self):
        with tempfile.TemporaryDirectory() as folder:
            self.assertEqual(build.source_provenance(Path(folder)), (None, None))

    def test_failed_rebuild_cannot_leave_an_old_success_manifest(self):
        with tempfile.TemporaryDirectory() as folder:
            directory = Path(folder); manifest = directory / 'BUILD.json'
            manifest.write_text(json.dumps({'tests':'passed','sourceCommit':'old'}))
            product=directory/'Submotif';product.write_bytes(b'old binary')
            def fake_run(command, **kwargs):
                if command[0] == 'cmake' and '--build' in command:
                    product.write_bytes(b'rebuilt binary')
                if command[0] == 'ctest': raise subprocess.CalledProcessError(1, command)
            with patch.object(build.platform, 'system', return_value='Darwin'), \
                 patch.object(build.shutil, 'which', return_value='/fake/tool'), \
                 patch.object(build, 'run', side_effect=fake_run), \
                 patch.object(build, 'verify_products', return_value=(product,product,[product])), \
                 patch.object(build.sys, 'argv', ['build.py','--target','macos-arm64','--build-dir',str(directory)]):
                with self.assertRaises(subprocess.CalledProcessError): build.main()
            self.assertEqual(product.read_bytes(), b'rebuilt binary')
            self.assertFalse(manifest.exists() and json.loads(manifest.read_text()).get('tests') == 'passed')

if __name__ == '__main__': unittest.main()
