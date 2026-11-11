'use strict';
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const core = require('../max/undertow-core.js');
let passed = 0;
function test(name, fn) { fn(); console.log('PASS ' + name); passed++; }
const settings = core.defaults();

test('new devices start at C1 with a one-bar loop',()=>{
  assert.equal(settings.root,0);assert.equal(settings.octave,1);assert.equal(settings.bars,1);
  const p=core.generate(settings,42);
  assert.equal(p.lengthBeats,4);assert.equal(p.notes[0].pitch,36);
});
test('seed determinism and procedural diversity', () => {
  assert.deepEqual(core.generate(settings, 42), core.generate(settings, 42));
  const seen = new Set();
  for (let seed=1; seed<=200; seed++) seen.add(JSON.stringify(core.generate(settings,seed).notes));
  assert.ok(seen.size > 195);
});
test('generation normalizes unit controls to the VST float boundary without changing caller settings',()=>{
  for(let style=0;style<6;style++) {
    const raw=Object.assign({},settings,{style,density:.73,swing:.33,gate:.42,movement:.85});
    const saved=JSON.stringify(raw),normalized=Object.assign({},raw);
    for(const key of ['density','swing','gate','movement'])normalized[key]=Math.fround(raw[key]);
    assert.deepEqual(core.generate(raw,0xffffffff),core.generate(normalized,0xffffffff));
    assert.equal(JSON.stringify(raw),saved,'generation must not rewrite saved control values');
  }
});
test('all styles, scales, keys, lengths are valid and scale-bound', () => {
  for(let style=0; style<6; style++) for(let scale=0;scale<5;scale++)
  for(let root=0;root<12;root++) for(const bars of [1,2,4,8]) {
    const s=Object.assign({},settings,{style,scale,root,bars});
    const p=core.generate(s,100+root);
    assert.ok(core.valid(p));
    assert.equal(p.lengthBeats,bars*4);
    assert.ok(p.notes.length>0 && p.notes.length<=128);
    for(const n of p.notes) assert.ok(core.scales[scale].includes((n.pitch-root+120)%12));
    assert.equal((p.notes[0].pitch-root+120)%12,0);
  }
});
test('root-only and octave switches have separate meanings', () => {
  const one=core.generate(Object.assign({},settings,{root:6,rootOnly:true,octaveJumps:false,density:1}),917);
  assert.ok(one.notes.every(n=>n.pitch===42));
  const many=core.generate(Object.assign({},settings,{root:6,style:4,rootOnly:true,octaveJumps:true,density:1,movement:1}),918);
  assert.ok(many.notes.every(n=>(n.pitch-6)%12===0));
  assert.ok(new Set(many.notes.map(n=>n.pitch)).size>1);
});
test('offbeat root anchor preserves delayed first onset', () => {
  for(let seed=1;seed<50;seed++) {
    const p=core.generate(Object.assign({},settings,{style:2}),seed);
    assert.ok(p.notes[0].start>0);
    assert.equal(p.notes[0].pitch%12,0);
  }
});
test('locked notes survive changed key and generator settings exactly', () => {
  const p=core.generate(settings,65); p.notes[1].locked=true;
  const keep=JSON.stringify(p.notes[1]);
  for(let seed=1;seed<30;seed++) {
    const q=core.generate(Object.assign({},settings,{root:7,swing:0.7,density:1,rootOnly:true}),seed,p);
    assert.ok(core.valid(q));
    assert.ok(q.notes.some(n=>JSON.stringify(n)===keep));
  }
});
test('MIDI has correct header, balanced notes, and exact loop endpoint', () => {
  const p=core.generate(settings,123), bytes=core.midiFile(p), b=Buffer.from(bytes);
  assert.equal(b.toString('ascii',0,4),'MThd');
  assert.equal(b.readUInt16BE(12),960);
  assert.equal(b.toString('ascii',14,18),'MTrk');
  assert.equal(b.readUInt32BE(18),b.length-22);
  let pos=22,tick=0,ons=0,offs=0,end=-1;
  while(pos<b.length) {
    let v=0,x; do {x=b[pos++];v=(v<<7)|(x&127);} while(x&128);tick+=v;
    const status=b[pos++];
    if(status===255) {const type=b[pos++],len=b[pos++];if(type===47)end=tick;pos+=len;}
    else {if((status&240)===144)ons++;if((status&240)===128)offs++;pos+=2;}
  }
  assert.equal(ons,p.notes.length);assert.equal(offs,ons);assert.equal(end,p.lengthBeats*960);
});
test('poll scheduler flushes on stop, seek, disable, replacement; handles loop', () => {
  const p={lengthBeats:4,seed:1,notes:[{start:0,duration:0.5,pitch:36,velocity:99,locked:false},{start:3.75,duration:0.2,pitch:38,velocity:90,locked:false}]};
  const s=new core.Scheduler();
  assert.deepEqual(s.process(p,0,true,true,1),[{pitch:36,velocity:99,on:true}]);
  assert.deepEqual(s.process(p,0.2,false,true,1),[{pitch:36,velocity:0,on:false}]);
  s.process(p,0,true,true,1);assert.ok(s.process(p,2,true,true,1).some(e=>!e.on));
  s.process(p,4,true,true,1);assert.ok(s.process(p,4.1,true,false,1).some(e=>!e.on));
  s.process(p,8,true,true,1);assert.ok(s.process(p,8.1,true,true,2).some(e=>!e.on));
  s.process(p,11.75,true,true,2);
  assert.ok(s.process(p,12,true,true,2).some(e=>e.on && e.pitch===36));
});
test('corrupt state/pattern data is rejected', () => {
  const p=core.generate(settings,11);
  p.notes[0].duration=-1;assert.equal(core.valid(p),false);
  p.notes[0].duration=Infinity;assert.equal(core.valid(p),false);
  assert.equal(core.valid({notes:[],lengthBeats:8,seed:1}),false);
  assert.equal(core.valid({notes:{},lengthBeats:8,seed:1}),false);
  assert.equal(core.valid({notes:{length:1,0:{start:0,duration:.5,pitch:36,velocity:99}},lengthBeats:8,seed:1}),false);
  for(const field of ['start','duration','pitch','velocity']) {
    const q=core.generate(settings,11);q.notes[0][field]=String(q.notes[0][field]);
    assert.equal(core.valid(q),false,'numeric strings are not valid note data');
  }
});
test('unsupported and denormal loop lengths cannot reach scheduler cycles',()=>{
  for(const lengthBeats of [Number.MIN_VALUE,1e-9,1,2,12,31.999999999]) {
    const p={lengthBeats,seed:1,notes:[{start:0,duration:1e-9,pitch:36,velocity:99,locked:false}]};
    // Fail validation before calling the scheduler: the old denormal case hangs.
    assert.equal(core.valid(p),false);
    const scheduler=new core.Scheduler();
    assert.deepEqual(scheduler.process(p,1,true,true,1),[]);
  }
  const p=core.generate(settings,123),scheduler=new core.Scheduler();
  scheduler.process(p,1,true,true,1);
  assert.deepEqual(scheduler.process(p,Number.MAX_VALUE,true,true,1).filter(e=>e.on),[]);
});
test('dense eight-bar variation reserves capacity for late locked notes',()=>{
  const before=Object.assign({},settings,{style:1,bars:8,gate:0,density:1,swing:0});
  const p=core.generate(before,1);p.notes[p.notes.length-1].locked=true;
  const locked=JSON.stringify(p.notes[p.notes.length-1]);
  for(let seed=1;seed<=20;seed++) {
    const q=core.generate(Object.assign({},before,{swing:1}),seed,p,true);
    assert.ok(core.valid(q));assert.ok(q.notes.length<=128);
    assert.ok(q.notes.some(n=>JSON.stringify(n)===locked),'late locked note must survive capacity limit');
  }
});
test('variation observes changed register while keeping explicit locks',()=>{
  const p={lengthBeats:4,seed:1,notes:[0,1,2].map(start=>({start,duration:.5,pitch:36,velocity:99,locked:false}))};
  for(const octaveJumps of [false,true])for(let seed=1;seed<20;seed++) {
    const q=core.generate(Object.assign({},settings,{octave:2,rootOnly:true,octaveJumps}),seed,p,true);
    assert.ok(q.notes.every(n=>n.pitch>=48&&n.pitch<=71));
    if(!octaveJumps)assert.ok(q.notes.every(n=>n.pitch===48));
  }
  p.notes[0].locked=true;
  const q=core.generate(Object.assign({},settings,{octave:2,rootOnly:true,octaveJumps:false}),1,p,true);
  assert.deepEqual(q.notes[0],p.notes[0]);
  assert.ok(q.notes.slice(1).every(n=>n.pitch===48));
});
test('extreme controls remain musical MIDI and locked notes outside shorter loops are omitted',()=>{
  for(let style=0;style<6;style++)for(const density of [0,1])for(const gate of [0,1]) {
    const p=core.generate(Object.assign({},settings,{style,density,gate,swing:1,octave:2,root:11,bars:8}),19);
    assert.ok(core.valid(p));assert.ok(p.notes.every(n=>n.pitch>=59&&n.pitch<=71));
  }
  const p=core.generate(Object.assign({},settings,{bars:8,density:1}),99);
  p.notes[p.notes.length-1].locked=true;
  const q=core.generate(Object.assign({},settings,{bars:1}),52,p);
  assert.ok(core.valid(q));assert.ok(q.notes.every(n=>n.start+n.duration<=4));
});
test('phrases retain their opening and resolve scalar connectors into actual targets',()=>{
  for(let style=0;style<6;style++)for(let scale=0;scale<5;scale++)for(let seed=1;seed<=24;seed++) {
    const s=Object.assign({},settings,{style,scale,bars:4,movement:seed%2?1:.35});
    const p=core.generate(s,(0xa341316c+Math.imul(seed,2654435761))>>>0);
    const triad=new Set([0,scale===3?4:3,7]);
    const shape=bar=>p.notes.filter(n=>n.start>=bar*4&&n.start<bar*4+2)
      .map(n=>[Math.round((n.start-bar*4)*1e9),Math.round(n.duration*1e9),n.pitch,n.velocity]);
    assert.ok(shape(0).length>0);
    for(let bar=1;bar<4;bar++)assert.deepEqual(shape(bar),shape(0),'opening rhythm, pitch and articulation must recur');
    for(let i=0;i<p.notes.length;i++) {
      const n=p.notes[i];assert.ok(n.pitch>=36&&n.pitch<=48);
      if(Math.abs(n.start-Math.round(n.start))<1e-6)assert.ok(triad.has(n.pitch%12));
      if(triad.has(n.pitch%12))continue;
      const target=p.notes[(i+1)%p.notes.length],distance=Math.abs(n.pitch-target.pitch);
      const gap=target.start-n.start+(i+1===p.notes.length?p.lengthBeats:0);
      assert.equal(Math.floor(n.start*4+1e-7)%2,1);
      assert.ok(n.duration<=.2+1e-9&&triad.has(target.pitch%12)&&distance>=1&&distance<=2&&gap<=.5+1e-9,
        'a scalar connector must be short, weak, and resolve stepwise, including at the loop seam');
    }
  }
});
test('repeated compatible Variations keep count and opening while editing a bounded subset',()=>{
  for(let style=0;style<6;style++) {
    const s=Object.assign({},settings,{style,bars:4,movement:1});
    let p=core.generate(s,444);
    const count=p.notes.length;
    for(let turn=1;turn<=12;turn++) {
      const next=core.generate(s,turn*173,p,true);
      assert.ok(core.valid(next));assert.equal(next.notes.length,count);
      let edited=0;
      for(let i=0;i<count;i++) {
        assert.equal(next.notes[i].start,p.notes[i].start);
        if(JSON.stringify(next.notes[i])!==JSON.stringify(p.notes[i]))edited++;
        if(p.notes[i].start%4<2)assert.deepEqual(next.notes[i],p.notes[i]);
      }
      assert.ok(edited<=Math.max(1,Math.ceil(count*.2)),'Variation must develop a small part of the phrase');
      p=next;
    }
  }
});
test('Variation cannot extend a tiny note across a nearby off-grid lock',()=>{
  const s=Object.assign({},settings,{rootOnly:true,octaveJumps:false});
  const p={lengthBeats:4,seed:1,notes:[
    {start:0,duration:.2,pitch:36,velocity:100,locked:false},
    {start:2,duration:.002,pitch:36,velocity:100,locked:false},
    {start:2.003,duration:.5,pitch:36,velocity:100,locked:true}]};
  const q=core.generate(s,5,p,true);
  assert.ok(core.valid(q));assert.equal(q.notes.length,3);
  assert.deepEqual(q.notes[2],p.notes[2]);
  assert.ok(q.notes[1].start+q.notes[1].duration<=q.notes[2].start);
});
test('all styles retain Dorian and Phrygian colors as optional resolving motion',()=>{
  for(let style=0;style<6;style++)for(const scale of [1,2]) {
    const s=Object.assign({},settings,{style,scale,movement:1,bars:1});
    let found=false;
    for(let i=0;i<256;i++) {
      const p=core.generate(s,(0xa341316c+Math.imul(i,2654435761))>>>0);
      found=found||p.notes.some(n=>n.pitch%12===(scale===1?9:1));
    }
    assert.ok(found,'characteristic mode colors must remain available in style '+style);
  }
});
test('Max core works without Math.fround or typed arrays',()=>{
  const legacyMath=Object.create(Math);legacyMath.fround=undefined;
  const context={Math:legacyMath,JSON,Array,Object,isFinite,module:{exports:{}}};
  vm.createContext(context);
  vm.runInContext(fs.readFileSync(path.join(__dirname,'../max/undertow-core.js'),'utf8'),context);
  const legacy=context.module.exports;
  const s=Object.assign({},settings,{density:.73,swing:.33,gate:.42,movement:.85});
  assert.deepEqual(JSON.parse(JSON.stringify(legacy.generate(s,0))),core.generate(s,0));
});

