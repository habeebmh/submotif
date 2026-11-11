# Project working preferences

- Make changes directly on `main`. Do not create a topic branch or pull request for owner-directed work unless the user explicitly requests one. External contributors may still submit PRs and issues.
- Releases must have immutable annotated Git tags `vMAJOR.MINOR.PATCH` matching `plugin/CMakeLists.txt`. Never move or overwrite a published release tag.
- Public release packaging/publication is on hold until the user approves it. Development builds, tests and temporary CI artifacts are allowed. Do not create a release tag or publish a release just because a build succeeds.
