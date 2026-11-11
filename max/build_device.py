#!/usr/bin/env python3
"""Build a MIDI-effect AMXD using the envelope of Ableton's shipped template.
No undocumented executable payload: the ptch chunk contains an ordinary Max patch.
The companion JavaScript files intentionally remain adjacent and editable.
"""
import json
import struct
from pathlib import Path

HERE = Path(__file__).resolve().parent
boxes, lines = [], []
parameters = {}
def colour(hex):
    return [int(hex[i:i+2],16)/255 for i in (0,2,4)]+[1.0]

BG=colour('191c1e')
PANEL=colour('24282a')
LIME=colour('c5ed76')
TEXT=colour('f0f2ed')
MUTED=colour('a7afad')
DIM=colour('69716f')
LINE=colour('383e40')

def box(id, kind, rect, **extra):
    b={'id':id,'maxclass':kind,'patching_rect':rect,'numinlets':1,'numoutlets':0,**extra}
    boxes.append({'box':b})
    return id

def visible(id,kind,rect,**extra):
    extra.setdefault('varname',id)
    return box(id,kind,rect,presentation=1,presentation_rect=rect,**extra)

def obj(id,text,rect=None,ins=1,outs=1,**extra):
    return box(id,'newobj',rect or [20,220+len(boxes)*24,180,22],text=text,numinlets=ins,numoutlets=outs,**extra)

def wire(src,out,dst,inp=0,**extra):
    lines.append({'patchline':{'source':[src,out],'destination':[dst,inp],**extra}})

def label(id,text,rect,size=11,color=MUTED,**extra):
    return visible(id,'comment',rect,text=text,fontsize=size,fontname='Avenir Next',textcolor=color,**extra)

def param(id,title,initial,type=0,minimum=0,maximum=1,enum=None):
    p={'parameter_longname':title,'parameter_shortname':title,'parameter_type':type,
       'parameter_initial_enable':1,'parameter_initial':[initial],'parameter_order':0,
       'parameter_mmin':minimum,'parameter_mmax':maximum}
    if enum is not None:p['parameter_enum']=enum
    parameters[id]=[title,title,0]
    return {'valueof':p}

def menu(id,title,items,initial,x,width,y=50,advanced=False,parameter_name=None,hint=''):
    label('label-'+id,title,[x,y-19,width,17],hidden=int(advanced))
    visible(id,'live.menu',[x,y,width,22],varname=id,numoutlets=3,parameter_enable=1,
            saved_attribute_attributes=param(id,parameter_name or title,initial,2,0,len(items)-1,items),
            items=items,fontsize=11,fontname='Avenir Next',bgcolor=PANEL,activebgcolor=PANEL,
            textcolor=TEXT,activetextcolor=TEXT,hidden=int(advanced),hint=hint)
    p=obj('cfg-'+id,'prepend cfg '+id)
    wire(id,0,p);wire(p,0,'engine')

def slider(id,title,initial,x,width,y=113,advanced=False,parameter_name=None,hint=''):
    attrs=param(id,parameter_name or title,initial,0,0,100)
    attrs['valueof']['parameter_unitstyle']=5
    label('label-'+id,title,[x,y-22,width-35,18],color=TEXT,hidden=int(advanced))
    label('value-'+id,str(initial)+'%',[x+width-35,y-22,35,18],justification=2,hidden=int(advanced))
    visible(id,'live.slider',[x,y,width,18],varname=id,numoutlets=2,parameter_enable=1,
            saved_attribute_attributes=attrs,orientation=1,showname=0,shownumber=0,
            slidercolor=LINE,tricolor=LIME,trioncolor=LIME,tribordercolor=BG,
            focusbordercolor=LIME,textcolor=TEXT,hidden=int(advanced),hint=hint)
    p=obj('cfg-'+id,'prepend cfg '+id);wire(id,0,p);wire(p,0,'engine')

def toggle(id,text,initial,rect,method=None,advanced=False,parameter_name=None,hint=''):
    visible(id,'live.text',rect,varname=id,numoutlets=2,mode=1,text=text,texton=text,
            parameter_enable=1,saved_attribute_attributes=param(id,parameter_name or text,initial,2,0,1,['Off','On']),
            fontsize=10,fontname='Avenir Next',activebgcolor=PANEL,activebgoncolor=LIME,
            activetextcolor=MUTED,activetextoncolor=BG,rounded=3,hidden=int(advanced),hint=hint)
    p=obj('cfg-'+id,'prepend '+(method or 'cfg '+id));wire(id,0,p);wire(p,0,'engine')