const runtimePath=path.join(__dirname,'../max/undertow.js');
assert.ok(fs.existsSync(runtimePath),'Max runtime exists');
{
  function runtime(api) {
    const events=[],ui={},uiMessages=[];
    const c={JSON,Math,Date,isFinite,parseInt,Number,String,Array,Object,UndertowCore:core,
      include(){}, outlet(...args){events.push(args);},notifyclients(){},post(){},error(){},
      arrayfromargs(a){return Array.from(a);},
      patcher:{getnamed(name){return {
        message(...args){ui[name]=args;uiMessages.push([name,...args]);},
        set hidden(value){uiMessages.push([name,'hidden',Number(value)]);}
      };}},
      LiveAPI:api,
      Task:function(fn,self){this.schedule=()=>fn.call(self);this.cancel=()=>{};}};
    vm.createContext(c);vm.runInContext(fs.readFileSync(runtimePath,'utf8'),c);
    return {c,events,ui,uiMessages};
  }
  test('Max runtime state round-trip retains exact notes/locks/settings',()=>{
    const {c}=runtime();c.loadbang();c.generate();c.locknote(0);
    const state=c.getvalueof();c.generate();c.setvalueof(state);
    assert.equal(c.getvalueof(),state);
    c.setvalueof('{bad json');assert.equal(c.getvalueof(),state);
  });
  test('existing Live Sets retain their saved register and two-bar length',()=>{
    const {c}=runtime(),old=Object.assign({},settings,{root:7,octave:2,bars:2});
    const saved=JSON.stringify({version:1,settings:old,committed:old,pattern:core.generate(old,42),enabled:1});
    c.restore(saved);assert.equal(c.getvalueof(),saved);
  });
  test('Change a few notes waits for pending controls to be applied with Generate',()=>{
    const {c,ui}=runtime();
    const initial=core.generate(settings,42);initial.notes[0].locked=true;
    c.restore(JSON.stringify({version:1,settings,committed:settings,pattern:initial,enabled:1}));
    c.cfg('density',0);c.cfg('gate',0);c.cfg('style',2);
    const saved=c.getvalueof(),history=c.undoStack.length;
    const before=JSON.parse(saved),seed=(before.pattern.seed+2654435769)>>>0;
    const expected=core.generate(before.settings,seed,before.pattern,false);
    assert.deepEqual(ui.action_vary,['active',0]);
    c.vary();
    assert.equal(c.getvalueof(),saved,'a disabled variation must not apply controls or consume a seed');
    assert.equal(c.undoStack.length,history,'a disabled variation must not create an undo step');
    assert.ok(ui.status[1].includes('Generate'));
    c.generate();
    const after=JSON.parse(c.getvalueof());
    assert.deepEqual(after.pattern,expected,'Generate applies pending settings');
    assert.ok(after.pattern.notes.some(n=>JSON.stringify(n)===JSON.stringify(initial.notes[0])));
    assert.deepEqual(after.committed,after.settings);
    assert.deepEqual(ui.action_vary,['active',1]);
  });
  test('Variation with unchanged controls develops the existing phrase',()=>{
    const {c}=runtime();
    const initial=core.generate(settings,42);
    c.restore(JSON.stringify({version:1,settings,committed:settings,pattern:initial,enabled:1}));
    const expected=core.generate(settings,(initial.seed+2654435769)>>>0,initial,true);
    c.vary();assert.deepEqual(JSON.parse(c.getvalueof()).pattern,expected);
  });
  test('More controls changes only presentation and retains pending settings, notes and history',()=>{
    const {c,events,uiMessages}=runtime();c.loadbang();c.generate();c.locknote(0);c.cfg('swing',29);
    const before=c.getvalueof(),revision=c.revision,undo=c.undoStack.length,redo=c.redoStack.length;
    events.length=0;uiMessages.length=0;
    c.details();
    assert.ok(uiMessages.some(m=>JSON.stringify(m)===JSON.stringify(['this-device','setwidth',1260])));
    assert.ok(uiMessages.some(m=>JSON.stringify(m)===JSON.stringify(['swing','hidden',0])));
    assert.equal(c.getvalueof(),before);assert.equal(c.revision,revision);
    assert.equal(c.undoStack.length,undo);assert.equal(c.redoStack.length,redo);
    assert.deepEqual(events,[],'presentation changes must not route MIDI or redraw a different phrase');
    c.details();
    assert.ok(uiMessages.some(m=>JSON.stringify(m)===JSON.stringify(['this-device','setwidth',1000])));
    assert.ok(uiMessages.some(m=>JSON.stringify(m)===JSON.stringify(['swing','hidden',1])));
    assert.equal(c.getvalueof(),before);
    assert.ok(!Object.hasOwn(JSON.parse(before),'detailsOpen'),'disclosure is not musical preset state');
  });
  test('device load and project restore do not invoke transient actions',()=>{
    const calls=[];
    function API(){calls.push('Live API action');}
    const {c,events}=runtime(API),saved=c.getvalueof();
    c.loadbang();c.ready();c.restore(saved);
    assert.equal(c.getvalueof(),saved,'restoring state must not generate another pattern');
    assert.deepEqual(calls,[],'restoring state must not create a clip');
    assert.deepEqual(events.filter(e=>e[0]===3),[],'restoring state must not open a save dialog');
  });
  test('lock-only undo, redo, and seed-only restore do not retrigger held notes',()=>{
    const {c,events}=runtime();
    c.pattern={lengthBeats:8,seed:1,notes:[{start:0,duration:1,pitch:36,velocity:99,locked:false}]};
    c.hostPlaying=1;c.ppq=.1;c.pulse();events.length=0;
    const revision=c.revision;
    c.locknote(0);c.undo();c.redo();c.unlock();c.undo();c.pulse();
    assert.equal(c.revision,revision);
    assert.deepEqual(events.filter(e=>e[0]===0),[]);
    events.length=0;
    const saved=JSON.parse(c.getvalueof());saved.pattern.seed=999;
    c.restore(JSON.stringify(saved));c.pulse();
    assert.equal(c.revision,revision);
    assert.deepEqual(events.filter(e=>e[0]===0),[]);
    saved.pattern.notes[0].pitch=40;
    c.restore(JSON.stringify(saved));c.pulse();
    assert.ok(c.revision>revision);
    assert.ok(events.some(e=>e[0]===0&&e[1]===128));
    assert.ok(events.some(e=>e[0]===0&&e[1]===144));
  });
  test('MIDI save writes exactly the path accepted in the dialog',()=>{
    const {c}=runtime(),opened=[];
    c.File=function(filePath){opened.push(filePath);this.isopen=true;this.writebytes=function(){};this.close=function(){};};
    c.beginexport();c.exportfile('/tmp/explicit choice');
    c.beginexport();c.exportfile('/tmp/selected.mid');
    assert.deepEqual(opened,['/tmp/explicit choice','/tmp/selected.mid']);
  });
  test('restoring or re-enabling generation releases notes from MIDI through',()=>{
    const {c,events}=runtime();
    c.locknote(0);c.output(0);events.length=0;
    c.undo();
    assert.equal(c.enabled,1);
    assert.ok(events.some(e=>e[0]===0&&e[1]===176),'restoring generation must release through notes before gate closes');
    c.deviceactive(0);events.length=0;c.deviceactive(1);
    assert.ok(events.some(e=>e[0]===0&&e[1]===176),'device re-enable must release through notes');
    events.length=0;c.deviceactive(1);
    assert.deepEqual(events.filter(e=>e[0]===0),[],'unchanged device-active messages must not cut notes');
  });
  test('clip creation refuses occupied slot and targets containing track only',()=>{
    const calls=[];
    const nodes={1:{type:'MaxDevice',parent:2},2:{type:'Track',path:'live_set tracks 3'},3:{type:'Scene',path:'live_set view selected_scene'},4:{type:'ClipSlot',occupied:1},6:{type:'Song'}};
    function API(cb,p){this.id=p==='this_device'?1:p==='live_set view selected_scene'?3:p==='live_set'?6:4;}
    Object.defineProperties(API.prototype,{type:{get(){return nodes[this.id].type;}},unquotedpath:{get(){return nodes[this.id].path||'live_set tracks 3 clip_slots 2';}},path:{get(){return this.unquotedpath;}}});
    API.prototype.get=function(k){if(k==='canonical_parent')return ['id',nodes[this.id].parent];if(k==='scenes')return ['id',55,'id',56,'id',3];if(k==='has_clip')return [1];if(k==='is_frozen')return[0];return[0];};
    API.prototype.call=function(...a){calls.push(a);};
    const {c,ui}=runtime(API);c.loadbang();c.ready();c.createclip();assert.equal(calls.length,0);
    assert.ok(ui.status[1].includes('already has a clip'));
  });
  test('clip creation resolves rack parents, adds exact notes and stops generator without launching',()=>{
    const calls=[],nodes={1:{type:'MaxDevice',parent:9},9:{type:'Chain',parent:8},8:{type:'RackDevice',parent:2},2:{type:'Track',path:'live_set tracks 3'},3:{type:'Scene',path:'live_set view selected_scene'},4:{type:'ClipSlot'},5:{type:'Clip'},6:{type:'Song'}};
    function API(cb,p){this.id=p==='this_device'?1:p==='live_set view selected_scene'?3:p==='live_set'?6:p.endsWith(' clip')?5:4;this.request=p;}
    Object.defineProperties(API.prototype,{type:{get(){return nodes[this.id].type;}},unquotedpath:{get(){return nodes[this.id].path||'live_set tracks 3 clip_slots 2';}},path:{get(){return this.unquotedpath;}}});
    API.prototype.get=function(k){if(k==='canonical_parent')return ['id',nodes[this.id].parent];if(k==='scenes')return ['id',55,'id',56,'id',3];return [0];};
    API.prototype.call=function(...a){calls.push([this.request,...a]);return [];};
    API.prototype.set=function(...a){calls.push([this.request,'set',...a]);};
    const {c}=runtime(API);c.loadbang();c.ready();const pattern=JSON.parse(c.getvalueof()).pattern;c.createclip();
    const add=calls.find(a=>a[1]==='add_new_notes');assert.ok(add);
    assert.equal(add[0],'live_set tracks 3 clip_slots 2 clip');
    const notes=JSON.parse(add[2]).notes;assert.equal(notes.length,pattern.notes.length);
    assert.deepEqual(notes.map(n=>[n.pitch,n.start_time,n.duration,n.velocity]),pattern.notes.map(n=>[n.pitch,n.start,n.duration,n.velocity]));
    assert.equal(JSON.parse(c.getvalueof()).enabled,0);
    assert.equal(calls.filter(a=>a[1]==='create_clip').length,1);
    assert.equal(calls.filter(a=>a[1]==='fire'||a[1]==='delete_clip'||a[1]==='remove_notes_extended').length,0);
  });
}

