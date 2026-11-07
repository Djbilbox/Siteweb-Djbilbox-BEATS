import json,re,sys,urllib.request
UA={'User-Agent':'Mozilla/5.0','Accept-Language':'en'}
import subprocess
def get(u,data=None):
    cmd=['curl','-sL','-A','Mozilla/5.0','-H','Accept-Language: en']
    if data is not None: cmd+=['-H','Content-Type: application/json','--data-binary','@-']
    r=subprocess.run(cmd+[u],input=(json.dumps(data).encode() if data is not None else None),capture_output=True,timeout=60)
    return r.stdout.decode('utf-8','ignore')
def initial(html):
    m=re.search(r'var ytInitialData\s*=\s*(\{.*?\});</script>',html,re.S); return json.loads(m.group(1)) if m else {}
def walk(o,key):
    if isinstance(o,dict):
        for k,v in o.items():
            if k==key: yield v
            yield from walk(v,key)
    elif isinstance(o,list):
        for v in o: yield from walk(v,key)
def vids(o):
    out=[]
    for r in walk(o,'playlistVideoRenderer'):
        t=''.join(x.get('text','') for x in r.get('title',{}).get('runs',[]))
        out.append((r['videoId'],t))
    for r in walk(o,'lockupViewModel'):
        if r.get('contentType')=='LOCKUP_CONTENT_TYPE_VIDEO':
            t=next(walk(r,'title'),{}); t=t.get('content','') if isinstance(t,dict) else t
            out.append((r['contentId'],t))
    tok=[c['token'] for ci in walk(o,'continuationItemRenderer') for c in walk(ci,'continuationCommand')]
    return out,tok
res={}
html=get('https://www.youtube.com/@Humpireplugins/playlists')
key=re.search(r'"INNERTUBE_API_KEY":"([^"]+)"',html).group(1)
ver=re.search(r'"INNERTUBE_CLIENT_VERSION":"([^"]+)"',html).group(1)
pls={}
d=initial(html)
for r in walk(d,'lockupViewModel'):
    pid=r.get('contentId'); 
    t=next(walk(r,'title'),{}); t=t.get('content') if isinstance(t,dict) else t
    if pid: pls[pid]=t
for pid,title in pls.items():
    if not pid.startswith('PL'): continue
    all_=[]; o=initial(get('https://www.youtube.com/playlist?list='+pid))
    v,tok=vids(o); all_+=v
    while tok:
        o=json.loads(get(f'https://www.youtube.com/youtubei/v1/browse?key={key}',{'context':{'client':{'clientName':'WEB','clientVersion':ver,'hl':'en'}},'continuation':tok[0]}))
        v,tok=vids(o); all_+=v
    res[pid]={'title':title,'videos':all_}
    print(pid,len(all_),title,file=sys.stderr)
json.dump(res,open(sys.argv[1],'w',encoding='utf-8'),ensure_ascii=False,indent=1)
