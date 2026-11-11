#!/usr/bin/env bash
# Configure an existing public repository; never authenticate or publish source.
set -euo pipefail
command -v python3 >/dev/null 2>&1 || { echo 'Python 3 is required.' >&2; exit 1; }
exec python3 - "$@" <<'PY'
import argparse
import base64
import copy
import difflib
import json
import os
import re
import shutil
import subprocess
import sys

OWNER = 'habeebmh'
REPO = OWNER + '/submotif'
BASE = 'repos/' + REPO
BRANCH = BASE + '/branches/main'
PROTECTION = BRANCH + '/protection'
CHECK = 'Portable checks'
API_VERSION = '2026-03-10'


class SetupError(Exception):
    pass


def require(condition, message):
    if not condition:
        raise SetupError(message)


def api(endpoint, method='GET', payload=None, missing_ok=False):
    # Explicit hostname/method avoid local git remotes, GH_HOST, and gh's implicit POST.
    command = ['gh', 'api', '--hostname', 'github.com', '--method', method,
               '-H', 'Accept: application/vnd.github+json',
               '-H', 'X-GitHub-Api-Version: ' + API_VERSION, endpoint]
    if payload is not None:
        command += ['--input', '-']
    environment = dict(os.environ, GH_PROMPT_DISABLED='1', GH_NO_UPDATE_NOTIFIER='1',
                       GH_NO_EXTENSION_UPDATE_NOTIFIER='1')
    environment.pop('GH_DEBUG', None)
    try:
        result = subprocess.run(command, input=json.dumps(payload) if payload is not None else None,
                                text=True, capture_output=True, env=environment, timeout=60)
    except subprocess.TimeoutExpired:
        raise SetupError('GitHub request timed out: ' + method + ' ' + endpoint) from None
    if result.returncode:
        status = re.search(r'HTTP (\d{3})', result.stderr)
        if missing_ok and status and status.group(1) == '404':
            return None
        detail = 'HTTP ' + status.group(1) if status else 'authentication, network, or CLI error'
        # Report the API's validation reason, never raw CLI stderr/debug output.
        try:
            body = json.loads(result.stdout)
            message = body.get('message', '') if isinstance(body, dict) else ''
        except json.JSONDecodeError:
            message = ''
        if not isinstance(message, str):
            message = ''
        message = re.sub(r'\b(?:gh[pousr]_[A-Za-z0-9_]+|github_pat_[A-Za-z0-9_]+)\b|\bBearer\s+\S+',
                         '[REDACTED]', message, flags=re.IGNORECASE)
        message = re.sub(r'[\x00-\x1f\x7f-\x9f]+', ' ', message).strip()[:1200]
        raise SetupError('GitHub request failed (' + detail + '): ' + method + ' ' + endpoint
                         + (': ' + message if message else ''))
    try:
        return json.loads(result.stdout) if result.stdout.strip() else None
    except json.JSONDecodeError:
        raise SetupError('GitHub returned an unexpected response for ' + endpoint) from None


def enabled(data, name):
    value = data.get(name)
    if value is None:
        return False
    require(isinstance(value, dict) and isinstance(value.get('enabled'), bool),
            'Unrecognized protection setting: ' + name)
    return value['enabled']


def known_keys(data, names, context):
    unknown = set(data) - set(names)
    require(not unknown, 'Unrecognized ' + context + ' fields; review manually: ' + ', '.join(sorted(unknown)))


def normalize_protection(raw):
    """Map documented GET fields to PUT fields, retaining unrelated protections."""
    raw = raw or {}
    boolean_names = ('required_linear_history', 'allow_force_pushes', 'allow_deletions',
                     'block_creations', 'required_conversation_resolution', 'lock_branch',
                     'allow_fork_syncing')
    known_keys(raw, ('url', 'required_status_checks', 'enforce_admins',
                    'required_pull_request_reviews', 'restrictions', 'required_signatures') + boolean_names,
               'branch-protection')
    # Those organization-only restrictions cannot safely be round-tripped here.
    require(raw.get('restrictions') is None, 'Unexpected push restrictions on a personal repository; review manually.')
    reviews = raw.get('required_pull_request_reviews')
    if reviews is not None:
        known_keys(reviews, ('url', 'dismiss_stale_reviews', 'require_code_owner_reviews',
                            'required_approving_review_count', 'require_last_push_approval',
                            'dismissal_restrictions', 'bypass_pull_request_allowances'), 'review')
        for field in ('dismissal_restrictions', 'bypass_pull_request_allowances'):
            actors = reviews.get(field) or {}
            require(not any(actors.get(key) for key in ('users', 'teams', 'apps')),
                    'Existing ' + field + ' needs manual review; no allowance will be removed silently.')
        reviews = {key: reviews.get(key, False) for key in
                   ('dismiss_stale_reviews', 'require_code_owner_reviews', 'require_last_push_approval')}
        reviews['required_approving_review_count'] = raw['required_pull_request_reviews'].get('required_approving_review_count', 0)
    status = raw.get('required_status_checks')
    if status is not None:
        known_keys(status, ('url', 'contexts_url', 'strict', 'contexts', 'checks', 'enforcement_level'), 'status-check')
        checks = copy.deepcopy(status.get('checks', []))
        contexts = status.get('contexts', [])
        require(all(isinstance(check, dict) and set(check) <= {'context', 'app_id'} for check in checks),
                'Unrecognized required check; review manually.')
        require(set(contexts) <= {check.get('context') for check in checks},
                'Existing check app bindings are unavailable; review manually rather than changing them.')
        for check in checks:
            # REST accepts -1, not null, when any app may supply an existing check.
            if check.get('app_id') is None:
                check['app_id'] = -1
        # PUT accepts either legacy contexts or app-bound checks, never both.
        status = {'strict': status.get('strict', False),
                  'checks': sorted(checks, key=lambda check: (check['context'], check['app_id']))}
    result = {'required_status_checks': status, 'enforce_admins': enabled(raw, 'enforce_admins'),
              'required_pull_request_reviews': reviews, 'restrictions': None}
    result.update({name: enabled(raw, name) for name in boolean_names})
    return result


