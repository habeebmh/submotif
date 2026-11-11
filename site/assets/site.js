/* The preview uses the actual portable engine; no saved pattern bank. */
(() => {
 'use strict';
 const config=window.UNDERTOW_SITE||{};
 const safeExternal=u=>{try{const url=new URL(u);return url.protocol==='https:'?url.href:null}catch{return null}};
 const repo=safeExternal(config.repository);
 if(repo){document.querySelectorAll('[data-repo]').forEach(a=>a.href=repo);document.querySelectorAll('[data-repo-path]').forEach(a=>a.href=repo+a.dataset.repoPath)}
 const instagram=safeExternal(config.instagram);
 if(instagram){document.querySelectorAll('[data-instagram]').forEach(a=>{a.href=instagram;a.rel='noopener noreferrer'});document.querySelectorAll('[data-instagram-box]').forEach(e=>e.hidden=false)}
 const release=safeExternal(config.releaseUrl);
 if(config.releaseReady&&release){document.querySelectorAll('[data-download],[data-download-final]').forEach(a=>a.href=release);document.querySelector('#release-status').textContent='Free download · Apple Silicon · macOS 11+'}
 else{const final=document.querySelector('[data-download-final]');final.setAttribute('aria-disabled','true');final.removeAttribute('href');final.setAttribute('tabindex','0');final.setAttribute('title','The first public release is not available yet.');final.addEventListener('click',e=>e.preventDefault())}
 const names=['Deep House','Rolling','Offbeat','Acid','Disco','Funk'];
 const descriptions=['A spacious motif with sustained notes and restrained movement.','A repeating motif, grounded in the root, with small rhythmic changes.','Bass notes between the main beats, leaving space for the kick.','Short phrases with accents and occasional close passing notes.','Octave motion and chord tones shape a more melodic line.','Syncopation, rests and answering phrases make room around the beat.'];
 const C=window.UndertowCore;if(!C){document.querySelector('#generator-message').textContent='The generator could not load. Reload the page to try again.';return}
 let settings=C.defaults(),pattern,playing=false,pendingStart=false,playbackGeneration=0,audio,master,timer,frame,startAt,nodes=[];settings.style=1;
 const featured=new Map(),showcasePlayers=[];
 for(const item of config.featured||[]){if(item.approved!==true||!names.includes(item.style)||!C.valid(item.pattern)||!/^([a-z]+)-[0-9]{2}$/.test(item.id||'')||typeof item.audio!=='string'||!item.audio.trim())continue;try{const url=new URL(item.audio,document.baseURI);if(url.origin!==location.origin||!/^https?:$/.test(url.protocol))continue;if(featured.has(names.indexOf(item.style)))throw new Error('duplicate style');featured.set(names.indexOf(item.style),{...item,audio:url.href})}catch{}}
 function stopShowcase(){showcasePlayers.forEach(player=>player.pause())}
 const roll=document.querySelector('#roll-notes'), play=document.querySelector('#preview-play'), head=document.querySelector('.playhead'), message=document.querySelector('#generator-message');
 function seed(){const b=new Uint32Array(1);if(globalThis.crypto?.getRandomValues){crypto.getRandomValues(b);return b[0]||1}return (Date.now()^Math.floor(Math.random()*0xffffffff))>>>0}
 function draw(){
  roll.replaceChildren();const low=Math.min(34,...pattern.notes.map(n=>n.pitch))-1,high=Math.max(46,...pattern.notes.map(n=>n.pitch))+1;
  for(const n of pattern.notes){const el=document.createElement('span');el.className='note';el.style.left=`${n.start/pattern.lengthBeats*100}%`;el.style.width=`${Math.max(.3,n.duration/pattern.lengthBeats*100)}%`;el.style.top=`${12+(high-n.pitch)/(high-low)*76}%`;el.title=`MIDI note ${n.pitch}, beat ${(n.start+1).toFixed(2)}`;roll.append(el)}
  document.querySelector('#pattern-label').textContent=names[settings.style];document.querySelector('#note-count').textContent=`${pattern.notes.length} notes`;document.querySelector('#style-description').textContent=descriptions[settings.style];document.querySelector('.generator__meta').textContent=`C minor / ${pattern.lengthBeats/4} ${pattern.lengthBeats===4?'bar':'bars'} / 126 BPM`;document.querySelector('.generator__status').textContent='Browser preview';document.querySelector('#piano-roll').setAttribute('aria-label',`${names[settings.style]} bassline in C minor with ${pattern.notes.length} MIDI notes`)
 }
 function stop(){playbackGeneration++;pendingStart=false;playing=false;clearTimeout(timer);cancelAnimationFrame(frame);nodes.forEach(n=>{try{n.stop()}catch{}});nodes=[];play.textContent='Play preview';play.removeAttribute('aria-busy');play.setAttribute('aria-pressed','false');head.hidden=true}
 function generate(){stopShowcase();stop();pattern=C.generate(settings,seed(),null,false);if(!C.valid(pattern))throw new Error('Invalid generator output');draw();message.textContent=`Generated ${names[settings.style]} with ${pattern.notes.length} notes.`}
 function note(n,at){
  const duration=n.duration*60/126,osc=audio.createOscillator(),sub=audio.createOscillator(),gain=audio.createGain(),filter=audio.createBiquadFilter();
  const hz=440*2**((n.pitch-69)/12),peak=.16*n.velocity/127,sustain=.07*n.velocity/127,attack=Math.min(.008,duration*.2),decay=Math.min(.09,duration*.65),releaseAt=at+duration;
  osc.type='sawtooth';osc.frequency.value=hz;sub.type='sine';sub.frequency.value=hz;filter.type='lowpass';filter.frequency.setValueAtTime(2000,at);filter.frequency.exponentialRampToValueAtTime(230,at+Math.min(.11,duration*.8));filter.Q.value=.9;
  gain.gain.setValueAtTime(.0001,at);gain.gain.exponentialRampToValueAtTime(peak,at+attack);gain.gain.exponentialRampToValueAtTime(sustain,at+decay);gain.gain.setValueAtTime(sustain,releaseAt);gain.gain.exponentialRampToValueAtTime(.0001,releaseAt+.035);
  osc.connect(filter);sub.connect(filter);filter.connect(gain);gain.connect(master);osc.start(at);sub.start(at);osc.stop(releaseAt+.045);sub.stop(releaseAt+.045);nodes.push(osc,sub);osc.onended=()=>{osc.disconnect();sub.disconnect();filter.disconnect();gain.disconnect()}
 }
 async function preview(){
  if(playing||pendingStart){stop();return}
  stopShowcase();const token=++playbackGeneration;pendingStart=true;play.textContent='Starting audio';play.setAttribute('aria-busy','true');
  try{
   audio||=(new (window.AudioContext||window.webkitAudioContext)());if(!master){master=audio.createGain();master.gain.value=.65;master.connect(audio.destination)}
   await audio.resume();
   if(token!==playbackGeneration||document.hidden){if(token===playbackGeneration)stop();return}
   pendingStart=false;playing=true;play.removeAttribute('aria-busy');play.textContent='Stop preview';play.setAttribute('aria-pressed','true');startAt=audio.currentTime+.04;const bar=pattern.lengthBeats*60/126;
   for(let repeat=0;repeat<4;repeat++)pattern.notes.forEach(n=>note(n,startAt+repeat*bar+n.start*60/126));head.hidden=false;
   const tick=()=>{if(!playing)return;const progress=Math.max(0,audio.currentTime-startAt)%bar/bar;head.style.left=`${progress*100}%`;frame=requestAnimationFrame(tick)};tick();timer=setTimeout(stop,(bar*4+.2)*1000)
  }catch(e){if(token!==playbackGeneration)return;stop();message.textContent='Audio could not start. Try Play preview again.'}
 }
 document.querySelectorAll('[data-style]').forEach(button=>button.addEventListener('click',()=>{settings.style=Number(button.dataset.style);document.querySelectorAll('[data-style]').forEach(b=>b.setAttribute('aria-pressed',String(b===button)));generate()}));
 document.querySelector('#generate').addEventListener('click',generate);play.addEventListener('click',preview);
 document.querySelector('#export-midi').addEventListener('click',()=>{const bytes=new Uint8Array(C.midiFile(pattern));const url=URL.createObjectURL(new Blob([bytes],{type:'audio/midi'}));const a=document.createElement('a');a.href=url;a.download=`Submotif-${names[settings.style].toLowerCase().replaceAll(' ','-')}-C-minor-${pattern.seed}.mid`;a.click();setTimeout(()=>URL.revokeObjectURL(url),1000);message.textContent='MIDI file saved. Drag it into an instrument track in your DAW.'});
  document.addEventListener('visibilitychange',()=>{if(document.hidden){stop();stopShowcase()}});window.addEventListener('pagehide',()=>{stop();stopShowcase()});
 function showcaseRoll(pattern,style){
  const ns='http://www.w3.org/2000/svg',svg=document.createElementNS(ns,'svg');svg.setAttribute('viewBox','0 0 320 64');svg.classList.add('showcase-roll');svg.setAttribute('role','img');svg.setAttribute('aria-label',`${style}: ${pattern.notes.length} exact selected MIDI notes`);
  const add=(tag,attrs)=>{const el=document.createElementNS(ns,tag);Object.entries(attrs).forEach(([k,v])=>el.setAttribute(k,v));svg.append(el)};
  for(let i=0;i<=32;i++)add('line',{x1:i*10,y1:0,x2:i*10,y2:64,stroke:i%4?'#28322e':'#3f4c43','stroke-width':'.5'});
  const low=Math.min(...pattern.notes.map(n=>n.pitch))-1,high=Math.max(...pattern.notes.map(n=>n.pitch))+1;
  pattern.notes.forEach(n=>add('rect',{x:n.start/pattern.lengthBeats*320,y:6+(high-n.pitch)/(high-low)*46,width:Math.max(1.5,n.duration/pattern.lengthBeats*320),height:4,rx:1,fill:'#b9dd65'}));return svg;
 }
 // Only a complete owner-approved six-style set becomes a public showcase.
 async function buildShowcase(){
  if(!Array.isArray(config.featured)||config.featured.length!==6||featured.size!==6)return;
  try{
   const manifestURL=new URL(config.showcaseManifest||'review/manifest.json',document.baseURI);if(manifestURL.origin!==location.origin)return;
   const response=await fetch(manifestURL);if(!response.ok)return;const manifest=await response.json();
   if(!/^[0-9a-f]{64}$/.test(config.showcaseBatchFingerprint||'')||manifest.batchFingerprint!==config.showcaseBatchFingerprint||manifest.batchFingerprintAlgorithm!=='sha256-canonical-json-v1')return;
   if(manifest.version!==1||manifest.bpm!==126||manifest.key!=='C natural minor'||manifest.bars!==2||!Array.isArray(manifest.candidates))return;
   const approvalURL=new URL(config.showcaseApprovalManifest||'assets/audio/approved-manifest.json',document.baseURI);if(approvalURL.origin!==location.origin)return;
   const approvalResponse=await fetch(approvalURL);if(!approvalResponse.ok)return;const approval=await approvalResponse.json();
   if(approval.version!==1||approval.sourceBatchFingerprint!==config.showcaseBatchFingerprint||!Array.isArray(approval.selections)||approval.selections.length!==6)return;
   for(const item of featured.values()){
    const candidate=manifest.candidates.find(c=>c.candidateId===item.id),approved=approval.selections.filter(a=>a.id===item.id);
    if(!candidate||candidate.styleName!==item.style||approved.length!==1)return;
    const record=approved[0],render=record.render;
    if(record.style!==item.style||record.midiApproved!==true||record.audioApproved!==true||!render||render.status!=='approved'||!['original-audition','serum'].includes(render.kind))return;
    if(!/^[0-9a-f]{64}$/.test(candidate.midiSha256||'')||!/^[0-9a-f]{64}$/.test(record.midi?.sha256||'')||render.sourceMidiSha256!==record.midi.sha256)return;
    const exact=JSON.parse(JSON.stringify(candidate.pattern)),transform=record.transformation;
    if(transform){
     if(transform.kind!=='transpose'||transform.approved!==true||!Number.isInteger(transform.semitones)||transform.semitones===0||Math.abs(transform.semitones)>24||transform.semitones%12!==0||transform.sourceMidiSha256!==candidate.midiSha256)return;
     exact.notes.forEach(note=>note.pitch+=transform.semitones);
     if(JSON.stringify(record.pattern)!==JSON.stringify(exact))return;
    }else if(record.midi.sha256!==candidate.midiSha256)return;
    if(!C.valid(exact)||JSON.stringify(item.pattern)!==JSON.stringify(exact))return;
    const midiURL=new URL(record.midi.path,approvalURL);if(midiURL.origin!==location.origin||!/^https?:$/.test(midiURL.protocol))return;
    item.midi=midiURL.href;item.transpose=transform?.semitones||0;item.variantId=record.variantId||item.id;
    if(render.kind==='serum'&&(typeof render.preset!=='string'||!render.preset.trim()))return;
    const paths=Object.values(record.audio||{}).filter(a=>typeof a?.path==='string'&&/^[0-9a-f]{64}$/.test(a.sha256||'')).map(a=>new URL(a.path,approvalURL)).filter(url=>url.origin===location.origin).map(url=>url.href);
    if(!paths.includes(item.audio))return;
   }
  }catch{return}
  const list=document.querySelector('#showcase-list');
  for(let style=0;style<6;style++){
   const item=featured.get(style),card=document.createElement('article'),title=document.createElement('h3'),details=document.createElement('p'),player=document.createElement('audio'),download=document.createElement('a');
   card.className='showcase-example ui ui-showcase';title.textContent=names[style];details.textContent=`C minor · ${item.pattern.lengthBeats/4} bars · 126 BPM${item.transpose?` · ${item.transpose>0?'+':''}${item.transpose/12} ${Math.abs(item.transpose)===12?'octave':'octaves'}`:''}`;player.controls=true;player.preload='none';player.src=item.audio;player.setAttribute('aria-label',`${names[style]} bassline`);
   download.href=item.midi;download.download=`Submotif-${names[style].toLowerCase().replaceAll(' ','-')}-${item.variantId}.mid`;download.textContent='Save this MIDI';download.className='link';
   player.addEventListener('play',()=>{stop();showcasePlayers.forEach(other=>{if(other!==player)other.pause()});if(document.hidden)player.pause()});showcasePlayers.push(player);card.append(title,details,showcaseRoll(item.pattern,names[style]),player,download);list.append(card);
  }
  document.querySelector('#showcase').hidden=false;
 }
 buildShowcase();generate();
})();