def action(id,text,method,rect,accent=False,advanced=False,parameter_name=None,hint=''):
    # Max 9.1.3+ has a documented Visible (Not Stored) parameter mode for
    # transient controls: host-accessible, neither automated nor recalled.
    # https://docs.cycling74.com/userguide/parameter_mode/
    # https://docs.cycling74.com/reference/live.text/
    attrs=param(id,parameter_name or text,0,2,0,1,['Idle','Trigger'])
    attrs['valueof'].update(parameter_invisible=4,parameter_initial_enable=0,
                            parameter_visible_undoable=0)
    visible(id,'live.text',rect,varname='action_'+id,numoutlets=2,mode=0,transition=0,
            text=text,texton=text,parameter_enable=1,saved_attribute_attributes=attrs,
            fontsize=11,fontname='Avenir Next',activebgcolor=LIME if accent else PANEL,
            activebgoncolor=LIME,activetextcolor=BG if accent else TEXT,activetextoncolor=BG,
            bgcolor=BG,textcolor=DIM,bordercolor=LINE,rounded=3,hidden=int(advanced),hint=hint)
    msg=box('action-'+id,'message',[930,300+len(boxes)*20,110,22],text=method,numoutlets=1)
    wire(id,0,msg);wire(msg,0,'engine')

label('brand','SUBMOTIF',[16,5,140,25],18,TEXT,fontface=1)
label('edition','by Habeeb',[159,10,130,17],10,MUTED)
label('transport','Stopped',[295,10,65,17],10,MUTED,justification=2)
label('summary','Ready',[380,10,224,18],11,TEXT)
label('status','Click a note to keep it. Amber notes are locked.',[380,145,416,18],10,MUTED)
# Preserve parameter creation order and internal names for existing Live Sets.
menu('root','Key',['C','C#','D','D#','E','F','F#','G','G#','A','A#','B'],0,136,50)
menu('scale','Scale',['Minor','Dorian','Phrygian','Major','Minor Pent.'],0,196,102)
menu('style','Style',['Deep House','Rolling','Offbeat','Acid','Disco','Funk'],0,16,110)
menu('bars','Length',['1 bar','2 bars','4 bars','8 bars'],0,308,52,parameter_name='Bars')
# The patcher already supplies the drawer background. A foreground panel here
# can cover controls in Max's saved stacking order when the drawer is revealed.
visible('advanced-divider','panel',[999,10,1,149],bgcolor=LINE,border=0,rounded=0,
        hidden=1,ignoreclick=1,background=1)
label('advanced-title','Timing & pitch',[1008,9,238,19],11,TEXT,hidden=1)
menu('octave','Register',['0','1','2'],1,1008,110,y=112,advanced=True,
     parameter_name='Octave',hint='Base octave in Live notation. The Key menu sets its pitch class.')
slider('density','Density',55,16,162,hint='Fewer or more notes in the next generated phrase.')
slider('swing','Swing',12,1008,110,y=62,advanced=True,hint='Delay alternate sixteenth notes. Generate to apply.')
slider('gate','Note length',65,1134,110,y=62,advanced=True,parameter_name='Gate',
       hint='Shorter or longer notes. Generate to apply.')
slider('movement','Melody',35,198,162,parameter_name='Movement',
       hint='Less or more pitch movement in the next generated phrase.')
toggle('rootOnly','Root notes only',0,[1134,90,110,20],advanced=True,parameter_name='ROOT ONLY',
       hint='Use only the selected key\'s root pitch class for new notes.')
toggle('anchorRoot','Start on root',1,[1134,115,110,20],advanced=True,parameter_name='ROOT ANCHOR',
       hint='Start on the root note without moving its rhythmic position.')
toggle('octaveJumps','Octave jumps',1,[1134,140,110,20],advanced=True,parameter_name='OCTAVE JUMPS',
       hint='Allow octave changes while generating new notes.')
visible('roll','jsui',[380,40,416,100],filename='undertow-roll.js',numoutlets=1,
        jsarguments=[],parameter_enable=0,varname='roll',border=0,
        hint='Click a note to lock or unlock it. Locked notes stay exact when generating or changing a few notes.')
action('generate','Generate','generate',[816,40,168,24],True,parameter_name='GENERATE',
       hint='Generate a new bassline using the selected settings. Locked notes stay exact.')
action('vary','Change a few notes','vary',[816,69,168,22],parameter_name='Variation',
       hint='Keep this rhythm and most notes; change a few unlocked notes. Apply changed settings with Generate first.')
action('undo','Undo','undo',[704,9,42,20],hint='Restore the previous phrase or note locks.')
action('redo','Redo','redo',[752,9,44,20],hint='Restore the next phrase or note locks.')
action('clip','Create clip','createclip',[816,105,82,22],parameter_name='CREATE CLIP',
       hint='Create a clip in this track\'s empty slot in the selected Session scene.')
action('export','Save MIDI','beginexport',[904,105,80,22],parameter_name='SAVE MIDI',
       hint='Save the displayed notes as a MIDI file.')
toggle('output','Live MIDI on',1,[888,9,96,20],'output',parameter_name='MIDI ON',
       hint='Play generated notes through the next instrument when Live is playing. Off passes track MIDI through.')
next(b['box'] for b in boxes if b['box']['id']=='output')['text']='Live MIDI off'
action('panic','Stop notes','panic',[1008,140,110,20],advanced=True,parameter_name='Panic')
action('unlock','Unlock all','unlock',[614,9,84,20],parameter_name='Unlock notes',
       hint='Release every note lock; the current notes stay unchanged.')
