/* Max for Live adapter. The adjacent core is ES5 and is tested outside Max. */
autowatch = 0;
inlets = 1;
outlets = 4; // MIDI bytes, piano roll, MIDI-through gate, save dialog
include('undertow-core.js');
var core=UndertowCore;
var settings=core.defaults(),committed=core.copy(settings);
var pattern=core.generate(settings,(new Date().getTime()>>>0)||1);
var registerShift=0,playbackPattern=core.copy(pattern);
var scheduler=new core.Scheduler(),revision=1,enabled=1,deviceEnabled=1,hostPlaying=0,ppq=0;
var apiReady=false,pending=false,undoStack=[],redoStack=[],exportPattern=null;
var detailsOpen=false;
var detailControls=['registerShift','swing','gate','rootOnly','anchorRoot','octaveJumps','action_panic'];
var detailLabels=['advanced-divider','advanced-title','label-registerShift',
    'label-swing','value-swing','label-gate','value-gate'];
var lastDraw=0;
var rootNames=['C','C#','D','D#','E','F','F#','G','G#','A','A#','B'];
var scaleNames=['Minor','Dorian','Phrygian','Major','Minor Pent.'];
var styleNames=['DEEP HOUSE','ROLLING','OFFBEAT','ACID','DISCO','FUNK'];

