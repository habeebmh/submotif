/* Undertow's portable ES5 Max engine. No pattern banks or copied basslines.
   See ../docs/MUSIC_THEORY.md for research and authored musical heuristics. */
var UndertowCore = (function () {
    var scales = [[0,2,3,5,7,8,10],[0,2,3,5,7,9,10],[0,1,3,5,7,8,10],[0,2,4,5,7,9,11],[0,3,5,7,10]];
    function clamp(x,a,b) { return Math.max(a,Math.min(b,x)); }
    function finite(x,fallback) { return typeof x === 'number' && isFinite(x) ? x : fallback; }
    function copy(x) { return JSON.parse(JSON.stringify(x)); }
    function defaults() { return {root:0,scale:0,style:0,bars:2,octave:1,density:0.55,swing:0.12,gate:0.65,movement:0.35,rootOnly:false,anchorRoot:true,octaveJumps:true}; }
    function sanitise(input) {
        var s=defaults(),k; input=input||{};
        for(k in s) if(input.hasOwnProperty(k)) s[k]=input[k];
        s.root=Math.round(clamp(finite(s.root,0),0,11));
        s.scale=Math.round(clamp(finite(s.scale,0),0,4));
        s.style=Math.round(clamp(finite(s.style,0),0,5));
        s.octave=Math.round(clamp(finite(s.octave,1),0,2));
        if([1,2,4,8].indexOf(s.bars)<0)s.bars=2;
        s.density=clamp(finite(s.density,0.55),0,1);s.swing=clamp(finite(s.swing,0.12),0,1);
        s.gate=clamp(finite(s.gate,0.65),0,1);s.movement=clamp(finite(s.movement,0.35),0,1);
        s.rootOnly=!!s.rootOnly;s.anchorRoot=!!s.anchorRoot;s.octaveJumps=!!s.octaveJumps;
        return s;
    }
    function rng(seed) { var x=(seed>>>0)||1; return function(){x^=x<<13;x^=x>>>17;x^=x<<5;return(x>>>0)/4294967296;}; }
    function valid(p) {
        if(!p || !Array.isArray(p.notes) || p.notes.length<1 || p.notes.length>128 || [4,8,16,32].indexOf(p.lengthBeats)<0)return false;
        var end=0;
        for(var i=0;i<p.notes.length;i++) {
            var n=p.notes[i];
            if(!n || typeof n.start!=='number'||typeof n.duration!=='number'||typeof n.pitch!=='number'||typeof n.velocity!=='number'||!isFinite(n.start)||!isFinite(n.duration)||n.start<0||n.duration<=0||n.start+1e-8<end||n.start+n.duration>p.lengthBeats+1e-8||!isFinite(n.pitch)||n.pitch!==Math.floor(n.pitch)||n.pitch<0||n.pitch>127||!isFinite(n.velocity)||n.velocity!==Math.floor(n.velocity)||n.velocity<1||n.velocity>127)return false;
            end=n.start+n.duration;
        }
        return true;
    }
    function contains(a,x) { return a.indexOf(x)>=0; }
    function samePlayback(a,b) {
        if(!a||!b||a.lengthBeats!==b.lengthBeats||a.notes.length!==b.notes.length)return false;
        for(var i=0;i<a.notes.length;i++) {
            var x=a.notes[i],y=b.notes[i];
            if(x.start!==y.start||x.duration!==y.duration||x.pitch!==y.pitch||x.velocity!==y.velocity)return false;
        }
        return true;
    }
    function onsetWeight(s,step,gap,run) {
        var beat=step%4===0,eighth=step%4===2,odd=step%2===1,w;
        // Probabilities depend on metric role, never a complete stored rhythm.
        if(s.style===0)w=beat?0.62:eighth?0.95:0.38;
        else if(s.style===1)w=beat?0.78:eighth?0.98:0.84;
        else if(s.style===2)w=eighth?1.45:odd?0.06:0.0;
        else if(s.style===3)w=beat?0.88:eighth?0.75:0.92;
        else if(s.style===4)w=odd?0.27:1.15;
        else w=beat?0.72:eighth?0.85:0.72;
        if(step%16>=13 && s.style!==2)w*=1.12;
        if(run>=3 && s.style!==1 && s.style!==3)w*=0.48;
        if(gap>=6)w*=1.3;
        return clamp(w*(0.08+0.9*s.density),0,0.97);
    }
    function choosePitch(s,random,previous,ordinal) {
        var base=(s.octave+2)*12+s.root,degree=0,collection=scales[s.scale];
        if(!s.rootOnly && random()<s.movement) {
            var weights=[],sum=0;
            for(var j=0;j<collection.length;j++) {
                var d=collection[j],dist=Math.abs(base+d-previous);
                var w=(d===0?4:d===7?2.7:d===3||d===4?1.8:1.0)/(1+dist*0.13);
                weights.push(w);sum+=w;
            }
            var draw=random()*sum;
            for(j=0;j<weights.length;j++){draw-=weights[j];if(draw<=0){degree=collection[j];break;}}
        }
        var octave=0;
        if(s.octaveJumps) {
            var chance=(s.style===4?0.45:s.style===5?0.24:0.12)*(0.3+0.7*s.movement);
            if(random()<chance && base+degree+12<=71)octave=12;
        }
        if(ordinal===0 && s.anchorRoot){degree=0;octave=0;}
        return clamp(base+degree+octave,0,127);
    }
    function generate(input,seed,previous,variation) {
        var s=sanitise(input),random=rng(seed),length=s.bars*4,notes=[],protectedNotes=[],i;
        var prevValid=valid(previous),base=(s.octave+2)*12+s.root;
        if(prevValid)for(i=0;i<previous.notes.length;i++) {
            var n=previous.notes[i],pc=(n.pitch-s.root+120)%12;
            var allowed=(!s.rootOnly||pc===0)&&contains(scales[s.scale],pc)&&n.pitch>=base&&n.pitch<=71&&(s.octaveJumps||n.pitch<base+12);
            if(n.start+n.duration<=length+1e-8 && (n.locked || (variation&&allowed&&random()<0.62))) protectedNotes.push(copy(n));
        }
        var gap=0,run=0,last=base;
        for(var step=0;step<s.bars*16;step++) {
            var selected=random()<onsetWeight(s,step,gap,run);
            if(s.style===2 && step%4===0)selected=false;
            if(!selected){gap++;run=0;continue;}
            var start=step*0.25+(step%2?0.125*s.swing:0),blocked=false;
            for(i=0;i<protectedNotes.length;i++)if(start>=protectedNotes[i].start-1e-8 && start<protectedNotes[i].start+protectedNotes[i].duration-1e-8)blocked=true;
            if(blocked)continue;
            var pitch=choosePitch(s,random,last,notes.length),accent=step%4===0?14:step%4===2?7:0;
            var velocity=Math.round(clamp(83+accent+(random()-0.5)*22,45,122));
            if(s.style===5&&step%2)velocity=Math.max(45,velocity-13);
            var duration=Math.max(0.045,(0.25+(random()<0.25?0.25:0)+(s.style===0&&random()<0.18?0.5:0))*s.gate);
            notes.push({start:start,duration:duration,pitch:pitch,velocity:velocity,locked:false});
            last=pitch;gap=0;run++;
        }
        if(notes.length===0 && protectedNotes.length===0)notes.push({start:s.style===2?0.5:0,duration:Math.max(0.045,0.25*s.gate),pitch:base,velocity:100,locked:false});
        notes=notes.concat(protectedNotes).sort(function(a,b){return a.start-b.start;});
        // Locked notes have priority; preceding generated notes are shortened safely.
        var out=[],remainingLocks=0;
        for(i=0;i<protectedNotes.length;i++)if(protectedNotes[i].locked)remainingLocks++;
        for(i=0;i<notes.length;i++) {
            n=notes[i];
            if(n.locked)remainingLocks--;
            // Preserve room for every remaining lock, including late phrase
            // notes when retained variation and fresh candidates exceed 128.
            if(!n.locked && out.length>=128-remainingLocks)continue;
            var prior=out.length?out[out.length-1]:null;
            if(prior && n.start<prior.start+prior.duration-1e-8) {
                if(prior.locked || (!n.locked && protectedNotes.indexOf(prior)>=0))continue;
                prior.duration=n.start-prior.start;
                if(prior.duration<0.001)out.pop();
            }
            n.duration=Math.min(n.duration,length-n.start);
            if(n.duration>=0.001)out.push(n);
        }
        if(out.length && s.anchorRoot && !out[0].locked)out[0].pitch=base;
        return {notes:out,lengthBeats:length,seed:seed>>>0};
    }
    function vlq(n) { var a=[n&127];while((n=Math.floor(n/128))>0)a.unshift((n&127)|128);return a; }
    function be32(n) { return [(n>>>24)&255,(n>>>16)&255,(n>>>8)&255,n&255]; }
    function midiFile(p) {
        if(!valid(p))throw new Error('Cannot export invalid pattern');
        var ev=[],i;
        for(i=0;i<p.notes.length;i++) {
            var n=p.notes[i],on=Math.round(n.start*960),off=Math.max(on+1,Math.round((n.start+n.duration)*960));
            ev.push({t:on,b:[144,n.pitch,n.velocity],order:1});ev.push({t:off,b:[128,n.pitch,0],order:0});
        }
        ev.sort(function(a,b){return a.t-b.t || a.order-b.order;});
        // 4/4 signature, no tempo event: inherit the destination project's tempo.
        var track=[0,255,88,4,4,2,24,8],last=0;
        for(i=0;i<ev.length;i++){track=track.concat(vlq(ev[i].t-last),ev[i].b);last=ev[i].t;}
        track=track.concat(vlq(Math.max(0,Math.round(p.lengthBeats*960)-last)),[255,47,0]);
        return [77,84,104,100,0,0,0,6,0,0,0,1,3,192,77,84,114,107].concat(be32(track.length),track);
    }
    function Scheduler() {this.last=null;this.active=-1;this.revision=-1;this.running=false;}
    Scheduler.prototype.flush=function(){var out=[];if(this.active>=0)out.push({pitch:this.active,velocity:0,on:false});this.active=-1;this.running=false;this.last=null;return out;};
    Scheduler.prototype.process=function(p,ppq,playing,enabled,revision) {
        if(!playing||!enabled||!isFinite(ppq)||ppq<0||ppq>=1e12||!valid(p))return this.flush();
        var events=[],reset=!this.running||this.last===null||ppq<this.last-1e-8||ppq-this.last>0.5||revision!==this.revision;
        var phase=((ppq%p.lengthBeats)+p.lengthBeats)%p.lengthBeats,i,n;
        if(reset) {
            events=this.flush();
            for(i=0;i<p.notes.length;i++){n=p.notes[i];if(phase>=n.start-1e-8&&phase<n.start+n.duration-1e-8){events.push({pitch:n.pitch,velocity:n.velocity,on:true});this.active=n.pitch;break;}}
        } else {
            var due=[],cycle=Math.floor(this.last/p.lengthBeats),endCycle=Math.floor(ppq/p.lengthBeats);
            if(!isFinite(cycle)||!isFinite(endCycle)||endCycle<cycle||endCycle-cycle>4096)return this.flush();
            for(var c=cycle;c<=endCycle;c++)for(i=0;i<p.notes.length;i++) {
                n=p.notes[i];var on=c*p.lengthBeats+n.start,off=on+n.duration;
                if(on>this.last+1e-8&&on<=ppq+1e-8)due.push({time:on,pitch:n.pitch,velocity:n.velocity,on:true});
                if(off>this.last+1e-8&&off<=ppq+1e-8)due.push({time:off,pitch:n.pitch,velocity:0,on:false});
            }
            due.sort(function(a,b){return a.time-b.time||Number(a.on)-Number(b.on);});
            for(i=0;i<due.length;i++) {
                var e=due[i];
                if(e.on){if(this.active>=0)events.push({pitch:this.active,velocity:0,on:false});this.active=e.pitch;}
                else {if(this.active!==e.pitch)continue;this.active=-1;}
                events.push({pitch:e.pitch,velocity:e.velocity,on:e.on});
            }
        }
        this.last=ppq;this.running=true;this.revision=revision;return events;
    };
    return {scales:scales,defaults:defaults,sanitise:sanitise,generate:generate,valid:valid,samePlayback:samePlayback,midiFile:midiFile,Scheduler:Scheduler,copy:copy};
}());
if(typeof module!=='undefined' && module.exports)module.exports=UndertowCore;
