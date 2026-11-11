(async () => {
  // Run with the shipped consistency checker --probe, on the old and fixed pages.
  const wait = ms => new Promise(resolve => setTimeout(resolve, ms));
  const frame = () => new Promise(resolve => requestAnimationFrame(resolve));
  const report = { viewport: [innerWidth, innerHeight], phases: [] };
  const ground = document.querySelector('.product-ground');
  const controller = ground?.__fx;
  const originals = [];
  let draws = 0, resets = 0, clearedAfterDraw = 0, blankFrames = 0, lastOperation = '';
  const raf = window.requestAnimationFrame;
  window.requestAnimationFrame = callback => raf.call(window, time => {
    const before = resets;
    callback(time);
    if (resets > before && lastOperation === 'clear') {
      const canvas = ground?.querySelector('canvas');
      const gl = canvas?.getContext('webgl2') || canvas?.getContext('webgl');
      if (gl) {
        const pixel = new Uint8Array(4);
        gl.readPixels(0, 0, 1, 1, gl.RGBA, gl.UNSIGNED_BYTE, pixel);
        if (pixel[0] === 0 && pixel[1] === 0 && pixel[2] === 0) blankFrames++;
      }
    }
  });
  originals.push(() => { window.requestAnimationFrame = raf; });
  for (const Context of [window.WebGLRenderingContext, window.WebGL2RenderingContext]) {
    if (!Context) continue;
    const original = Context.prototype.drawArrays;
    Context.prototype.drawArrays = function (...args) {
      draws++;
      lastOperation = 'draw';
      return original.apply(this, args);
    };
    originals.push(() => { Context.prototype.drawArrays = original; });
  }
  for (const property of ['width', 'height']) {
    const descriptor = Object.getOwnPropertyDescriptor(HTMLCanvasElement.prototype, property);
    Object.defineProperty(HTMLCanvasElement.prototype, property, {
      ...descriptor,
      set(value) {
        if (this.classList.contains('fx-canvas')) {
          resets++;
          if (lastOperation === 'draw') clearedAfterDraw++;
          lastOperation = 'clear';
        }
        descriptor.set.call(this, value);
      },
    });
    originals.push(() => Object.defineProperty(HTMLCanvasElement.prototype, property, descriptor));
  }
  async function measure(name, scroll = false, duration = 4000, busyMs = 0) {
    const gaps = [], start = performance.now(), d = draws, r = resets, c = clearedAfterDraw, b = blankFrames;
    let maxScrollY = 0;
    let previous = await frame();
    while (performance.now() - start < duration) {
      if (scroll) {
        const progress = (performance.now() - start) / duration;
        const distance = document.documentElement.scrollHeight - innerHeight;
        scrollTo(0, Math.round(distance * (1 - Math.cos(progress * Math.PI * 4)) / 2));
      }
      if (busyMs) { const end = performance.now() + busyMs; while (performance.now() < end) {} }
      const next = await frame();
      maxScrollY = Math.max(maxScrollY, scrollY);
      gaps.push(next - previous); previous = next;
    }
    gaps.sort((a, b) => a - b);
    report.phases.push({ name, frames: gaps.length, medianMs: +gaps[Math.floor(gaps.length / 2)].toFixed(1),
      p95Ms: +gaps[Math.floor(gaps.length * .95)].toFixed(1), maxMs: +gaps.at(-1).toFixed(1),
      gapsOver50Ms: gaps.filter(x => x > 50).length, draws: draws - d, canvasResets: resets - r,
      clearsFollowingDraw: clearedAfterDraw - c, blankFrames: blankFrames - b, maxScrollY });
  }
  try {
    scrollTo(0, 0); await wait(250);
    report.effectBefore = controller?.info || null;
    await measure('hero');
    await measure('scroll', true);
    controller?.pause();
    scrollTo(0, 0); await wait(250);
    await measure('hero-paused');
    await measure('scroll-paused', true);
    scrollTo(0, 0); controller?.resume(); await wait(250);
    // Deterministically exercise the renderer's adaptive-resolution path under contention.
    await measure('constrained-hero', false, 8500, 35);
    report.effectAfter = controller?.info || null;
    report.canvasCount = document.querySelectorAll('canvas').length;
    report.effectModules = performance.getEntriesByType('resource').filter(entry => /\/effects\//.test(entry.name)).map(entry => new URL(entry.name).pathname);
    report.overflow = document.documentElement.scrollWidth > innerWidth;
    report.hiddenContent = [...document.querySelectorAll('main h1, main h2, .generator, .recording-window')]
      .filter(el => getComputedStyle(el).visibility === 'hidden' || getComputedStyle(el).opacity === '0').map(el => el.className || el.tagName);
    report.pass = !report.overflow && !report.hiddenContent.length && !report.canvasCount && !report.effectModules.length
      && report.phases.every(phase => phase.blankFrames === 0)
      && report.phases.filter(phase => phase.name.startsWith('scroll')).every(phase => phase.maxScrollY > innerHeight);
  } finally {
    originals.reverse().forEach(restore => restore());
    controller?.resume();
    scrollTo(0, 0);
  }
  return report;
})()
