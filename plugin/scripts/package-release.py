#!/usr/bin/env python3
"""Package a tested build from the checked-out annotated release tag. No upload."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import shutil
import stat
import subprocess
import tarfile
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[2]


def module(name, filename):
    spec = importlib.util.spec_from_file_location(name, Path(__file__).with_name(filename))
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


build = module('native_build', 'build.py')
tags = module('release_tags', 'release-tag.py')


def verify_stage(stage, target, commit, fingerprint):
    manifest = json.loads((stage / 'BUILD.json').read_text())
    if (manifest['target'] != target or manifest['sourceCommit'] != commit
            or manifest['workingTreeDirty'] is not False or manifest['tests'] != 'passed'
            or manifest['coreOnly'] is not False):
        raise ValueError('Release requires a clean, fully tested build of this exact commit and target')
    if manifest['sourceFiles'] != fingerprint or build.source_fingerprint(stage / 'source/plugin') != fingerprint:
        raise ValueError('Build, bundled source and release source fingerprints must match')
    listed = set()
    for line in (stage / 'SHA256SUMS.txt').read_text().splitlines():
        digest, name = line.split('  ', 1)
        path = stage / name
        if not path.resolve().is_relative_to(stage.resolve()) or name in listed:
            raise ValueError('Invalid checksum path')
        if not path.is_file() or path.is_symlink() or build.sha(path) != digest:
            raise ValueError('Build checksum mismatch: ' + name)
        listed.add(name)
    actual = {p.relative_to(stage).as_posix() for p in stage.rglob('*')
              if p.is_file() and not p.is_symlink() and p != stage / 'SHA256SUMS.txt'}
    if actual != listed:
        raise ValueError('Build contains missing or unchecksummed files')
    expected_binaries = ({'VST3/Submotif.vst3/Contents/x86_64-win/Submotif.vst3', 'Standalone/Submotif.exe'}
                         if target == 'windows-x64' else
                         {'VST3/Submotif.vst3/Contents/MacOS/Submotif', 'Standalone/Submotif.app/Contents/MacOS/Submotif'})
    if set(manifest['binaries']) != {'Undertow_artefacts/Release/' + p for p in expected_binaries}:
        raise ValueError('Build must contain both expected binaries')
    if set(manifest['architectures']) != set(build.ARCHES[target].split(';')):
        raise ValueError('Incorrect build architecture metadata')
    for path, digest in manifest['binaries'].items():
        relative = Path(path).relative_to('Undertow_artefacts/Release')
        binary = stage.joinpath(*relative.parts[1:])
        if build.sha(binary) != digest:
            raise ValueError('Binary differs from tested build')
        if target == 'windows-x64':
            build.verify_windows_binary(binary)
        else:
            arches = subprocess.check_output(['lipo', '-archs', str(binary)], text=True).split()
            if set(arches) != set(build.ARCHES[target].split(';')):
                raise ValueError('Incorrect Mac architectures')
    if target.startswith('macos-'):
        for name in ('Submotif.vst3', 'Submotif.app'):
            subprocess.run(['codesign', '--verify', '--deep', '--strict', str(stage / name)], check=True)
    for name in ('LICENSE', 'JUCE-LICENSE.md', 'THIRD_PARTY_NOTICES.md',
                 'source/.deps/JUCE/CMakeLists.txt', 'max/Submotif.amxd'):
        if not (stage / name).is_file():
            raise ValueError('Missing release content: ' + name)
    return manifest


def write_zip(stage, destination):
    # Unix file modes and symbolic links are retained for Finder / Archive Utility.
    with zipfile.ZipFile(destination, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
        for path in sorted(stage.rglob('*')):
            name = (Path(stage.name) / path.relative_to(stage)).as_posix()
            if path.is_symlink():
                info = zipfile.ZipInfo(name)
                info.create_system = 3
                info.external_attr = (stat.S_IFLNK | 0o777) << 16
                archive.writestr(info, os.readlink(path))
            else:
                archive.write(path, name)
    with zipfile.ZipFile(destination) as archive:
        bad = archive.testzip()
        if bad:
            raise ValueError('ZIP integrity check failed: ' + bad)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target', required=True, choices=list(build.ARCHES))
    parser.add_argument('--tag', required=True)
    args = parser.parse_args()
    tags.validate_tag(ROOT, args.tag)
    commit, dirty = build.source_provenance(ROOT)
    if dirty:
        raise ValueError('Commit all changes before packaging')
    artifact = ROOT / 'dist/builds' / f'Submotif-{args.target}-development.tar.gz'
    output = ROOT / 'dist/releases' / args.tag
    output.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='.release-', dir=output) as temporary:
        temporary = Path(temporary)
        with tarfile.open(artifact) as archive:
            archive.extractall(temporary, filter='data')
        stage = temporary / f'Submotif-{args.target}-development'
        manifest = verify_stage(stage, args.target, commit, build.source_fingerprint(ROOT / 'plugin'))
        renamed = temporary / f'Submotif-{args.tag}-{args.target}'
        stage.rename(renamed)
        stage = renamed
        (stage / 'DEVELOPMENT.txt').unlink()
        shutil.copy2(ROOT / 'docs/INSTALL.md', stage / 'INSTALL.md')
        release = {'tag': args.tag, 'sourceCommit': commit, 'target': args.target,
                   'architectures': manifest['architectures'], 'tests': manifest['tests'],
                   'signing': manifest['signing'], 'binaries': manifest['binaries'],
                   'buildManifest': 'BUILD.json', 'source': 'source/'}
        (stage / 'RELEASE.json').write_text(json.dumps(release, indent=2) + '\n')
        (stage / 'SHA256SUMS.txt').write_text(''.join(
            f'{build.sha(p)}  {p.relative_to(stage).as_posix()}\n'
            for p in sorted(stage.rglob('*'))
            if p.is_file() and not p.is_symlink() and p != stage / 'SHA256SUMS.txt'))
        name = f'Submotif-{args.target}.zip'
        write_zip(stage, temporary / name)
        os.replace(temporary / name, output / name)
        (output / (name + '.sha256')).write_text(f'{build.sha(output / name)}  {name}\n')
        (output / f'Submotif-{args.target}.json').write_text(json.dumps(release, indent=2) + '\n')
    print(f'Verified release package: {output / name}')


if __name__ == '__main__':
    main()