def identity():
    user = api('user')
    require(user.get('login', '').lower() == OWNER, 'Use an existing GitHub CLI session authenticated as @' + OWNER + '.')
    repository = api(BASE)
    require(repository.get('full_name', '').lower() == REPO and
            repository.get('owner', {}).get('login', '').lower() == OWNER and
            repository.get('owner', {}).get('type') == 'User', 'Repository ownership does not match ' + REPO + '.')
    require(repository.get('permissions', {}).get('admin') is True, 'Repository administration access is required.')
    require(repository.get('private') is False and repository.get('visibility') == 'public',
            'The repository must already be public. This script never changes visibility.')
    require(not any(repository.get(key) for key in ('archived', 'disabled', 'fork')),
            'The repository must be an active original repository, not an archived, disabled, or forked repository.')
    require(repository.get('default_branch') == 'main', 'The existing default branch must be main.')
    return repository


def main_head():
    branch = api(BRANCH, missing_ok=True)
    require(branch is not None, 'main does not exist. Publish the reviewed initial source and establish CI first.')
    sha = branch.get('commit', {}).get('sha', '')
    require(re.fullmatch(r'[0-9a-f]{40}', sha) is not None, 'Cannot determine the main commit.')
    return sha


def rulesets():
    result = api(BASE + '/rulesets?includes_parents=true&per_page=100')
    require(isinstance(result, list) and len(result) < 100, 'Too many or unrecognized rulesets; review manually.')
    require(not any(rule.get('enforcement') not in ('disabled', None) for rule in result),
            'Existing active/evaluating rulesets require manual review. They will not be replaced or disabled.')
    return result


def codeowners(sha):
    data = api(BASE + '/contents/.github/CODEOWNERS?ref=' + sha)
    require(data.get('encoding') == 'base64', 'Cannot read the committed CODEOWNERS file.')
    try:
        lines = base64.b64decode(data['content']).decode('utf-8').splitlines()
    except (KeyError, ValueError, UnicodeDecodeError):
        raise SetupError('Cannot decode the committed CODEOWNERS file.') from None
    entries = [line.split('#', 1)[0].split() for line in lines if line.split('#', 1)[0].strip()]
    require(any(entry == ['*', '@' + OWNER] for entry in entries) and
            all(len(entry) == 2 and entry[1].lower() == '@' + OWNER for entry in entries),
            'Committed CODEOWNERS must assign every path solely to @' + OWNER + ', without overriding owners.')
    errors = api(BASE + '/codeowners/errors?ref=' + sha)
    require(errors.get('errors') == [], 'GitHub reports CODEOWNERS errors; correct them before setup.')


def successful_ci(sha):
    workflow = api(BASE + '/actions/workflows/ci.yml')
    require(workflow.get('state') == 'active', 'Enable and run the committed CI workflow before setup.')
    listing = api(BASE + '/actions/workflows/ci.yml/runs?branch=main&head_sha=' + sha + '&per_page=100')
    require(listing.get('total_count', 0) <= 100, 'Too many CI runs to assess safely; review manually.')
    runs = [run for run in listing.get('workflow_runs', [])
            if run.get('head_sha') == sha and run.get('head_branch') == 'main'
            and run.get('event') in ('push', 'workflow_dispatch')]
    require(runs, 'CI has not run on the current main commit. Wait for a successful main run first.')
    latest = max(runs, key=lambda run: run['id'])
    require(latest.get('status') == 'completed' and latest.get('conclusion') == 'success',
            'The latest main CI run is not successful. Fix or finish CI before requiring it.')
    listing = api(BASE + '/actions/runs/' + str(latest['id']) + '/jobs?filter=latest&per_page=100')
    require(listing.get('total_count', 0) <= 100, 'Too many CI jobs to assess safely; review manually.')
    jobs = [job for job in listing.get('jobs', []) if job.get('name') == CHECK]
    require(len(jobs) == 1 and jobs[0].get('status') == 'completed' and jobs[0].get('conclusion') == 'success',
            'The latest CI run must contain exactly one successful ' + CHECK + ' job.')
    match = re.fullmatch(r'https://api\.github\.com/repos/' + re.escape(REPO) + r'/check-runs/(\d+)',
                         jobs[0].get('check_run_url', ''), re.IGNORECASE)
    require(match is not None, 'Cannot establish the CI check provider.')
    check = api(BASE + '/check-runs/' + match.group(1))
    require(check.get('head_sha') == sha and check.get('name') == CHECK and check.get('conclusion') == 'success'
            and check.get('app', {}).get('slug') == 'github-actions', 'CI check provider does not match GitHub Actions.')
    app_id = check['app'].get('id')
    require(type(app_id) is int and app_id > 0, 'Invalid CI application identifier.')
    return app_id


