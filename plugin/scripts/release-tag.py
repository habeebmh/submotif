#!/usr/bin/env python3
"""Create or validate immutable, annotated version tags. Never publish a Release."""
import argparse
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args], text=True, stderr=subprocess.PIPE).strip()


def project_version(root):
    text = (root / 'plugin/CMakeLists.txt').read_text()
    match = re.search(r'project\(Undertow\s+VERSION\s+(\d+\.\d+\.\d+)\b', text)
    if not match: raise ValueError('Could not read the three-part project version from plugin/CMakeLists.txt')
    return match.group(1)


def validate_tag(root, tag, *, main_ref='origin/main'):
    if not re.fullmatch(r'v(?:0|[1-9]\d*)\.(?:0|[1-9]\d*)\.(?:0|[1-9]\d*)', tag):
        raise ValueError('Release tags must use vMAJOR.MINOR.PATCH')
    if tag != 'v' + project_version(root): raise ValueError('Release tag must match the CMake project version')
    ref = 'refs/tags/' + tag
    if git(root, 'cat-file', '-t', ref) != 'tag': raise ValueError('Release tags must be annotated')
    if git(root, 'rev-parse', ref + '^{commit}') != git(root, 'rev-parse', 'HEAD'):
        raise ValueError('Release tag must point to the checked-out HEAD')
    result = subprocess.run(['git', '-C', str(root), 'merge-base', '--is-ancestor', 'HEAD', main_ref], capture_output=True)
    if result.returncode: raise ValueError('Release commit must be part of main history')


def create_tag(root, *, push=False):
    if git(root, 'branch', '--show-current') != 'main': raise ValueError('Create releases from main')
    if git(root, 'status', '--porcelain'): raise ValueError('Commit all changes before creating a release tag')
    tag = 'v' + project_version(root)
    exists = subprocess.run(['git', '-C', str(root), 'show-ref', '--verify', '--quiet', 'refs/tags/' + tag])
    if exists.returncode == 0: raise ValueError(f'{tag} already exists; never move an existing release tag')
    remote = git(root, 'ls-remote', '--exit-code', 'origin', 'refs/heads/main').split()[0]
    if remote != git(root, 'rev-parse', 'HEAD'): raise ValueError('Push main and verify its checks before creating a release tag')
    # This is an explicit release action; do not call it while publication is held.
    git(root, 'tag', '--annotate', tag, '--message', 'Submotif ' + project_version(root))
    print(f'Created annotated tag {tag}')
    if push:
        subprocess.run(['git', '-C', str(root), 'push', 'origin', 'refs/tags/' + tag], check=True)
    else:
        print(f'Not pushed. When ready: git push origin refs/tags/{tag}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    action = parser.add_mutually_exclusive_group(required=True)
    action.add_argument('--check', metavar='TAG', help='Validate the checked-out annotated release tag and main ancestry.')
    action.add_argument('--dry-run', action='store_true', help='Show the planned version tag without changing Git.')
    action.add_argument('--create', action='store_true', help='Explicitly create the annotated tag after release approval.')
    parser.add_argument('--push', action='store_true', help='Also push the newly created tag; requires --create.')
    args = parser.parse_args()
    if args.push and not args.create: parser.error('--push requires --create')
    if args.check:
        validate_tag(ROOT, args.check)
        print(f'Validated annotated release tag {args.check}')
    elif args.dry_run:
        tag = 'v' + project_version(ROOT)
        print(f'Planned tag: {tag}\nCreation requires clean main, synchronized with origin/main.\nNo tag created or pushed.')
    else: create_tag(ROOT, push=args.push)


if __name__ == '__main__':
    try: main()
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        print(f'Release tag failed: {error}', file=sys.stderr); sys.exit(1)
