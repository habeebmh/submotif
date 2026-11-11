/* Approved bassline recordings and their exact MIDI downloads. */
(() => {
 'use strict';
 const config=window.UNDERTOW_SITE||{};
 const safeExternal=u=>{try{const url=new URL(u);return url.protocol==='https:'?url.href:null}catch{return null}};
 const repo=safeExternal(config.repository);
 if(repo){document.querySelectorAll('[data-repo]').forEach(a=>a.href=repo);document.querySelectorAll('[data-repo-path]').forEach(a=>a.href=repo+a.dataset.repoPath)}
 const instagram=safeExternal(config.instagram);
 if(instagram){document.querySelectorAll('[data-instagram]').forEach(a=>{a.href=instagram;a.rel='noopener noreferrer'});document.querySelectorAll('[data-instagram-box]').forEach(e=>e.hidden=false)}
 const release=safeExternal(config.releaseUrl);
 if(config.releaseReady&&release){document.querySelectorAll('[data-download],[data-download-final]').forEach(a=>a.href=release);document.querySelector('#release-status').textContent='Free · macOS 11+ · Apple Silicon + Intel.'}
 else{const final=document.querySelector('[data-download-final]');final.setAttribute('aria-disabled','true');final.removeAttribute('href');final.setAttribute('tabindex','0');final.setAttribute('title','The first public release is not available yet.');final.addEventListener('click',e=>e.preventDefault())}
 const names=['Deep House','Rolling','Offbeat','Acid','Disco','Funk'];
 // The shared core validates the approved MIDI patterns; generation happens in the plugin.
 const C=window.UndertowCore;if(!C)return;
 const featured=new Map(),showcasePlayers=[];
 for(const item of config.featured||[]){if(item.approved!==true||!names.includes(item.style)||!C.valid(item.pattern)||!/^([a-z]+)-[0-9]{2}$/.test(item.id||'')||typeof item.audio!=='string'||!item.audio.trim())continue;try{const url=new URL(item.audio,document.baseURI);if(url.origin!==location.origin||!/^https?:$/.test(url.protocol))continue;if(featured.has(names.indexOf(item.style)))throw new Error('duplicate style');featured.set(names.indexOf(item.style),{...item,audio:url.href})}catch{}}
 function stopShowcase(){showcasePlayers.forEach(player=>player.pause())}
 document.addEventListener('visibilitychange',()=>{if(document.hidden)stopShowcase()});
 window.addEventListener('pagehide',stopShowcase);
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
  const windowEl=document.createElement('div'),choices=document.createElement('div'),detail=document.createElement('div'),heading=document.createElement('div'),title=document.createElement('p'),meta=document.createElement('p'),player=document.createElement('audio'),download=document.createElement('a');
  windowEl.className='recording-window app-frame ui';choices.className='recording-choices';choices.setAttribute('role','group');choices.setAttribute('aria-label','Selected bassline recording');detail.className='recording-detail';heading.className='recording-head';title.className='recording-title';
  const text=document.createElement('div');text.append(title,meta);heading.append(text);player.controls=true;player.preload='none';download.className='link';download.textContent='Save this MIDI';
  let currentRoll;
  function selectRecording(style){
   player.pause();const item=featured.get(style);title.textContent=names[style];meta.textContent=`C minor · 2 bars · 126 BPM${item.transpose?` · ${item.transpose>0?'+':''}${item.transpose/12} ${Math.abs(item.transpose)===12?'octave':'octaves'}`:''}`;
   player.src=item.audio;player.setAttribute('aria-label',names[style]+' bassline');player.load();download.href=item.midi;download.download=`Submotif-${item.variantId}.mid`;
   const next=showcaseRoll(item.pattern,names[style]);if(currentRoll)currentRoll.replaceWith(next);else detail.insertBefore(next,player);currentRoll=next;
   choices.querySelectorAll('button').forEach((button,index)=>button.setAttribute('aria-pressed',String(index===style)));
  }
  for(let style=0;style<6;style++){const button=document.createElement('button');button.type='button';button.textContent=names[style];button.addEventListener('click',()=>selectRecording(style));choices.append(button)}
  player.addEventListener('play',()=>{if(document.hidden)player.pause()});showcasePlayers.push(player);
  detail.append(heading,player,download);windowEl.append(choices,detail);list.append(windowEl);selectRecording(0);
  document.querySelector('#showcase').hidden=false;
 }
 buildShowcase();
})();