function ui(name,method,value) {
    try {
        var o=this.patcher.getnamed(name);if(!o)return;
        // Visibility belongs to the Max object box, not its message inlet.
        if(method==='hidden')o.hidden=!!value;
        else o.message(method,value);
    } catch(e) {}
}
function status(text) { ui('status','set',text); }
function midi(events) { for(var i=0;i<events.length;i++){var e=events[i];outlet(0,e.on?144:128);outlet(0,e.pitch);outlet(0,e.velocity);} }
function flush() { midi(scheduler.flush()); }
function routing() { outlet(2,enabled&&deviceEnabled?0:1); }
function applyDisclosure() {
    // Live fixes the presentation height. Its documented setwidth message lets
    // secondary controls expand sideways without touching musical state.
    var hidden=detailsOpen?0:1,i;
    for(i=0;i<detailLabels.length;i++)ui(detailLabels[i],'hidden',hidden);
    for(i=0;i<detailControls.length;i++) {
        ui(detailControls[i],'hidden',hidden);
        ui(detailControls[i],'active',detailsOpen?1:0);
    }
    ui('action_more','text',detailsOpen?'Fewer controls':'More controls');
    ui('action_more','texton',detailsOpen?'Fewer controls':'More controls');
    ui('this-device','setwidth',detailsOpen?1260:1000);
}
function details() {detailsOpen=!detailsOpen;applyDisclosure();}
function broadcast() {
    playbackPattern=core.transposeOctaves(pattern,registerShift);
    outlet(1,'pattern',JSON.stringify(playbackPattern));
    var bars=pattern.lengthBeats/4,locks=0,i;
    for(i=0;i<pattern.notes.length;i++)if(pattern.notes[i].locked)locks++;
    ui('summary','set',rootNames[committed.root]+' '+scaleNames[committed.scale]+'  ·  '+bars+(bars===1?' bar':' bars')+'  ·  '+pattern.notes.length+' notes');
    ui('action_vary','active',pending?0:1);
    ui('action_undo','active',undoStack.length?1:0);
    ui('action_redo','active',redoStack.length?1:0);
    ui('action_unlock','hidden',locks?0:1);
    ui('action_unlock','active',locks?1:0);
    var amounts=['density','swing','gate','movement'];
    for(i=0;i<amounts.length;i++)ui('value-'+amounts[i],'set',Math.round(settings[amounts[i]]*100)+'%');
    if(pending)status('Settings changed — Generate to apply');
    else status(locks?locks+(locks===1?' note locked':' notes locked')+' — click again to unlock.':'Click a note to keep it. Amber notes are locked.');
    routing();
}
function syncControls() {
    var keys=['root','scale','style','octave'],i;
    for(i=0;i<keys.length;i++)ui(keys[i],'set',settings[keys[i]]);
    ui('registerShift','set',registerShift+5);
    ui('bars','set',[1,2,4,8].indexOf(settings.bars));
    keys=['density','swing','gate','movement'];
    for(i=0;i<keys.length;i++)ui(keys[i],'set',Math.round(settings[keys[i]]*100));
    keys=['rootOnly','anchorRoot','octaveJumps'];
    for(i=0;i<keys.length;i++)ui(keys[i],'set',settings[keys[i]]?1:0);
    ui('output','set',enabled);
}
function loadbang() {syncControls();applyDisclosure();broadcast();}
function ready() {apiReady=true;flush();syncControls();applyDisclosure();broadcast();}
function changed() {notifyclients();broadcast();}
function cfg(name,value) {
    if(name==='registerShift'){shiftregister(Number(value)-5);return;}
    if(!settings.hasOwnProperty(name))return;
    if(name==='bars')value=[1,2,4,8][Math.max(0,Math.min(3,Math.round(value)))];
    else if(name==='density'||name==='swing'||name==='gate'||name==='movement')value=Number(value)/100;
    else if(name==='rootOnly'||name==='anchorRoot'||name==='octaveJumps')value=Number(value)!==0;
    else value=Number(value);
    settings[name]=value;settings=core.sanitise(settings);
    pending=JSON.stringify(settings)!==JSON.stringify(committed);
    changed();
}
function checkpoint() {undoStack.push(getvalueof());if(undoStack.length>32)undoStack.shift();redoStack=[];}
function make(variation) {
    var next=(pattern.seed+2654435769)>>>0;
    // Only Generate applies pending settings. Variation always edits this motif.
    var generated=core.generate(settings,next,pattern,variation&&!pending);
    if(!core.transposeOctaves(generated,registerShift)){status('MIDI note limit: reduce Octave shift, then Generate.');return;}
    checkpoint();
    if(!core.samePlayback(pattern,generated)){flush();revision++;}
    pattern=generated;
    committed=core.copy(settings);pending=false;changed();
}
function shiftregister(octaves) {
    if(!core.transposeOctaves(pattern,octaves)){
        ui('registerShift','set',registerShift+5);
        status('MIDI note limit: choose a smaller octave shift.');return false;
    }
    if(octaves===registerShift)return true;
    checkpoint();flush();registerShift=octaves;revision++;
    ui('registerShift','set',registerShift+5);changed();
    status('Octave shift applied to playback, clips and MIDI export.');return true;
}
function generate() {make(false);}
function vary() {
    if(pending){status('Generate to apply settings before changing a few notes.');return;}
    make(true);
}
function locknote(index) {
    index=Math.floor(index);if(index<0||index>=pattern.notes.length)return;
    checkpoint();pattern.notes[index].locked=!pattern.notes[index].locked;changed();
}
function unlock() {checkpoint();for(var i=0;i<pattern.notes.length;i++)pattern.notes[i].locked=false;changed();}
function undo() {if(!undoStack.length){status('Nothing to undo');return;}redoStack.push(getvalueof());restore(undoStack.pop());}
function redo() {if(!redoStack.length){status('Nothing to redo');return;}undoStack.push(getvalueof());restore(redoStack.pop());}
function output(value) {
    var next=Number(value)!==0?1:0;
    if(next!==enabled) {
        flush();
        // On entering generation mode release any keyboard notes previously passed through.
        if(next)alloff();
    }
    enabled=next;routing();notifyclients();
    status(enabled?'Generator on — start Live transport to play':'Generator off — track MIDI passes through');
}
function alloff() {for(var ch=0;ch<16;ch++){outlet(0,176+ch);outlet(0,64);outlet(0,0);outlet(0,176+ch);outlet(0,123);outlet(0,0);}}
function panic() {flush();alloff();status('All notes released');}
function deviceactive(v) {
    var next=Number(v)!==0?1:0,wasGenerating=enabled&&deviceEnabled;
    if(next!==deviceEnabled)flush();
    deviceEnabled=next;
    if(!wasGenerating&&enabled&&deviceEnabled)alloff();
    routing();
}
function position(ticks) {ppq=Number(ticks)/480;}
function playing(v) {hostPlaying=Number(v)!==0?1:0;if(!hostPlaying)flush();}
function pulse() {
    midi(scheduler.process(playbackPattern,ppq,!!hostPlaying,!!enabled&&!!deviceEnabled,revision));
    var now=new Date().getTime();
    if(now-lastDraw>32) {outlet(1,'playhead',hostPlaying&&enabled&&deviceEnabled?ppq%pattern.lengthBeats:-1);ui('transport','set',hostPlaying?'Playing':'Stopped');lastDraw=now;}
}
function freebang() {flush();}
function getvalueof() {
    var saved={version:1,settings:settings,committed:committed,pattern:pattern,enabled:enabled};
    if(registerShift!==0)saved.registerShift=registerShift;
    return JSON.stringify(saved);
}
function restore(text) {
    try {
        if(typeof text!=='string'||text.length>100000)throw new Error('State is not valid');
        var saved=JSON.parse(text);
        if(saved.version!==1||!core.valid(saved.pattern))throw new Error('Invalid pattern');
        var shift=typeof saved.registerShift==='undefined'?0:saved.registerShift;
        if(!core.transposeOctaves(saved.pattern,shift))throw new Error('Invalid octave shift');
        var clean=core.sanitise(saved.settings),c=core.sanitise(saved.committed);
        var nextEnabled=saved.enabled?1:0,wasGenerating=enabled&&deviceEnabled;
        if(shift!==registerShift||!core.samePlayback(pattern,saved.pattern)||nextEnabled!==enabled){flush();revision++;}
        settings=clean;committed=c;pattern=core.copy(saved.pattern);registerShift=shift;enabled=nextEnabled;
        if(!wasGenerating&&enabled&&deviceEnabled)alloff();
        pending=JSON.stringify(settings)!==JSON.stringify(committed);
        syncControls();changed();
    } catch(e) {status('Saved state rejected — current pattern kept');}
}
function setvalueof(value) {restore(String(value));}
function beginexport() {exportPattern=core.copy(playbackPattern);outlet(3,'bang');}
function exportfile() {
    var path=arrayfromargs(arguments).join(' '),p=exportPattern||playbackPattern;exportPattern=null;
    if(!path)return;
    var file=null;
    try {
        var bytes=core.midiFile(p);file=new File(path,'write',['Midi']);
        if(!file.isopen)throw new Error('Could not open file');
        file.position=0;file.writebytes(bytes);file.eof=bytes.length;file.close();file=null;
        status('MIDI saved — drag the file into any instrument track');
    } catch(e) {if(file)file.close();status('MIDI export failed — choose a writable folder');}
}
function valueOf(result) {return result instanceof Array?result[result.length-1]:result;}
function idOf(result) {var a=result instanceof Array?result:[result];for(var i=0;i<a.length;i++)if(a[i]==='id')return Number(a[i+1]);return Number(a[0]);}
function createclip() {
    if(!apiReady){status('Open this MIDI effect in Ableton Live to create a clip');return;}
    var slot=null,created=false;
    try {
        // Ascend canonical parents so devices nested in racks still target their own track.
        var track=new LiveAPI(null,'this_device'),depth=0;
        while(track.id&&track.type!=='Track'&&depth++<16){var parent=idOf(track.get('canonical_parent'));if(!parent)break;track.id=parent;}
        if(!track.id||track.type!=='Track')throw new Error('Could not find the containing MIDI track');
        if(Number(valueOf(track.get('is_frozen'))))throw new Error('Unfreeze this track before creating a clip');
        var scene=new LiveAPI(null,'live_set view selected_scene');
        // A view path can remain "live_set view selected_scene" rather than
        // resolve to a canonical scene path. Resolve its identity in song order.
        var song=new LiveAPI(null,'live_set'),sceneList=song.get('scenes'),sceneIds=[];
        if(!(sceneList instanceof Array))sceneList=[sceneList];
        for(var sceneEntry=0;sceneEntry<sceneList.length;sceneEntry++) {
            if(sceneList[sceneEntry]==='id'&&sceneEntry+1<sceneList.length)
                sceneIds.push(Number(sceneList[++sceneEntry]));
            else if(Number(sceneList[sceneEntry])>0)sceneIds.push(Number(sceneList[sceneEntry]));
        }
        var sceneIndex=sceneIds.indexOf(Number(scene.id));
        if(sceneIndex<0)throw new Error('Select a scene in Session View first');
        var trackPath=String(track.unquotedpath||track.path).replace(/"/g,'');
        if(!/^live_set tracks \d+$/.test(trackPath))throw new Error('Clip creation requires a regular MIDI track');
        var slotPath=trackPath+' clip_slots '+sceneIndex;slot=new LiveAPI(null,slotPath);
        if(!slot.id)throw new Error('No clip slot exists in the selected scene');
        if(Number(valueOf(slot.get('has_clip'))))throw new Error('That scene already has a clip on this track — select an empty scene');
        var snapshot=core.copy(playbackPattern),notes=[];
        for(var i=0;i<snapshot.notes.length;i++){var n=snapshot.notes[i];notes.push({pitch:n.pitch,start_time:n.start,duration:n.duration,velocity:n.velocity,mute:0,probability:1});}
        slot.call('create_clip',snapshot.lengthBeats);created=true;
        var clip=new LiveAPI(null,slotPath+' clip');
        if(!clip.id)throw new Error('Live did not return the new clip');
        clip.call('add_new_notes',JSON.stringify({notes:notes}));
        clip.set('name','Submotif · '+rootNames[committed.root]+' '+scaleNames[committed.scale]+' · '+styleNames[committed.style]);
        clip.set('loop_start',0);clip.set('loop_end',snapshot.lengthBeats);clip.set('looping',1);
        clip.set('signature_numerator',4);clip.set('signature_denominator',4);
        // Do not launch or overwrite clips. Let its notes pass when the user launches it.
        output(0);ui('output','set',0);
        status('Clip created on this track in scene '+(sceneIndex+1)+' — launch it to play');
    } catch(e) {status((created?'Clip created, but could not finish: ':'')+String(e.message||e));}
}
