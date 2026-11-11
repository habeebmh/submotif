# Submotif website and publication

The static product site is in `site/`. It has no package install or build step. For a local preview, run `python3 -m http.server 8080 --directory site --bind 127.0.0.1` and open `http://127.0.0.1:8080/`.

- Product: **SUBMOTIF**, by Habeeb. Intended custom domain: **submotif.com**.
- Public source destination: [habeebmh/submotif](https://github.com/habeebmh/submotif).
- Firebase site: [submotif.web.app](https://submotif.web.app).
- Original software license: **AGPL-3.0-only**. Official downloads will be free; forks, changes, and sales remain permitted under the license's conditions.
- Release packaging and publication remain **on hold for review**. No release archive has been generated for version 1.2.2. `releaseReady: false` keeps the download action disabled; its eventual destination is always [GitHub's latest release](https://github.com/habeebmh/submotif/releases/latest).
- The optional [Instagram follow](https://www.instagram.com/habeeeeeeeeeeeeeeeb/) is a thank-you, never an access requirement.

## Listening review

The original ten-per-style gallery is at `/review/`. It stores choices only in the visitor's browser. The owner selected Deep House 01, Rolling 06, Offbeat 01, Acid 03, Disco 01, and Funk 02. The exact original batch is pinned by fingerprint in `docs/showcase-selections.json` and the site configuration.

The selected examples page is `/selected-examples.html`. Acid uses the owner's requested Serum preset. The owner approved the version raised by two octaves (+24 semitones). The main showcase activates only when all six exact recordings are approved and their provenance agrees. An octave adjustment must be recorded explicitly, with the displayed and exported MIDI matching the recording. Generated patterns in the interactive demo always come from a fresh algorithm run, independently of the fixed showcase examples.

## Hosting

`firebase.json` deploys only `site/` to the named Hosting site `submotif`. `.firebaserc` points to the newly created backing project `lowweave`; that internal ID predates the final product rename. The public address uses only Submotif. No Analytics, authentication, database, Functions, billing upgrade, or private credential belongs in this setup.

The deployment excludes hidden files, browser profiles, screenshots, internal Markdown plans, and staging folders. Headers restrict framing, device permissions, cross-origin resources, and referrer information. The preview and listening pages remain excluded from search indexing pending launch.

Deploy only Hosting using the Firebase connector, or the official CLI:

```sh
npx -y firebase-tools@latest deploy --only hosting --project lowweave
```

After deploying, verify the live page, resource URLs, security headers, legal links, MIDI exports, playback, and disabled release state. A successful local build does not establish a successful deployment. Repository approval settings are described in [GITHUB_SETUP.md](GITHUB_SETUP.md); native/host testing limits are in [VALIDATION.md](VALIDATION.md). The website design and verification record is in `site/SITE.md`.

## Legal pages

The source documents in `docs/legal/` render to `site/legal/` with:

```sh
python3 scripts/build-legal-pages.py
```

The renderer verifies the exact GNU AGPLv3 text and includes local copies of the license, notices, and contribution guide. Website terms do not narrow software license rights. No governing jurisdiction has been invented; these documents do not claim universal enforceability.

## Domain

On November 11, 2026 at 06:39:48 UTC, the authoritative [Verisign .com RDAP endpoint](https://rdap.verisign.com/com/v1/domain/submotif.com) returned HTTP 404: no registration record. The spelling contains no consecutive repeated characters. This check is evidence of an unregistered name at that time, not a purchase, reserved price, or trademark clearance. A basic search found no exact-name plugin; the term also occurs in music theory and historical event names.

The owner will purchase the domain. Recheck with the registrar before buying. Once owned, use Firebase Hosting's Add custom domain flow and the DNS records it actually supplies. Preserve unrelated mail and DNS records. Verify ownership, certificate provisioning, and HTTPS before changing canonical URLs. See [Firebase's custom domain instructions](https://firebase.google.com/docs/hosting/custom-domain).
