/* Compact piano roll for Max's native mgraphics canvas. */
autowatch=0;inlets=1;outlets=1;
mgraphics.init();mgraphics.relative_coords=0;mgraphics.autofill=0;
var notes=[],length=8,head=-1,low=36,high=48,hits=[];
function pattern(value) {
    try {var p=JSON.parse(value);notes=p.notes;length=p.lengthBeats;low=127;high=0;
        for(var i=0;i<notes.length;i++){low=Math.min(low,notes[i].pitch);high=Math.max(high,notes[i].pitch);}
        if(high-low<12)high=low+12;
        mgraphics.redraw();
    }catch(e){}
}
function playhead(v){if(head!==v){head=v;mgraphics.redraw();}}
function colour(r,g,b,a){mgraphics.set_source_rgba(r,g,b,a===undefined?1:a);}
function rect(x,y,w,h){mgraphics.rectangle(x,y,w,h);mgraphics.fill();}
function text(t,x,y,size){mgraphics.select_font_face('Avenir Next');mgraphics.set_font_size(size);mgraphics.move_to(x,y);mgraphics.show_text(t);}
function noteName(p){return ['C','C#','D','D#','E','F','F#','G','G#','A','A#','B'][p%12]+(Math.floor(p/12)-2);}
function paint() {
    var w=box.rect[2]-box.rect[0],h=box.rect[3]-box.rect[1],left=31,top=19,bottom=h-7,gw=w-left-6,gh=bottom-top;
    colour(0.14,0.157,0.165);rect(0,0,w,h);hits=[];
    for(var beat=0;beat<=length;beat++) {
        var x=left+gw*beat/length;
        colour(beat%4===0?0.25:0.19,beat%4===0?0.28:0.21,beat%4===0?0.29:0.22);rect(x,top,1,gh);
        if(beat%4===0 && beat<length){colour(0.65,0.69,0.68);text(String(beat/4+1),x+3,13,10);}
    }
    for(var row=0;row<=4;row++){colour(0.19,0.21,0.22);rect(left,top+row*gh/4,gw,1);}
    colour(0.65,0.69,0.68);text(noteName(low),3,bottom,9);text(noteName(high),3,top+8,9);
    var nh=Math.max(4,Math.min(8,gh/(high-low+1)));
    for(var i=0;i<notes.length;i++) {
        var n=notes[i],nx=left+gw*n.start/length,ny=bottom-nh-(n.pitch-low)/(high-low)*(gh-nh),nw=Math.max(3,gw*n.duration/length);
        if(n.locked)colour(0.97,0.76,0.4);else colour(0.773,0.929,0.463,0.55+n.velocity/280);
        mgraphics.rectangle_rounded(nx,ny,nw,nh,2,2);mgraphics.fill();
        if(n.locked && nw>6){colour(0.2,0.16,0.09);rect(nx+2,ny+1,2,nh-2);}
        hits.push([nx,ny,nw,nh,i]);
    }
    if(head>=0){colour(0.9,0.98,0.83,0.8);rect(left+gw*head/length,top,1,gh);}
}
function onclick(x,y){for(var i=hits.length-1;i>=0;i--){var r=hits[i];if(x>=r[0]-2&&x<=r[0]+r[2]+2&&y>=r[1]-3&&y<=r[1]+r[3]+3){outlet(0,'locknote',r[4]);return;}}}
onclick.local=1;
function onresize(){mgraphics.redraw();}onresize.local=1;
