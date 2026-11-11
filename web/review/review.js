/* Local listening/selection only. No network requests or automatic approval. */
(() => {
  'use strict';
  const data = window.UNDERTOW_AUDITIONS;
  if (!data || !Array.isArray(data.candidates)) return;
  const $ = id => document.getElementById(id);
  const audio = $('audio');
  if (!/^[a-f0-9]{64}$/.test(data.batchFingerprint || '')) {
    $('feedback').textContent = 'These previews could not be verified. Please reload the page.';
    return;
  }
  const storageKey = 'undertow-auditions-v2:' + data.batchFingerprint;
  const byId = new Map(data.candidates.map(candidate => [candidate.candidateId, candidate]));
  let style = data.styles[0].id, mode = 'mix', current = null, compilationStyle = null;
  let choices = {};
  try {
    const stored = JSON.parse(localStorage.getItem(storageKey) || '{}');
    for (const item of data.styles) if (byId.get(stored[item.id])?.slug === item.id) choices[item.id] = stored[item.id];
  } catch (_) { /* Local storage may be unavailable in private/file browsing. */ }

  function persist() {
    try { localStorage.setItem(storageKey, JSON.stringify(choices)); }
    catch (_) { $('feedback').textContent = 'This browser cannot save choices locally. Copy your picks before closing the page.'; }
  }
  function selection() {
    return {version: 1, status: 'candidate selections — awaiting explicit approval',
      batchFingerprint: data.batchFingerprint, sourceHashes: data.sources, choices: {...choices}};
  }
  function summary() {
    return 'SUBMOTIF — my six bassline picks\nPreview set: ' + data.batchFingerprint + '\n' + data.styles.map(item => {
      const candidate = byId.get(choices[item.id]);
      return `${item.name}: ${candidate ? String(candidate.number).padStart(2, '0') + ' (' + candidate.candidateId + ')' : 'not chosen'}`;
    }).join('\n');
  }
  function refreshChoices() {
    const count = Object.keys(choices).length;
    $('selection-title').textContent = count ? `${count} of 6 styles chosen.` : 'No picks yet.';
    $('picked-list').replaceChildren(...data.styles.map(item => {
      const li = document.createElement('li'), label = document.createElement('span'), value = document.createElement('strong');
      label.textContent = item.name;
      const candidate = byId.get(choices[item.id]);
      value.textContent = candidate ? 'No. ' + String(candidate.number).padStart(2, '0') : 'Not chosen';
      li.append(label, value); return li;
    }));
    $('copy-picks').disabled = count !== 6;
    $('save-picks').disabled = count !== 6;
    $('clear-picks').disabled = count === 0;
    for (const card of document.querySelectorAll('.candidate')) {
      const candidate = byId.get(card.dataset.candidate);
      const selected = choices[candidate.slug] === candidate.candidateId;
      card.classList.toggle('chosen', selected);
      card.querySelector('input').checked = selected;
    }
  }
  function refreshPlaying() {
    const compilation = data.styles.find(item => item.id === compilationStyle);
    const chapter = compilation?.chapters.find(item => audio.currentTime >= item.startSeconds && audio.currentTime < item.endSeconds);
    for (const card of document.querySelectorAll('.candidate')) {
      const active = card.dataset.candidate === current && !audio.paused;
      card.classList.toggle('playing', active || (card.dataset.candidate === chapter?.candidateId && !audio.paused));
      const button = card.querySelector('.play');
      button.textContent = active ? 'Pause' : 'Play';
      button.setAttribute('aria-pressed', String(active));
    }
    $('stop').disabled = !audio.src;
    $('compilation').textContent = compilationStyle === style && !audio.paused ? 'Pause all ten' : 'Play all ten';
    const candidate = byId.get(current);
    if (candidate) $('now-title').textContent = `${candidate.styleName} ${String(candidate.number).padStart(2, '0')} · ${mode === 'mix' ? 'with drums' : 'bass only'}`;
    else if (compilation) $('now-title').textContent = `${compilation.name} ${chapter ? String(chapter.number).padStart(2, '0') + ' of 10' : '· between candidates'} · ${mode === 'mix' ? 'with drums' : 'bass only'}`;
  }
  async function playCandidate(candidate) {
    if (current === candidate.candidateId && !compilationStyle && !audio.paused) {
      audio.pause(); return;
    }
    if (current !== candidate.candidateId || compilationStyle) {
      audio.pause(); audio.src = candidate.files[mode]; current = candidate.candidateId; compilationStyle = null;
    }
    try { await audio.play(); }
    catch (error) { if (error.name !== 'AbortError') $('feedback').textContent = 'Audio could not start. Try the player controls, or download the MIDI to hear it in your DAW.'; }
    refreshPlaying();
  }
  function roll(pattern, title) {
    const ns = 'http://www.w3.org/2000/svg';
    const svg = document.createElementNS(ns, 'svg');
    svg.setAttribute('viewBox', '0 0 480 84'); svg.setAttribute('class', 'roll');
    svg.setAttribute('role', 'img'); svg.setAttribute('aria-label', title + ' piano roll');
    for (let beat = 0; beat <= 8; beat++) {
      const line = document.createElementNS(ns, 'path');
      line.setAttribute('d', `M ${beat * 60} 0 V 84`);
      line.setAttribute('stroke', beat % 4 === 0 ? '#bdc5b4' : '#dde3d5');
      line.setAttribute('stroke-width', '1'); svg.append(line);
    }
    for (const note of pattern.notes) {
      const rect = document.createElementNS(ns, 'rect');
      rect.setAttribute('x', String(note.start * 60)); rect.setAttribute('y', String(72 - (note.pitch - 36) * 5));
      rect.setAttribute('width', String(Math.max(2, note.duration * 60))); rect.setAttribute('height', '5');
      rect.setAttribute('rx', '1.5'); rect.setAttribute('fill', '#5c7b2d');
      rect.setAttribute('opacity', String(.5 + note.velocity / 254)); svg.append(rect);
    }
    return svg;
  }
  function renderStyle() {
    const item = data.styles.find(value => value.id === style);
    $('style-title').textContent = item.name;
    for (const button of $('styles').children) button.setAttribute('aria-pressed', String(button.dataset.style === style));
    const cards = data.candidates.filter(value => value.slug === style).map(candidate => {
      const card = document.createElement('article'); card.className = 'candidate'; card.dataset.candidate = candidate.candidateId;
      const top = document.createElement('div'); top.className = 'candidate-head';
      const heading = document.createElement('h3'); heading.textContent = 'No. ' + String(candidate.number).padStart(2, '0');
      const count = document.createElement('span'); count.className = 'note-count'; count.textContent = candidate.noteCount + ' notes';
      top.append(heading, count);
      const controls = document.createElement('div'); controls.className = 'candidate-actions';
      const play = document.createElement('button'); play.type = 'button'; play.className = 'quiet play'; play.textContent = 'Play';
      play.setAttribute('aria-label', `Play or pause ${candidate.styleName} candidate ${candidate.number}`);
      play.onclick = () => playCandidate(candidate);
      const choose = document.createElement('label'); choose.className = 'choose';
      const radio = document.createElement('input'); radio.type = 'radio'; radio.name = 'pick-' + style; radio.value = candidate.candidateId;
      radio.setAttribute('aria-label', `Choose ${candidate.styleName} candidate ${candidate.number}`);
      radio.onchange = () => { choices[style] = candidate.candidateId; persist(); refreshChoices(); };
      choose.append(radio, document.createTextNode('Choose ' + String(candidate.number).padStart(2, '0')));
      const midi = document.createElement('a'); midi.className = 'midi'; midi.href = candidate.files.midi; midi.download = '';
      midi.textContent = 'MIDI'; midi.setAttribute('aria-label', `Download ${candidate.styleName} candidate ${candidate.number} MIDI`);
      controls.append(play, choose, midi); card.append(top, roll(candidate.pattern, `${candidate.styleName} ${candidate.number}`), controls);
      return card;
    });
    $('candidates').replaceChildren(...cards); refreshChoices(); refreshPlaying();
  }
  for (const [index, item] of data.styles.entries()) {
    const button = document.createElement('button'); button.type = 'button'; button.dataset.style = item.id; button.textContent = item.name;
    button.onclick = () => { style = item.id; renderStyle(); };
    button.onkeydown = event => {
      if (event.key !== 'ArrowLeft' && event.key !== 'ArrowRight') return;
      event.preventDefault();
      const next = (index + (event.key === 'ArrowRight' ? 1 : -1) + data.styles.length) % data.styles.length;
      $('styles').children[next].focus(); $('styles').children[next].click();
    };
    $('styles').append(button);
  }
  for (const radio of document.querySelectorAll('input[name=mode]')) radio.onchange = async () => {
    mode = radio.value;
    if (current || compilationStyle) {
      const wasPlaying = !audio.paused, time = audio.currentTime;
      audio.pause();
      const compilation = data.styles.find(item => item.id === compilationStyle);
      audio.src = compilation ? compilation[mode === 'mix' ? 'compilation' : 'compilationSolo'] : byId.get(current).files[mode];
      audio.currentTime = time;
      if (wasPlaying) try { await audio.play(); } catch (_) { /* Native player remains available. */ }
    }
    refreshPlaying();
  };
  $('compilation').onclick = async () => {
    if (compilationStyle === style && !audio.paused) { audio.pause(); return; }
    if (compilationStyle !== style) {
      const compilation = data.styles.find(item => item.id === style);
      audio.pause(); audio.src = compilation[mode === 'mix' ? 'compilation' : 'compilationSolo']; compilationStyle = style; current = null;
    }
    try { await audio.play(); } catch (error) { if (error.name !== 'AbortError') $('feedback').textContent = 'Use the player controls to start the compilation.'; }
    refreshPlaying();
  };
  $('stop').onclick = () => { audio.pause(); audio.currentTime = 0; refreshPlaying(); };
  for (const event of ['play', 'pause', 'ended']) audio.addEventListener(event, refreshPlaying);
  audio.addEventListener('timeupdate', () => { if (compilationStyle) refreshPlaying(); });
  audio.addEventListener('error', () => { $('feedback').textContent = 'This preview could not be loaded. Try another preview, or download the MIDI to hear it in your DAW.'; });
  $('copy-picks').onclick = async () => {
    if (Object.keys(choices).length !== 6) return;
    try { await navigator.clipboard.writeText(summary()); $('feedback').textContent = 'Picks copied. Send them to approve your six examples.'; $('copy-fallback').hidden = true; }
    catch (_) { $('copy-fallback').value = summary(); $('copy-fallback').hidden = false; $('copy-fallback').focus(); $('copy-fallback').select(); $('feedback').textContent = 'Copy the selected text below and send your six picks.'; }
  };
  $('save-picks').onclick = () => {
    if (Object.keys(choices).length !== 6) return;
    const url = URL.createObjectURL(new Blob([JSON.stringify(selection(), null, 2) + '\n'], {type: 'application/json'}));
    const link = document.createElement('a'); link.href = url; link.download = 'submotif-my-six-picks.json'; link.click();
    setTimeout(() => URL.revokeObjectURL(url), 1000);
    $('feedback').textContent = 'Selection file saved. These picks are still awaiting your explicit approval.';
  };
  $('clear-picks').onclick = () => { choices = {}; persist(); refreshChoices(); $('feedback').textContent = 'Picks cleared.'; $('copy-fallback').hidden = true; };
  renderStyle();
})();
