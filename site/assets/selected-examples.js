(() => {
 'use strict';
 const list=document.querySelector('#selected-list'),status=document.querySelector('#selected-status'),mode=document.querySelector('#selected-mode');
 const expected=['deep-01','rolling-06','offbeat-01','acid-03','disco-01','funk-02'];
 const fingerprint='e9ed6426621b0aa0ac331289525153c9da8b5784aaab23f3d85977a704750e5c';
 const players=[];
 function stop(){players.forEach(({player})=>player.pause())}
 document.addEventListener('visibilitychange',()=>{if(document.hidden)stop()});window.addEventListener('pagehide',stop);
 function localAsset(file,base){const url=new URL(file,base);if(url.origin!==location.origin||!/^https?:$/.test(url.protocol))throw Error('Invalid asset');return url.href}
 function roll(pattern,style){
  const ns='http://www.w3.org/2000/svg',svg=document.createElementNS(ns,'svg');svg.classList.add('selected-roll');svg.setAttribute('viewBox','0 0 480 144');svg.setAttribute('role','img');svg.setAttribute('aria-label',`${style}: exact selected two-bar MIDI pattern, ${pattern.notes.length} notes`);
  const add=(tag,attrs)=>{const el=document.createElementNS(ns,tag);Object.entries(attrs).forEach(([k,v])=>el.setAttribute(k,v));svg.append(el);return el};
  for(let i=0;i<=32;i++)add('line',{x1:12+i*456/32,y1:26,x2:12+i*456/32,y2:132,stroke:i%4?'#27302e':'#3a4641','stroke-width':i%16?'.5':'1'});
  for(let i=0;i<8;i++){const text=add('text',{x:12+i*57,y:17,fill:'#9aa79e','font-size':'9','font-family':'sans-serif'});text.textContent=`${Math.floor(i/4)+1}.${i%4+1}`}
  const low=Math.min(...pattern.notes.map(n=>n.pitch))-1,high=Math.max(...pattern.notes.map(n=>n.pitch))+1;
  for(const note of pattern.notes)add('rect',{x:12+note.start/pattern.lengthBeats*456,y:32+(high-note.pitch)/(high-low)*88,width:Math.max(2,note.duration/pattern.lengthBeats*456),height:6,rx:1.5,fill:'#b9dd65'});
  return svg;
 }
 async function build(){
  try{
   const approvalURL=new URL('assets/audio/approved-manifest.json',document.baseURI),sourceURL=new URL('review/manifest.json',document.baseURI);
   const responses=await Promise.all([fetch(approvalURL),fetch(sourceURL)]);if(responses.some(r=>!r.ok))throw Error('Recordings unavailable');
   const [approval,source]=await Promise.all(responses.map(r=>r.json()));
   if(approval.sourceBatchFingerprint!==fingerprint||source.batchFingerprint!==fingerprint||approval.selections.length!==6)throw Error('Batch mismatch');
   const records=expected.map(id=>{
    const found=approval.selections.filter(r=>r.id===id),candidate=source.candidates.find(c=>c.candidateId===id);
    if(found.length!==1||!candidate||!UndertowCore.valid(candidate.pattern))throw Error('Selection mismatch');
    const item=found[0],pattern=JSON.parse(JSON.stringify(candidate.pattern));
    if(item.midiApproved!==true||item.style!==candidate.styleName||item.render.sourceMidiSha256!==item.midi.sha256)throw Error('MIDI mismatch');
    if(item.transformation){
     const t=item.transformation;if(t.kind!=='transpose'||t.approved!==true||!Number.isInteger(t.semitones)||t.semitones===0||Math.abs(t.semitones)>24||t.semitones%12!==0||t.sourceMidiSha256!==candidate.midiSha256)throw Error('Unapproved transformation');
     pattern.notes.forEach(n=>n.pitch+=t.semitones);
     if(!UndertowCore.valid(pattern)||JSON.stringify(pattern)!==JSON.stringify(item.pattern))throw Error('Transposed notes mismatch');
    }else if(item.midi.sha256!==candidate.midiSha256)throw Error('MIDI mismatch');
    if(!['approved','rendered-awaiting-listening-approval'].includes(item.render.status))throw Error('Recording pending');
    const mix=localAsset(item.audio.mix.path,approvalURL),solo=localAsset(item.audio.solo.path,approvalURL),midi=localAsset(item.midi.path,approvalURL);
    return {item,candidate,pattern,mix,solo,midi};
   });
   for(const record of records){
    const {item,candidate}=record,card=document.createElement('article'),header=document.createElement('header'),titlebox=document.createElement('div'),title=document.createElement('h2'),id=document.createElement('p'),badge=document.createElement('span'),player=document.createElement('audio'),actions=document.createElement('div'),download=document.createElement('a');
    card.className='selected-card';card.id=item.id;title.textContent=item.style;id.className='candidate';id.textContent=item.id+(item.transformation?' · 2 octaves higher':'');titlebox.append(title,id);badge.className='selected-state'+(item.audioApproved?'':' pending');badge.textContent=item.audioApproved?'Recording approved':'New sound · approval pending';header.append(titlebox,badge);
    player.controls=true;player.preload='none';player.src=record[mode.value];player.setAttribute('aria-label',`${item.style} selected recording`);player.addEventListener('play',()=>{players.forEach(p=>{if(p.player!==player)p.player.pause()});if(document.hidden)player.pause()});players.push({player,record});
    actions.className='selected-actions';download.className='link';download.href=record.midi;download.download=`Submotif-${item.variantId||item.id}.mid`;download.textContent='Save exact MIDI';actions.append(download);card.append(header,roll(record.pattern,item.style),player,actions);
    if(item.render.kind==='serum'){const details=document.createElement('details'),summary=document.createElement('summary'),p=document.createElement('p');summary.textContent='Your Serum acid preset';p.textContent=item.render.preset;details.append(summary,p);actions.append(details)}
    list.append(card);
   }
   const approved=records.filter(r=>r.item.audioApproved).length;status.textContent=approved===6?'All six recordings approved.':`${approved} recordings approved. Acid’s new sound awaits listening approval.`;
  }catch{status.textContent='The selected recordings could not load. Refresh this page or return to the full audition gallery.'}
 }
 mode.addEventListener('change',()=>{stop();players.forEach(({player,record})=>{player.src=record[mode.value];player.load()})});
 build();
})();