next(b['box'] for b in boxes if b['box']['id']=='unlock')['hidden']=1
action('more','More controls','details',[864,142,120,20],hint='Show or hide timing and pitch controls.')

obj('engine','js undertow.js',[448,250,260,23],outs=4,varname='engine')
obj('midi-in','midiin',[20,250,60,22])
obj('thru','gate 1 0',[20,290,65,22],ins=2)
obj('midi-out','midiout',[20,340,60,22],outs=0)
wire('midi-in',0,'thru',1);wire('engine',2,'thru');wire('thru',0,'midi-out');wire('engine',0,'midi-out')
wire('engine',1,'roll');wire('roll',0,'engine')
obj('clock','metro 2 @active 1',[150,250,150,22])
obj('transport-reader','transport',[150,290,190,22],ins=2,outs=9)
wire('clock',0,'transport-reader')
# Transport fires right-to-left: read raw ticks and running state before bar outlet pulse.
obj('position-message','prepend position',[180,330,130,22]);wire('transport-reader',7,'position-message');wire('position-message',0,'engine')
obj('playing-message','prepend playing',[180,360,130,22]);wire('transport-reader',6,'playing-message');wire('playing-message',0,'engine')
box('pulse-message','message',[180,390,70,22],text='pulse',numoutlets=1);wire('transport-reader',0,'pulse-message');wire('pulse-message',0,'engine')
obj('this-device','live.thisdevice',[20,390,110,22],outs=3,varname='this-device')
box('ready-message','message',[20,420,50,22],text='ready',numoutlets=1);wire('this-device',0,'ready-message');wire('ready-message',0,'engine')
obj('active-message','prepend deviceactive',[20,450,160,22]);wire('this-device',1,'active-message');wire('active-message',0,'engine')
obj('free','freebang',[20,480,65,22]);box('free-message','message',[20,510,65,22],text='freebang',numoutlets=1);wire('free',0,'free-message');wire('free-message',0,'engine')
obj('save-dialog','savedialog',[930,260,100,22],outs=3,varname='save-dialog')
wire('engine',3,'save-dialog')
obj('export-message','prepend exportfile',[930,290,150,22]);wire('save-dialog',0,'export-message');wire('export-message',0,'engine')
obj('save-name','loadmess name Submotif.mid',[930,230,190,22]);wire('save-name',0,'save-dialog')

obj('state','pattr patternState @bindto engine @parameter_enable 1',[448,300,375,22],outs=3,
    varname='patternState',saved_attribute_attributes={'valueof':{
        'parameter_longname':'Pattern State','parameter_shortname':'Pattern State','parameter_type':3,
        'parameter_invisible':1,'parameter_order':100}},
    saved_object_attributes={'parameter_enable':1})
parameters['state']=['Pattern State','Pattern State',0]
parameters['parameterbanks']={'0':{'index':0,'name':'Groove','parameters':['density','swing','gate','movement','root','scale','style','bars']}}
parameters['inherited_shortname']=1

patch={'patcher':{
    'fileversion':1,'appversion':{'major':8,'minor':6,'revision':5,'architecture':'x64','modernui':1},
    'classnamespace':'box','rect':[80,80,1260,700],'openrect':[0,0,1000,169],
    'openinpresentation':1,'bglocked':1,'default_fontsize':11,'default_fontname':'Avenir Next',
    'bgcolor':BG,'editing_bgcolor':BG,'locked_bgcolor':BG,'presentation':1,'devicewidth':1000.0,
    'enablehscroll':0,'enablevscroll':0,'description':'SUBMOTIF by Habeeb — procedural MIDI basslines for any instrument.',
    'digest':'A musical bassline generator with native single-track MIDI output.','tags':'MIDI bass generator house disco funk',
    'boxes':boxes,'lines':lines,'parameters':parameters,'latency':0,
    'dependency_cache':[{'name':n,'bootpath':'.','patcherrelativepath':'.','type':'TEXT','implicit':1} for n in ['undertow.js','undertow-core.js','undertow-roll.js']],
    'project':{'version':1,'autoorganize':0,'hideprojectwindow':1,'showdependencies':1,'autolocalize':0,
               'contents':{'patchers':{},'code':{n:{'kind':'javascript','local':1} for n in ['undertow.js','undertow-core.js','undertow-roll.js']}},
               'searchpath':{},'amxdtype':1835887981,'readonly':0,'devpathtype':0,'devpath':'.'},
    'autosave':0}}
payload=(json.dumps(patch,indent=2,ensure_ascii=False)+'\n').encode('utf8')+b'\0'
header=b'ampf'+struct.pack('<I',4)+b'mmmmmeta'+struct.pack('<I',4)+b'\0\0\0\0ptch'+struct.pack('<I',len(payload))
(HERE/'Submotif.maxpat').write_text(json.dumps(patch,indent=2,ensure_ascii=False)+'\n')
(HERE/'Submotif.amxd').write_bytes(header+payload)
print(f'Built {HERE / "Submotif.amxd"} ({len(header)+len(payload)} bytes; MIDI effect).')
