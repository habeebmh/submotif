# Submotif documentation

- [Contribution guide](CONTRIBUTING.md): code layout, checks, and pull requests.
- [Music theory](MUSIC_THEORY.md): musical rationale and primary sources.
- [Validation](VALIDATION.md): completed checks and remaining host testing.
- [Website guide](WEBSITE.md): site configuration, recordings, and design verification.
- [Hosting and launch](WEBSITE_LAUNCH.md): Firebase, domain, and publication status.
- [GitHub setup](GITHUB_SETUP.md): owner approval and repository controls.
- [Security](SECURITY.md): private vulnerability reporting.
- [Third-party notices](THIRD_PARTY_NOTICES.md): dependency and asset licenses.
- [Legal documents](legal/terms.md): terms, [liability](legal/liability.md), [privacy](legal/privacy.md), and [license guide](legal/license-guide.md).
- `research/`: source-backed musical research and immutable measurement records.
- `design/`: interface design records.
- [Showcase selections](showcase-selections.json): approved examples and recording provenance.

The repository is divided into `plugin/`, `web/`, and `docs/`. GitHub automation remains in `.github/`; the project README, full license, and Firebase configuration remain at the root.

Historical audition manifests and research snapshots retain their original source paths and hashes. The current equivalents are `Source/PatternEngine.*` → `plugin/src/PatternEngine.*`, `max/undertow-core.js` → `plugin/max/undertow-core.js`, and `scripts/render_showcase.py` → `web/scripts/render_showcase.py`. Moving these files does not regenerate the approved MIDI or recordings.
