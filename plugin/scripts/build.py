#!/usr/bin/env python3
"""Build/test Submotif on macOS or Windows; never install or publish a release."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import shutil
import struct
import subprocess
import sys
import tarfile
import tempfile

PLUGIN = Path(__file__).resolve().parents[1]
ROOT = PLUGIN.parent
ARCHES = {'macos-arm64': 'arm64', 'macos-x86_64': 'x86_64',
          'macos-universal': 'arm64;x86_64', 'windows-x64': 'x64'}


def build_plan(target, system, *, ninja=False, jobs=4, directory=None, core_only=False):
    required = 'Windows' if target == 'windows-x64' else 'Darwin'
    if target not in ARCHES or system != required:
        raise ValueError(f'{target} requires {required}; use the GitHub Native builds workflow for other systems.')
    if jobs < 1:
        raise ValueError('--jobs must be positive')
    directory = Path(directory or PLUGIN / ('build-' + target + ('-core' if core_only else ''))).resolve()
    configure = ['cmake', '-S', str(PLUGIN), '-B', str(directory)]
    if system == 'Windows':
        configure += ['-G', 'Visual Studio 17 2022', '-A', 'x64']
    else:
        configure += ['-G', 'Ninja' if ninja else 'Unix Makefiles',
                      '-DCMAKE_OSX_ARCHITECTURES=' + ARCHES[target], '-DCMAKE_OSX_DEPLOYMENT_TARGET=11.0']
    configure += ['-DCMAKE_BUILD_TYPE=Release', '-DUNDERTOW_CORE_ONLY=' + ('ON' if core_only else 'OFF')]
    probe = directory / ('Release/UndertowGenerationProbe.exe' if system == 'Windows' else 'UndertowGenerationProbe')
    return {'target': target, 'directory': directory, 'probe': probe, 'configure': configure,
            'build': ['cmake', '--build', str(directory), '--config', 'Release', '--parallel', str(jobs)],
            'test': ['ctest', '--test-dir', str(directory), '-C', 'Release', '--output-on-failure', '--no-tests=error']}


def read_cache(directory):
    path = directory / 'CMakeCache.txt'
    if not path.exists(): return {}
    values = {}
    for line in path.read_text(errors='replace').splitlines():
        if '=' in line and ':' in line.split('=', 1)[0]:
            key, value = line.split('=', 1); values[key.split(':', 1)[0]] = value
    return values


def check_cache(directory, target):
    cache = read_cache(directory)
    key = 'CMAKE_GENERATOR_PLATFORM' if target == 'windows-x64' else 'CMAKE_OSX_ARCHITECTURES'
    if key in cache and set(cache[key].split(';')) != set(ARCHES[target].split(';')):
        raise ValueError(f'Existing build architecture {cache[key]!r} differs from {target}; choose a new build directory.')


def run(command, **kwargs):
    print('+ ' + subprocess.list2cmdline(list(map(str, command))), flush=True)
    return subprocess.run(list(map(str, command)), check=True, **kwargs)


def verify_windows_binary(path):
    with Path(path).open('rb') as source:
        header = source.read(64)
        if len(header) != 64 or header[:2] != b'MZ': raise ValueError(f'Invalid Windows binary: {path}')
        source.seek(struct.unpack_from('<I', header, 0x3c)[0]); pe = source.read(6)
    if len(pe) != 6 or pe[:4] != b'PE\0\0' or struct.unpack_from('<H', pe, 4)[0] != 0x8664:
        raise ValueError(f'Expected an x64 PE binary: {path}')


def verify_products(plan):
    base = plan['directory'] / 'Undertow_artefacts/Release'
    vst = base / 'VST3/Submotif.vst3'
    if plan['target'] == 'windows-x64':
        app = base / 'Standalone/Submotif.exe'
        binaries = [vst / 'Contents/x86_64-win/Submotif.vst3', app]
        for binary in binaries: verify_windows_binary(binary)
    else:
        app = base / 'Standalone/Submotif.app'
        binaries = [vst / 'Contents/MacOS/Submotif', app / 'Contents/MacOS/Submotif']
        for binary in binaries:
            actual = set(run(['lipo', '-archs', binary], capture_output=True, text=True).stdout.split())
            if actual != set(ARCHES[plan['target']].split(';')):
                raise ValueError(f'{binary} has {actual}, expected {ARCHES[plan["target"]]}')
        # JUCE writes its VST3 manifest after linking. Seal the finished bundles.
        for bundle in (vst, app):
            run(['codesign', '--force', '--deep', '--sign', '-', bundle])
            run(['codesign', '--verify', '--deep', '--strict', bundle])
    return vst, app, binaries


def sha(path): return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def source_provenance(root):
    # Source-inclusive artifacts are rebuildable without a .git directory.
    try:
        top = subprocess.check_output(['git', '-C', str(root), 'rev-parse', '--show-toplevel'],
                                      stderr=subprocess.DEVNULL, text=True).strip()
        if Path(top).resolve() != root.resolve(): return None, None
        commit = subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD'], text=True).strip()
        dirty = bool(subprocess.check_output(['git', '-C', str(root), 'status', '--porcelain'], text=True))
        return commit, dirty
    except (OSError, subprocess.CalledProcessError):
        return None, None


def source_fingerprint(plugin):
    files = [plugin / 'CMakeLists.txt']
    for folder in ('src', 'tests', 'scripts', 'max'):
        files += [p for p in (plugin / folder).rglob('*') if p.is_file()
                  and '__pycache__' not in p.parts and p.suffix != '.pyc' and p.name != '.DS_Store']
    return {p.relative_to(plugin).as_posix(): sha(p) for p in sorted(files)}


def development_artifact(plan, products, manifest):
    """A CI testing artifact with rebuildable source, not a release installer."""
    cache = read_cache(plan['directory'])
    juce = Path(cache.get('JUCE_DIR', ROOT / '.deps/JUCE'))
    if not (juce / 'CMakeLists.txt').is_file():
        juce = Path(cache.get('juce_SOURCE_DIR', plan['directory'] / '_deps/juce-src'))
    pin = re.search(r'GIT_TAG\s+([0-9a-f]{40})', (PLUGIN / 'CMakeLists.txt').read_text()).group(1)
    actual = run(['git', '-C', juce, 'rev-parse', 'HEAD'], capture_output=True, text=True).stdout.strip()
    dirty = run(['git', '-C', juce, 'status', '--porcelain', '--untracked-files=no'], capture_output=True, text=True).stdout
    if actual != pin or dirty: raise ValueError('Development artifacts require the clean pinned JUCE source used by the build.')
    output = ROOT / 'dist/builds'; output.mkdir(parents=True, exist_ok=True)
    name = 'Submotif-' + plan['target'] + '-development'
    with tempfile.TemporaryDirectory(prefix='.native-', dir=output) as temporary:
        temporary = Path(temporary); stage = temporary / name; stage.mkdir()
        for product in products[:2]:
            if product.is_dir(): shutil.copytree(product, stage / product.name, symlinks=True)
            else: shutil.copy2(product, stage / product.name)
        ignore = shutil.ignore_patterns('__pycache__', '*.pyc', '.DS_Store')
        shutil.copytree(PLUGIN / 'max', stage / 'max', ignore=ignore)
        source = stage / 'source'; (source / 'plugin').mkdir(parents=True)
        for folder in ('src', 'tests', 'scripts', 'max'):
            shutil.copytree(PLUGIN / folder, source / 'plugin' / folder, ignore=ignore)
        for filename in ('CMakeLists.txt', 'README.md'): shutil.copy2(PLUGIN / filename, source / 'plugin' / filename)
        for filename in ('LICENSE', 'README.md'):
            shutil.copy2(ROOT / filename, stage / filename)
            shutil.copy2(ROOT / filename, source / filename)
        shutil.copytree(ROOT / 'docs', source / 'docs', ignore=shutil.ignore_patterns('superpowers', '__pycache__', '*.pyc'))
        if source_fingerprint(source / 'plugin') != manifest['sourceFiles']:
            raise ValueError('Source changed while copying the development artifact; rebuild before distributing.')
        vendor = source / '.deps/JUCE'; vendor.mkdir(parents=True)
        vendor_tar = temporary / 'juce.tar'
        run(['git', '-C', juce, 'archive', '--format=tar', '--output', vendor_tar, pin])
        with tarfile.open(vendor_tar) as archive: archive.extractall(vendor, filter='data')
        shutil.copy2(juce / 'LICENSE.md', stage / 'JUCE-LICENSE.md')
        shutil.copy2(ROOT / 'docs/THIRD_PARTY_NOTICES.md', stage / 'THIRD_PARTY_NOTICES.md')
        (stage / 'BUILD.json').write_text(json.dumps(manifest, indent=2) + '\n')
        (stage / 'DEVELOPMENT.txt').write_text(
            'Development/testing build. Not an approved public release.\n'
            'macOS: ad-hoc signed, not Developer ID signed or notarized. Windows: unsigned.\n'
            'source/ contains matching project source and complete pinned JUCE source/notices.\n'
            'Keep every file in max/ together. In Ableton, put the Max MIDI device before your instrument.\n'
            'The VST3 uses a separate MIDI track in Ableton; see source/README.md.\n')
        (stage / 'SHA256SUMS.txt').write_text(''.join(f'{sha(p)}  {p.relative_to(stage).as_posix()}\n'
            for p in sorted(stage.rglob('*')) if p.is_file() and not p.is_symlink()))
        archive_path = temporary / (name + '.tar.gz')
        with tarfile.open(archive_path, 'w:gz') as archive: archive.add(stage, arcname=name)
        os.replace(archive_path, output / archive_path.name)
    print(f'Development artifact: {output / (name + ".tar.gz")}', flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target', choices=list(ARCHES) + ['macos-all'])
    parser.add_argument('--jobs', type=int, default=4)
    parser.add_argument('--build-dir', type=Path, help='Override one target directory; normally use the isolated default.')
    parser.add_argument('--core-only', action='store_true')
    parser.add_argument('--skip-tests', action='store_true', help='Explicit compile-only cross-architecture check; recorded as untested.')
    parser.add_argument('--artifact', action='store_true', help='Create a source-inclusive development tar.gz after successful tests.')
    parser.add_argument('--dry-run', action='store_true', help='Print commands without building or writing files.')
    args = parser.parse_args()
    system = platform.system()
    target = args.target or ('windows-x64' if system == 'Windows' else 'macos-universal')
    targets = [t for t in ARCHES if t.startswith('macos-')] if target == 'macos-all' else [target]
    if args.build_dir and len(targets) > 1: parser.error('--build-dir requires a single target')
    if args.artifact and (args.skip_tests or args.core_only): parser.error('--artifact requires a full build with tests')
    if args.artifact and sys.version_info < (3, 12): parser.error('--artifact requires Python 3.12+ for safe source archive extraction')
    # Prefer the repository's existing CMake environment; nested JUCE helpers need the same PATH.
    local_bin = ROOT / '.tools/venv' / ('Scripts' if system == 'Windows' else 'bin')
    if local_bin.is_dir(): os.environ['PATH'] = str(local_bin) + os.pathsep + os.environ.get('PATH', '')
    plans = [build_plan(t, system, ninja=shutil.which('ninja') is not None, jobs=args.jobs,
                        directory=args.build_dir, core_only=args.core_only) for t in targets]
    for plan in plans:
        if args.dry_run:
            print(json.dumps({k: str(v) if isinstance(v, Path) else v for k,v in plan.items()}, indent=2)); continue
        for tool in ('cmake', 'ctest') + (() if args.core_only or args.skip_tests else ('node',)):
            if not shutil.which(tool): raise ValueError(f'{tool} is required; see docs/BUILDING.md')
        check_cache(plan['directory'], plan['target'])
        manifest_path = plan['directory'] / 'BUILD.json'
        # A failed rebuild can already have replaced binaries. Never retain an
        # earlier success claim for the now-partially-rebuilt output directory.
        manifest_path.unlink(missing_ok=True)
        before = source_fingerprint(PLUGIN)
        run(plan['configure']); run(plan['build'])
        products = None if args.core_only else verify_products(plan)
        if not args.skip_tests:
            run(plan['test'])
            if not args.core_only:
                run(['node', PLUGIN / 'tests/max-tests.js'])
                run([sys.executable, PLUGIN / 'tests/generation-parity.py', '--adapter', plan['probe']])
                run([sys.executable, '-B', PLUGIN / 'tests/audit-tests.py'])
        if source_fingerprint(PLUGIN) != before:
            raise ValueError('Source changed during the build; rerun with a stable source tree.')
        commit, dirty = source_provenance(ROOT)
        manifest = {'target': plan['target'], 'architectures': ARCHES[plan['target']].split(';'),
                    'builtAtUTC': datetime.now(timezone.utc).isoformat(), 'sourceCommit': commit, 'workingTreeDirty': dirty,
                    'sourceFiles': before,
                    'tests': 'skipped' if args.skip_tests else 'passed', 'coreOnly': args.core_only,
                    'signing': 'unsigned' if system == 'Windows' else 'ad-hoc development',
                    'binaries': {p.relative_to(plan['directory']).as_posix(): sha(p) for p in products[2]} if products else {},
                    'releasePublished': False}
        pending_manifest = plan['directory'] / '.BUILD.json.tmp'
        pending_manifest.write_text(json.dumps(manifest, indent=2) + '\n')
        os.replace(pending_manifest, manifest_path)
        if args.artifact: development_artifact(plan, products, manifest)
        print(f'Build complete: {plan["directory"]}', flush=True)


if __name__ == '__main__':
    try: main()
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        print(f'Build failed: {error}', file=sys.stderr); sys.exit(1)
