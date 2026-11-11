# GitHub repository setup

The public repository is **[habeebmh/submotif](https://github.com/habeebmh/submotif)**. `scripts/setup-github.sh` configures an existing repository; it does not create one, authenticate, upload source, change visibility, merge pull requests, or publish releases.

## Verified current setup

On November 11, 2026, main-branch protection was applied and read back successfully after Portable checks passed on the published source. Community PRs require one approving Code Owner review from `@habeebmh`, passing up-to-date Portable checks, and resolved conversations. Stale approvals are dismissed; force pushes and branch deletion are disabled. Issues and private vulnerability reporting are enabled. Owner/admin bypass remains available as explained below. The repository remains open to forks and pull requests.

## Before applying settings

1. Resolve the project license and finish the public-source review. Create the public repository under `habeebmh` and publish the reviewed initial source to `main` separately.
2. Ensure `main` is the default branch, `.github/CODEOWNERS` assigns all files solely to `@habeebmh`, and `.github/workflows/ci.yml` is present.
3. Allow the **Checks** workflow to finish successfully on the current `main` commit. Its job must be named **Portable checks**. Keep that job name unique across workflows.
4. Install Python 3 and GitHub CLI, and authenticate the CLI separately as `habeebmh` with repository administration access. Never put a token in this script or repository.

Preview the exact changes:

```sh
./scripts/setup-github.sh
```

Apply them after reviewing the plan:

```sh
./scripts/setup-github.sh --apply
```

The default is read-only. Both modes verify the account, repository ownership, public visibility, current `main` commit, committed CODEOWNERS validity, and a successful run of the actual CI workflow. Missing `main`, missing or failing CI, unexpected protection fields, and conflicting rules stop setup before writes.

## What it configures

- Enables public Issues. A public repository accepts forks and pull requests without adding contributors as collaborators; the API's `allow_forking` setting concerns private forks and is left alone.
- Requires pull requests and Code Owner review. Since `@habeebmh` owns every path, community changes require that owner's review.
- Requires at least one approving review, dismisses stale approvals after changes, and requires conversations to be resolved.
- Requires **Portable checks**, bound to the GitHub Actions app that supplied the verified result, with the branch up to date before merging.
- Disables force pushes and deletion for the protected `main` branch.

An existing higher review count and unrelated status checks, linear-history settings, and signature requirement are retained. Repository descriptions, visibility, merge methods, auto-merge settings, Actions permissions, secrets, and other branches are untouched. Active rulesets, special bypass/review-dismissal allowances, existing administrator enforcement, or a locked branch need manual review; the script never silently weakens or removes them. It rechecks the observed state before writing and verifies the result afterward. API changes are not atomic: if a later request fails, an earlier update can remain in place. Rerunning is safe and skips settings already applied.

## The sole-owner limitation

GitHub does not let an author [approve their own pull request](https://docs.github.com/en/pull-requests/how-tos/review-pull-requests/reviewing-proposed-changes-in-a-pull-request). Requiring the sole owner to approve every PR while also applying that requirement to the owner's own work would block normal maintenance. This setup retains administrator bypass (`enforce_admins: false`); the owner can maintain the repository and merge their own work administratively. Other administrators, if granted access later, also gain that bypass. It is an intentional exception, not an absolute guarantee that every merge received an approval. Keep admin access limited and use ordinary reviewed PRs for community contributions. See [GitHub's branch protection behavior](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-protected-branches/about-protected-branches).

The script checks remote state each time rather than relying on a saved local configuration. A new commit on `main` must complete CI before setup can be rerun. If protections are changed independently, preview again before applying.

## Finish repository settings

Enable **Private vulnerability reporting** in GitHub's security settings, then update `SECURITY.md` to reflect the enabled channel. Review Actions' fork-approval policy; CI uses `pull_request`, read-only token permissions, and no secrets. Do not switch it to `pull_request_target` to run untrusted contributor code. No deployment or release workflow is installed by this setup.

Check a community pull request in the GitHub interface before announcing that approval rules are active. Confirm the owner review, Portable checks, and unresolved-conversation requirements. Keep the release link at `https://github.com/habeebmh/submotif/releases/latest`; it becomes a usable download destination after the first public release exists.

Implementation references: [branch protection REST API](https://docs.github.com/en/rest/branches/branch-protection?apiVersion=2026-03-10#update-branch-protection), [repository settings API](https://docs.github.com/en/rest/repos/repos?apiVersion=2026-03-10#update-a-repository), [workflow runs API](https://docs.github.com/en/rest/actions/workflow-runs?apiVersion=2026-03-10#list-workflow-runs-for-a-workflow), and [GitHub CLI API requests](https://cli.github.com/manual/gh_api).