def desired_protection(current, app_id):
    require(not current['enforce_admins'],
            'Administrator enforcement is already enabled. This script will not weaken it; review the sole-owner policy manually.')
    require(not current['lock_branch'], 'main is locked. This script will not unlock it.')
    desired = copy.deepcopy(current)
    reviews = desired['required_pull_request_reviews'] or {'require_last_push_approval': False}
    reviews.update(dismiss_stale_reviews=True, require_code_owner_reviews=True,
                   required_approving_review_count=max(1, reviews.get('required_approving_review_count', 0)))
    desired['required_pull_request_reviews'] = reviews
    status = desired['required_status_checks'] or {'checks': []}
    existing = [check for check in status['checks'] if check['context'] == CHECK]
    require(all(check['app_id'] in (app_id, -1) for check in existing),
            'Portable checks is already bound to a different application; review manually.')
    status.update(strict=True, checks=sorted([check for check in status['checks'] if check['context'] != CHECK]
                                           + [{'context': CHECK, 'app_id': app_id}],
                                           key=lambda check: (check['context'], check['app_id'])))
    desired.update(required_status_checks=status, required_conversation_resolution=True,
                   allow_force_pushes=False, allow_deletions=False)
    return desired


def run(apply):
    require(shutil.which('gh') is not None, 'Install GitHub CLI and authenticate separately as @' + OWNER + '.')
    repository = identity()
    sha = main_head()
    existing_rules = rulesets()
    raw = api(PROTECTION, missing_ok=True)
    current = normalize_protection(raw)
    codeowners(sha)
    app_id = successful_ci(sha)
    desired = desired_protection(current, app_id)
    change_protection = desired != current
    change_issues = repository.get('has_issues') is not True
    print(('APPLY' if apply else 'READ-ONLY PLAN') + ': https://github.com/' + REPO)
    print('Verified owner, public repository, main, CODEOWNERS, and successful Portable checks.')
    print('Main commit: ' + sha)
    print('Owner/admin bypass remains available; these rules are not bypass-proof.')
    if change_protection:
        before = json.dumps(current, indent=2, sort_keys=True).splitlines()
        after = json.dumps(desired, indent=2, sort_keys=True).splitlines()
        print('\n'.join(difflib.unified_diff(before, after, fromfile='current protection', tofile='proposed protection', lineterm='')))
    if change_issues:
        print('Repository change: enable Issues (has_issues = true).')
    if not change_protection and not change_issues:
        print('No changes needed; the requested settings are already present.')
        return
    if not apply:
        print('No settings changed. Re-run with --apply after reviewing this plan.')
        return
    # Stop if another administrator/commit changed the relevant state during planning.
    require(identity() == repository and main_head() == sha and rulesets() == existing_rules
            and api(PROTECTION, missing_ok=True) == raw,
            'Repository state changed during planning. No writes were made; run the plan again.')
    if change_protection:
        api(PROTECTION, method='PUT', payload=desired)
        print('Branch protection updated.')
    if change_issues:
        api(BASE, method='PATCH', payload={'has_issues': True})
        print('Issues enabled.')
    actual = api(PROTECTION)
    require(normalize_protection(actual) == desired, 'Protection verification failed; inspect settings before retrying.')
    require(enabled(actual, 'required_signatures') == enabled(raw or {}, 'required_signatures'),
            'Signature protection changed unexpectedly; inspect settings before retrying.')
    require(api(BASE).get('has_issues') is True, 'Issues verification failed; inspect repository settings.')
    print('Verified requested settings. No source, releases, or merge actions were published.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='setup-github.sh', description='Preview owner-review setup for an existing public ' + REPO + ' repository.')
    parser.add_argument('--apply', action='store_true', help='apply the reviewed settings after all preflight checks pass')
    options = parser.parse_args()
    try:
        run(options.apply)
    except (SetupError, KeyError, TypeError, ValueError) as error:
        print('STOP: ' + str(error), file=sys.stderr)
        if options.apply:
            print('Requests are not transactional. If an update was reported above, it may already be applied; inspect and rerun.', file=sys.stderr)
        sys.exit(1)
PY
