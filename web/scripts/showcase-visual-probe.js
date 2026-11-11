(async () => {
  // Execute with the design-taste consistency checker's --probe option.
  const approvalURL = new URL('assets/audio/approved-manifest.json', document.baseURI);
  const approval = await (await fetch(approvalURL)).json();
  const buttons = [...document.querySelectorAll('.recording-choices button')];
  const assert = (value, message) => { if (!value) throw new Error(message); };
  assert(buttons.length === 6, 'All six approved styles must be available');
  assert(document.querySelectorAll('.recording-detail audio').length === 1, 'One shared audio player');
  const results = [];
  for (const button of buttons) {
    button.click();
    const name = button.getAttribute('aria-label');
    const record = approval.selections.find(item => item.style === name);
    const player = document.querySelector('.recording-detail audio');
    const download = document.querySelector('.recording-detail > .link');
    const wave = document.querySelector('.recording-wave');
    await wave.decode();
    assert(record, `Unknown selection: ${name}`);
    assert(player.src === new URL(record.audio.mix.path, approvalURL).href, `${name}: audio mismatch`);
    assert(download.href === new URL(record.midi.path, approvalURL).href, `${name}: MIDI mismatch`);
    assert(button.getAttribute('aria-pressed') === 'true', `${name}: selection state`);
    assert(buttons.filter(item => item.getAttribute('aria-pressed') === 'true').length === 1, 'Only one selection');
    assert(document.querySelector('.recording-title').textContent === name, `${name}: title`);
    assert(wave.src.endsWith(`/${record.id.split('-')[0]}-wave.svg`), `${name}: waveform mismatch`);
    assert(wave.naturalWidth > 0, `${name}: missing waveform`);
    assert(document.querySelector('.showcase-roll').getBoundingClientRect().height >= 140, `${name}: hidden notes`);
    if (name === 'Acid') {
      assert(record.transformation.semitones === 24, 'Acid must retain the approved +24 transpose');
      assert(document.querySelector('.recording-head').textContent.includes('+2 octaves'), 'Acid transpose label');
      const pitches = [...document.querySelectorAll('[data-midi-note]')].map(note => +note.dataset.midiNote);
      assert(JSON.stringify(pitches) === JSON.stringify(record.pattern.notes.map(note => note.pitch)), 'Acid pitch diagram mismatch');
    }
    results.push({ style: name, midi: download.download, waveformLoaded: true });
  }
  buttons[0].click();
  const brokenImages = [...document.images].filter(img => img.complete && !img.naturalWidth).map(img => img.src);
  assert(!brokenImages.length, `Broken images: ${brokenImages.join(', ')}`);
  assert(!document.querySelector('canvas'), 'No canvas renderer should be mounted');
  assert(document.documentElement.scrollWidth <= innerWidth, 'Horizontal overflow');
  return { pass: true, viewport: [innerWidth, innerHeight], selections: results, canvasCount: 0 };
})()
