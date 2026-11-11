# Website redesign — November 11, 2026

The previous page shipped with an errored finish gate and unresolved findings. Repeated mobile height overrides and a decorative overlay complicated the layout; static screenshots did not adequately verify short phone screens or live scrolling. That was a failure of implementation and verification.

The replacement reduces the page to five sections, shows the actual native editor, keeps the working procedural generator, and uses one recording player with six explicit choices. It removes the extra repository/FAQ screen and keeps contribution, Instagram support and legal links in the footer. Scrolling stays in normal document flow, with a short entrance, quiet hover feedback, one text-highlight moment and reduced-motion support.

The final design gate passed with no waivers. Performance checks reported zero failures, zero layout shift in the measured loads, and no broken requests; font weight in bytes and image oversizing remain advisory warnings. Browser interaction checks covered 320, 390, 640 and 1440 pixels and preserved all six approved recordings, including Acid two octaves higher. No plugin engine, Max device, approved MIDI or audio was changed. Physical iPhone/Safari verification remains outstanding.

Complete reports, retained warnings and desktop/phone captures are in the local `web/.taste-skill/` and `web/shots/` folders, excluded from hosting and version control. Reproduce the design checks with the unmodified `design-taste-frontend` checker, performance and screenshot scripts.

The public website is https://submotif.web.app. Public release packaging remains on hold.