const amxdPath=path.join(__dirname,'../max/Submotif.amxd');
assert.ok(fs.existsSync(amxdPath),'Built Max device exists');
test('AMXD MIDI type, dependency files and wiring are intact',()=>{
  const b=fs.readFileSync(amxdPath);assert.equal(b.toString('ascii',0,4),'ampf');
  assert.equal(b.toString('ascii',8,12),'mmmm');assert.equal(b.toString('ascii',24,28),'ptch');
  assert.equal(b.readUInt32LE(28),b.length-32);
  const p=JSON.parse(b.subarray(32).toString('utf8').replace(/\0+$/,'' )).patcher;
  const ids=new Set(p.boxes.map(o=>o.box.id));
  for(const l of p.lines){assert.ok(ids.has(l.patchline.source[0]));assert.ok(ids.has(l.patchline.destination[0]));}
  assert.ok(p.boxes.some(o=>o.box.text==='midiout'));
  assert.ok(p.boxes.some(o=>o.box.text==='transport'));
  assert.ok(p.boxes.some(o=>o.box.text==='loadmess name Submotif.mid'));
  for(const id of ['generate','vary','undo','redo','clip','export','panic','unlock','more']) {
    const action=p.boxes.find(o=>o.box.id===id).box;
    assert.equal(action.parameter_enable,1,id+' must expose its named control to the host');
    assert.equal(action.mode,0,id+' remains momentary');
    assert.equal(action.transition,0,id+' fires only on the rising edge');
    const parameter=action.saved_attribute_attributes.valueof;
    assert.equal(parameter.parameter_invisible,4,id+' uses Visible (Not Stored), without automation or preset recall');
    assert.equal(parameter.parameter_initial_enable,0,id+' must not fire an initial value on load');
    assert.equal(parameter.parameter_visible_undoable,0,id+' must not insert transient button state in Live undo');
    assert.ok(parameter.parameter_longname&&action.varname);
  }
  assert.ok(p.boxes.some(o=>o.box.text && o.box.text.includes('pattr patternState')));
  for(const d of p.dependency_cache)assert.ok(fs.existsSync(path.join(__dirname,'../max',d.name)));
  assert.deepEqual(p,JSON.parse(fs.readFileSync(path.join(__dirname,'../max/Submotif.maxpat'),'utf8')).patcher,
    'readable source and loaded AMXD must contain the same patch');
});
test('Max presentation keeps core workflow compact and secondary controls hidden',()=>{
  const p=JSON.parse(fs.readFileSync(path.join(__dirname,'../max/Submotif.maxpat'),'utf8')).patcher;
  const box=id=>p.boxes.find(o=>o.box.id===id).box;
  assert.equal(p.devicewidth,1000);assert.equal(p.openrect[3],169);
  assert.equal(box('root').saved_attribute_attributes.valueof.parameter_initial[0],0);
  assert.equal(box('octave').saved_attribute_attributes.valueof.parameter_initial[0],1);
  assert.equal(box('bars').saved_attribute_attributes.valueof.parameter_initial[0],0);
  assert.equal(box('vary').text,'Change a few notes');
  const generate=box('generate').presentation_rect,vary=box('vary').presentation_rect;
  assert.ok(vary[1]-(generate[1]+generate[3])<=6,'related generation actions stay together');
  for(const id of ['octave','swing','gate','rootOnly','anchorRoot','octaveJumps','panic'])assert.equal(box(id).hidden,1);
  for(const id of ['density','movement','swing','gate']) {
    assert.equal(box(id).maxclass,'live.slider');assert.equal(box(id).orientation,1);
  }
  const essentials=['root','scale','style','bars','density','movement','generate','vary','undo','redo','clip','export','output','more','roll'];
  for(const id of essentials) {
    const b=box(id),[x,y,w,h]=b.presentation_rect;
    assert.ok(!b.hidden&&x>=0&&x+w<=1000&&y>=0&&y+h<=169,id+' is accessible in the basic view');
  }
  const interactive=p.boxes.map(o=>o.box).filter(b=>b.presentation&&(/live\.(menu|slider|text)/.test(b.maxclass)||b.id==='roll'));
  for(const decoration of p.boxes.map(o=>o.box).filter(b=>b.presentation&&b.maxclass==='panel'&&!b.background)) {
    const [dx,dy,dw,dh]=decoration.presentation_rect;
    for(const control of interactive) {
      const [x,y,w,h]=control.presentation_rect;
      assert.ok(dx+dw<=x||x+w<=dx||dy+dh<=y||y+h<=dy,'foreground decoration must not cover '+control.id);
    }
  }
  for(let i=0;i<interactive.length;i++)for(let j=i+1;j<interactive.length;j++) {
    const a=interactive[i],b=interactive[j],[ax,ay,aw,ah]=a.presentation_rect,[bx,by,bw,bh]=b.presentation_rect;
    assert.ok(ax+aw<=bx||bx+bw<=ax||ay+ah<=by||by+bh<=ay,a.id+' must not overlap '+b.id);
  }
});
console.log(`${passed} Max companion checks passed.`);
