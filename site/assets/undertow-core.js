/* Undertow's portable ES5 Max engine. No pattern banks or copied basslines.
   See ../docs/MUSIC_THEORY.md for research and authored musical heuristics. */
var UndertowCore = (function () {
    var scales = [[0,2,3,5,7,8,10],[0,2,3,5,7,9,10],[0,1,3,5,7,8,10],[0,2,4,5,7,9,11],[0,3,5,7,10]];
    function clamp(x,a,b) { return Math.max(a,Math.min(b,x)); }
    function finite(x,fallback) { return typeof x === 'number' && isFinite(x) ? x : fallback; }
    function copy(x) { return JSON.parse(JSON.stringify(x)); }
    function defaults() { return {root:0,scale:0,style:0,bars:1,octave:1,density:0.55,swing:0.12,gate:0.65,movement:0.35,rootOnly:false,anchorRoot:true,octaveJumps:true}; }
    function sanitise(input) {
        var s=defaults(),k; input=input||{};
        for(k in s) if(input.hasOwnProperty(k)) s[k]=input[k];
        s.root=Math.round(clamp(finite(s.root,0),0,11));
        s.scale=Math.round(clamp(finite(s.scale,0),0,4));
        s.style=Math.round(clamp(finite(s.style,0),0,5));
        s.octave=Math.round(clamp(finite(s.octave,1),0,2));
        if([1,2,4,8].indexOf(s.bars)<0)s.bars=1;
        s.density=clamp(finite(s.density,0.55),0,1);s.swing=clamp(finite(s.swing,0.12),0,1);
        s.gate=clamp(finite(s.gate,0.65),0,1);s.movement=clamp(finite(s.movement,0.35),0,1);
        s.rootOnly=!!s.rootOnly;s.anchorRoot=!!s.anchorRoot;s.octaveJumps=!!s.octaveJumps;
        return s;
    }
    function rng(seed) {
        var x=(seed>>>0)||0x9e3779b9;
        var unit=function(){x^=x<<13;x^=x>>>17;x^=x<<5;return(x>>>8)/16777216;};
        unit.integer=function(count){return Math.floor(unit()*count);};
        return unit;
    }
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
    // Match Settings' float32 boundary without requiring Math.fround or typed
    // arrays in Max's ES5 runtime. Controls are already finite and in [0,1].
    function floatUnit(x) {
        if(x===0||x===1)return x;
        var exponent=Math.floor(Math.log(x)/Math.LN2);
        var spacing=Math.pow(2,Math.max(-149,exponent-23));
        var scaled=x/spacing,lower=Math.floor(scaled),fraction=scaled-lower;
        return (lower+(fraction>0.5||(fraction===0.5&&lower%2!==0)?1:0))*spacing;
    }
    var epsilon=1e-9;
    function basePitch(s){return(s.octave+2)*12+s.root;}
    function third(s){return s.scale===3?4:3;}
    function relativePitch(pitch,s){return((pitch-s.root)%12+12)%12;}
    function belongsToScale(pitch,s){return contains(scales[s.scale],relativePitch(pitch,s));}
    function triadic(pitch,s){var pc=relativePitch(pitch,s);return pc===0||pc===third(s)||pc===7;}
    function gridStep(start){return Math.floor(start*4+1e-7);}
    function onset(step,s){return step*0.25+(step%2?0.125*s.swing:0);}
    function roundCount(n){return Math.floor(n+0.5);}
    function zeros(){var a=[];for(var i=0;i<16;i++)a.push(0);return a;}
    function sortNotes(notes){notes.sort(function(a,b){return a.start-b.start;});}
    function intersects(a,b){return a.start<b.start+b.duration-epsilon&&b.start<a.start+a.duration-epsilon;}
    // Faithful counterpart of Source/PatternEngine.cpp: same arithmetic,
    // draw ordering, motif development and lock precedence; no pattern bank.
    function weightedOrder(input,random) {
        var weights=input.slice(0),order=[];
        for(var pick=0;pick<16;pick++) {
            var sum=0,i;
            for(i=0;i<16;i++)sum+=weights[i];
            if(sum<=0)break;
            var draw=random()*sum,selected=-1;
            for(i=0;i<16;i++) {
                if(weights[i]<=0)continue;
                selected=i;draw-=weights[i];if(draw<0)break;
            }
            order.push(selected);weights[selected]=0;
        }
        return order;
    }
    function makeRhythm(s,random) {
        var mask=zeros(),weights=zeros(),budget=0,order,i,step;
        if(s.style===2) {
            mask[2]=mask[10]=true;weights[6]=weights[14]=1;
            order=weightedOrder(weights,random);
            var extras=roundCount(2*s.density);
            for(i=0;i<extras;i++)mask[order[i]]=true;
            weights=zeros();
            for(step=3;step<16;step+=2)weights[step]=step>=11?2:0.5;
            order=weightedOrder(weights,random);
            var count=s.density>0.75?roundCount((s.density-0.75)*8):0;
            for(i=0;i<count;i++)mask[order[i]]=true;
            return mask;
        }
        if(s.style===1) {
            mask[0]=mask[6]=true;budget=2+roundCount(3*s.density);
            for(step=1;step<8;step++)if(step!==6)weights[step]=step===2?2.2:(step===3||step===7?1.7:0.65);
            order=weightedOrder(weights,random);
            for(i=0;i<budget-2;i++)mask[order[i]]=true;
            for(i=0;i<8;i++)mask[i+8]=mask[i];
            return mask;
        }
        mask[0]=mask[8]=true;
        if(s.style===4){mask[2]=mask[10]=true;budget=4+roundCount(8*s.density);}
        else if(s.style===0)budget=2+roundCount(6*s.density);
        else if(s.style===3)budget=4+roundCount(7*s.density);
        else budget=3+roundCount(6*s.density);
        for(step=0;step<16;step++) {
            if(mask[step])continue;
            var beat=step%4===0,eighth=step%4===2;
            if(s.style===0)weights[step]=eighth?2:(step%4===3?1.5:(beat?0.55:0.35));
            else if(s.style===3)weights[step]=step%2?1.4:1;
            else if(s.style===4)weights[step]=step%2?0.18:5;
            else weights[step]=step%4===3?2.5:(eighth?1.3:(beat?0.5:0.8));
            if(step>=12&&s.style!==4)weights[step]*=1.15;
        }
        order=weightedOrder(weights,random);
        var mandatory=s.style===4?4:2;
        for(i=0;i<budget-mandatory;i++)mask[order[i]]=true;
        return mask;
    }
    function nearestTriad(pitch,s) {
        var base=basePitch(s),choices=[base,base+third(s),base+7,base+12];
        var best=base,distance=Math.abs(pitch-base);
        for(var i=0;i<choices.length;i++) {
            var p=choices[i];
            if(p>71||(s.rootOnly&&p%12!==s.root)||(!s.octaveJumps&&p===base+12))continue;
            if(Math.abs(p-pitch)<distance){best=p;distance=Math.abs(p-pitch);}
        }
        return best;
    }
    function fitDurations(notes,length) {
        sortNotes(notes);
        for(var i=0;i<notes.length;i++) {
            var n=notes[i];if(n.locked)continue;
            var next=i+1<notes.length?notes[i+1].start:length;
            n.duration=Math.max(0.0011,Math.min(n.duration,next-n.start));
        }
    }
    function resolves(notes,i,length,s) {
        var n=notes[i],target=notes[(i+1)%notes.length],interval=Math.abs(target.pitch-n.pitch);
        var gap=target.start-n.start+(i+1===notes.length?length:0);
        return gridStep(n.start)%2===1&&n.duration<=0.20+epsilon&&triadic(target.pitch,s)&&interval>=1&&interval<=2&&gap<=0.5+epsilon;
    }
    function resolveConnectors(notes,length,s) {
        for(var i=0;i<notes.length;i++)if(!notes[i].locked&&!triadic(notes[i].pitch,s)&&!resolves(notes,i,length,s))notes[i].pitch=nearestTriad(notes[i].pitch,s);
    }
    function articulate(notes,s,random,accentStep) {
        var centreVelocity=96+random.integer(7)-3,i,n;
        for(i=0;i<notes.length;i++) {
            n=notes[i];var step=gridStep(n.start),anchor=step%8===0,beat=step%4===0;
            var accent=s.style===3&&step%8===accentStep;
            var velocity=centreVelocity+(anchor?10:(beat?4:(step%2?-9:-2)));
            if(accent)velocity+=13;
            n.velocity=clamp(velocity+random.integer(5)-2,48,123);
            var duration=0.1+0.25*s.gate;
            if(s.style===0)duration=anchor?0.35+0.85*s.gate:0.13+0.30*s.gate;
            else if(s.style===1)duration=0.065+0.19*s.gate;
            else if(s.style===2)duration=step%2?0.045+0.11*s.gate:0.08+0.32*s.gate;
            else if(s.style===3)duration=accent?0.12+0.40*s.gate:0.065+0.24*s.gate;
            else if(s.style===4)duration=step%2?0.05+0.13*s.gate:0.12+0.37*s.gate;
            else duration=anchor?0.13+0.30*s.gate:0.06+0.18*s.gate;
            var next=i+1<notes.length?notes[i+1].start:4,room=next-n.start;
            n.duration=Math.max(0.04,Math.min(duration,room*(0.74+0.20*s.gate)));
        }
        if(s.style===1)for(i=0;i<notes.length;i++) {
            n=notes[i];if(n.start<2)continue;
            for(var j=0;j<notes.length;j++)if(Math.abs(notes[j].start+2-n.start)<epsilon){n.velocity=notes[j].velocity;n.duration=notes[j].duration;break;}
        }
    }
    function addConnectors(notes,s,random) {
        if(s.rootOnly||s.movement<=0)return;
        var base=basePitch(s),ceiling=Math.min(base+(s.octaveJumps?12:11),71);
        var budget=Math.floor(2*s.movement);
        if(random()<2*s.movement-budget)budget++;
        var weights=zeros(),i,step;
        for(i=0;i<notes.length;i++){step=gridStep(notes[i].start);if(step%2===0)weights[step]=step===0||step>=12?2:1;}
        var targets=weightedOrder(weights,random);
        for(var t=0;t<targets.length;t++) {
            if(budget<=0)break;
            var targetStep=targets[t],connectorStep=(targetStep+15)%16,start=onset(connectorStep,s);
            if(s.anchorRoot&&start<notes[0].start)continue;
            var target=null;
            for(i=0;i<notes.length;i++)if(gridStep(notes[i].start)===targetStep){target=notes[i];break;}
            if(target===null||!triadic(target.pitch,s))continue;
            var choices=[];
            for(var pitch=base;pitch<=ceiling;pitch++) {
                var distance=Math.abs(pitch-target.pitch);
                if(distance>=1&&distance<=2&&belongsToScale(pitch,s)&&!triadic(pitch,s))choices.push(pitch);
            }
            if(choices.length===0)continue;
            var existing=-1;
            for(i=0;i<notes.length;i++)if(gridStep(notes[i].start)===connectorStep){existing=i;break;}
            if(existing<0&&notes.length>=12)continue;
            pitch=choices[random.integer(choices.length)];
            var connector={start:start,duration:0.07+0.08*s.gate,pitch:pitch,velocity:Math.max(48,target.velocity-22),locked:false};
            if(existing>=0)notes[existing]=connector;else notes.push(connector);
            budget--;sortNotes(notes);
        }
        fitDurations(notes,4);resolveConnectors(notes,4,s);
    }
    function makeMotif(s,random) {
        var mask=makeRhythm(s,random),base=basePitch(s),upper=base+12,canUpper=s.octaveJumps&&upper<=71;
        var colour=s.rootOnly?base:base+(random()<0.68?7:third(s));
        var answer=random()<0.18+0.70*s.movement;
        var upperRole=canUpper&&random()<(s.style===4?1:(s.style===1?0.50:0.25+0.30*s.movement));
        var pivot=random()<0.5?4:6,finalHalfStep=0,accentStep=0,step;
        for(step=0;step<8;step++)if(mask[step])finalHalfStep=step;
        var accentOptions=[];
        for(step=1;step<8;step++)if(mask[step])accentOptions.push(step);
        if(accentOptions.length)accentStep=accentOptions[random.integer(accentOptions.length)];
        var notes=[];
        for(step=0;step<16;step++) {
            if(!mask[step])continue;
            var pitch=base,half=step%8;
            if(s.style===1){if(answer&&half===finalHalfStep)pitch=upperRole?upper:colour;}
            else if(s.style===4){if(upperRole&&step%4>=2)pitch=upper;else if(answer&&half===4)pitch=colour;}
            else if(s.style===2){if(answer&&step>=14)pitch=upperRole?upper:colour;}
            else if(s.style===5){if(answer&&step%4===3)pitch=colour;if(half===6&&(upperRole||answer))pitch=upperRole?upper:colour;}
            else {if(answer&&half>=pivot)pitch=colour;if(s.style===3&&upperRole&&s.movement>0.6&&half===accentStep)pitch=upper;}
            notes.push({start:onset(step,s),duration:0.1,pitch:pitch,velocity:96,locked:false});
        }
        if(!s.anchorRoot&&!s.rootOnly&&random()<0.25+0.40*s.movement)notes[0].pitch=colour;
        if(s.anchorRoot)notes[0].pitch=base;
        articulate(notes,s,random,accentStep);addConnectors(notes,s,random);
        return notes;
    }
    function changedTriadPitch(current,s,random) {
        var base=basePitch(s),possible=[base,base+third(s),base+7,base+12],choices=[];
        for(var i=0;i<possible.length;i++) {
            var pitch=possible[i];
            if(pitch===current||pitch>71||(!s.octaveJumps&&pitch===base+12)||(s.rootOnly&&pitch%12!==s.root))continue;
            choices.push(pitch);
        }
        return choices.length?choices[random.integer(choices.length)]:current;
    }
    function response(motif,s,random,larger) {
        var notes=copy(motif),edits=larger&&s.movement>0.55?2:1,available=[],i;
        for(i=0;i<notes.length;i++)if(notes[i].start>=2.5&&triadic(notes[i].pitch,s))available.push(i);
        if(!available.length)for(i=0;i<notes.length;i++)if(notes[i].start>=2&&triadic(notes[i].pitch,s))available.push(i);
        for(var edit=0;edit<edits&&available.length;edit++) {
            var at=random.integer(available.length),index=available[at],n=notes[index];available.splice(at,1);
            var preceding=notes[(index+notes.length-1)%notes.length],original=n.pitch;
            if(triadic(preceding.pitch,s)&&random()<0.35+0.55*s.movement)n.pitch=changedTriadPitch(n.pitch,s,random);
            if(n.pitch===original){n.velocity=clamp(n.velocity+(random()<0.5?-7:7),48,123);n.duration*=random()<0.5?0.8:1.1;}
        }
        if(s.style!==2&&random()<0.15+0.45*s.movement) {
            for(i=notes.length-1;i>=0;i--) {
                var step=gridStep(notes[i].start);
                if(step<12||step%4===0||!triadic(notes[i].pitch,s))continue;
                if(s.style===4&&step%2===0)continue;
                var destination=step===15?14:step+1,occupied=false;
                for(var j=0;j<notes.length;j++)if(gridStep(notes[j].start)===destination){occupied=true;break;}
                if(occupied)continue;
                notes[i].start=onset(destination,s);break;
            }
        }
        fitDurations(notes,4);resolveConnectors(notes,4,s);return notes;
    }
    function generate(input,seed,previous,variation) {
        var s=sanitise(input),keys=['density','swing','gate','movement'],i;
        for(i=0;i<keys.length;i++)s[keys[i]]=floatUnit(s[keys[i]]);
        seed=seed>>>0;
        var random=rng(seed),length=s.bars*4,locked=[],n;
        if(!valid(previous))previous=null;
        if(previous!==null)for(i=0;i<previous.notes.length;i++) {
            n=previous.notes[i];if(n.locked&&n.start+n.duration<=length+epsilon)locked.push(copy(n));
        }
        var compatible=!!variation&&previous!==null&&previous.notes.length>0&&previous.lengthBeats===length,prior=[];
        if(compatible) {
            prior=copy(previous.notes);
            var base=basePitch(s),ceiling=Math.min(base+(s.octaveJumps?12:11),71);
            for(i=0;i<prior.length;i++) {
                n=prior[i];if(n.locked)continue;
                if(n.pitch<base||n.pitch>ceiling||!belongsToScale(n.pitch,s)||(s.rootOnly&&n.pitch%12!==s.root)
                    ||Math.abs(n.start-onset(gridStep(n.start),s))>1e-6
                    ||(!triadic(n.pitch,s)&&!resolves(prior,i,length,s)))compatible=false;
            }
            if(prior.length&&s.anchorRoot&&!prior[0].locked&&prior[0].pitch!==base)compatible=false;
        }
        if(compatible) {
            var editable=[];
            for(i=0;i<prior.length;i++)if(!prior[i].locked&&prior[i].start%4>=2&&triadic(prior[i].pitch,s))editable.push(i);
            var budget=Math.max(1,Math.ceil(prior.length*0.12));
            for(var edit=0;edit<budget&&editable.length;edit++) {
                var slot=random.integer(editable.length),index=editable[slot];editable.splice(slot,1);
                n=prior[index];var preceding=prior[(index+prior.length-1)%prior.length],oldPitch=n.pitch;
                if(triadic(preceding.pitch,s)&&random()<0.35+0.55*s.movement)n.pitch=changedTriadPitch(n.pitch,s,random);
                if(n.pitch===oldPitch) {
                    n.velocity=clamp(n.velocity+(random()<0.5?-6:6),48,123);
                    var next=index+1<prior.length?prior[index+1].start:length;
                    n.duration=Math.min(Math.max(0.0011,n.duration*(random()<0.5?0.85:1.1)),Math.max(n.duration,(next-n.start)*0.94));
                }
            }
            return {notes:prior,lengthBeats:length,seed:seed};
        }
        var motif=makeMotif(s,random),answerB=response(motif,s,random,false),answerC=response(motif,s,random,true);
        var alternating=random()<0.5,candidates=[];
        for(var bar=0;bar<s.bars;bar++) {
            var selected=motif;
            if(bar%4===3)selected=answerC;else if(bar%2===1&&(alternating||s.bars===2))selected=answerB;
            for(i=0;i<selected.length;i++){n=copy(selected[i]);n.start+=bar*4;candidates.push(n);}
        }
        var assembled=locked.slice(0);
        for(i=0;i<candidates.length;i++) {
            if(assembled.length>=128)break;
            n=candidates[i];n.duration=Math.min(n.duration,length-n.start);
            var reject=false;
            for(var k=0;k<locked.length;k++) {
                var kept=locked[k];
                if(n.start>=kept.start-epsilon&&n.start<kept.start+kept.duration-epsilon){reject=true;break;}
                if(intersects(n,kept))n.duration=Math.min(n.duration,kept.start-n.start);
            }
            if(!reject&&n.duration>=0.0011)assembled.push(n);
        }
        fitDurations(assembled,length);
        if(assembled.length&&s.anchorRoot&&!assembled[0].locked)assembled[0].pitch=basePitch(s);
        resolveConnectors(assembled,length,s);
        return {notes:assembled,lengthBeats:length,seed:seed};
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
