
from pathlib import Path
import sys, struct as S, hashlib, collections as C, json, re
u=lambda b,o:S.unpack_from('<H',b,o)[0]
d=lambda b,o:S.unpack_from('<I',b,o)[0]
def name(b,o):return b[o+1:o+1+b[o]].decode('cp1252',errors='replace')
def ne(b,n):
 h=b[n:]; seg=[]; imports=[name(h,u(h,0x2a)+u(h,u(h,0x28)+i*2)) for i in range(u(h,0x1e))]
 for i in range(u(h,0x1c)):
  l,z,f,m=S.unpack_from('<4H',h,u(h,0x22)+8*i); off=l<<u(h,0x32); size=(z or 65536) if l else 0; fix=[]
  if l and f&256:
   for j in range(u(b,off+size)):
    a,k,s,x,y=S.unpack_from('<BBHHH',b,off+size+2+j*8)
    label=imports[x-1] if k&3 in (1,2) and 1<=x<=len(imports) else ''
    target=name(h,u(h,0x2a)+y) if label and k&3==2 else y
    fix.append([a,k,s,x,y,label,target])
  seg.append(dict(id=i+1,offset=off,size=size,flags=hex(f),minalloc=m or 65536,fixups=fix))
 resources={}; q=u(h,0x24); re_end=u(h,0x26); align=u(h,q) if q and q<re_end else 0; q+=2
 while q+2<=re_end and u(h,q):
  typ,count=u(h,q),u(h,q+2); q+=8
  resources[str(typ) if typ&32768 else name(h,u(h,0x24)+typ)]=[list(S.unpack_from('<6H',h,q+i*12)) for i in range(count)]
  q+=count*12
 entries=[]; q=u(h,4); end=q+u(h,6); ordinal=1
 while q+2<=end and h[q]:
  count,kind=h[q:q+2];q+=2
  for _ in range(count):
   if kind==255: entries.append([ordinal,h[q+3],u(h,q+4),h[q]]);q+=6
   elif kind:entries.append([ordinal,kind,u(h,q+1),h[q]]);q+=3
   ordinal+=1
 return dict(cs_ip=[u(h,22),u(h,20)],ss_sp=[u(h,26),u(h,24)],dgroup=u(h,14),flags=hex(u(h,12)),os=h[54],segments=seg,modules=imports,resources=resources,resource_shift=align,entries=entries)
def atoms(f,start,end,depth=0):
 f.seek(start); out=[]
 while f.tell()+8<=end:
  off=f.tell(); size,typ=S.unpack('>I4s',f.read(8)); header=8
  if size==1:size=S.unpack('>Q',f.read(8))[0];header=16
  if size==0:size=end-off
  if size<header or off+size>end:break
  t=typ.decode('latin1'); row=[t,off,size]
  if t in ('moov','trak','mdia','minf','stbl') and depth<8:row.append(atoms(f,off+header,off+size,depth+1))
  elif t=='stsd':row.append(f.read(min(size-header,160)).hex())
  out.append(row); f.seek(off+size)
 return out

def media_index(f,base,size,nm):
 def read(off,n):
  if off<0 or n<0 or off+n>size:raise ValueError(f'{nm}: read outside extent')
  f.seek(base+off)
  return f.read(n)
 if nm.endswith('.MOV'):
  count=S.unpack('>I',read(0,4))[0]
  offsets=S.unpack(f'>{count}I',read(4,count*4))
  start=4+count*4
  assert start<offsets[0] and all(a<b for a,b in zip(offsets,offsets[1:]))
  movies=[]
  for moov in offsets:
   mdat_n,mdat_type=S.unpack('>I4s',read(start,8))
   moov_n,moov_type=S.unpack('>I4s',read(moov,8))
   assert mdat_type==b'mdat' and moov_type==b'moov' and start+mdat_n==moov
   tree=atoms(f,base+moov,base+moov+moov_n)
   def walk(rows):
    for row in rows:
     yield row
     if len(row)==4 and isinstance(row[3],list):yield from walk(row[3])
   codes=[]; chunks=[]
   for node in walk(tree):
    kind,where,length=node[:3]
    if kind=='stsd':
     desc=bytes.fromhex(node[3])
     if len(desc)>=16:codes.append(desc[12:16].decode('latin1'))
    if kind=='stco':
     q=where-base
     number=S.unpack('>I',read(q+12,4))[0]
     assert 16+number*4<=length
     vals=S.unpack(f'>{number}I',read(q+16,number*4))
     chunks.append(dict(count=number,in_mdat=sum(start+8<=v<moov for v in vals)))
   movies.append(dict(mdat=start,moov=moov,end=moov+moov_n,codecs=codes,chunks=chunks))
   start=moov+moov_n
  return dict(count=count,tail=size-start,movies=movies)
 assert read(16,4)==b'MM\0\0'
 count=S.unpack('>H',read(102,2))[0]
 offsets=S.unpack(f'>{count+1}I',read(104,4*(count+1)))
 assert 104+4*(count+1)<=offsets[0] and offsets[-1]==size
 assert all(a<=b for a,b in zip(offsets,offsets[1:]))
 return dict(count=count,offsets=offsets,index_offset=102,first_prefixes=[read(a,min(24,b-a)).hex(' ') for a,b in zip(offsets[:8],offsets[1:9])])

def inspect(iso):
 result={'iso':str(iso),'files':[],'NE':{},'media':{},'evidence':{}}
 with iso.open('rb') as f:
  result['sha256']=hashlib.file_digest(f,'sha256').hexdigest()
  def read(l,n):f.seek(l*2048);return f.read(n)
  p=read(16,2048); assert p[:7]==b'\x01CD001\x01'
  result['volume']=p[40:72].decode('ascii').strip()
  def walk(rec,dir=''):
   page=read(d(rec,2),d(rec,10));i=0
   while i<len(page):
    z=page[i]
    if not z:i=(i//2048+1)*2048;continue
    r=page[i:i+z];i+=z
    if r[33] in (0,1) and r[32]==1:continue
    nm=dir+r[33:33+r[32]].decode('ascii').split(';')[0].rstrip('.')
    l,n=d(r,2),d(r,10)
    if r[25]&2:yield from walk(r,nm+'/')
    else:yield nm,l,n,r[25]
  for nm,l,n,flags in walk(p[156:]):
   head=read(l,min(96,n)); item=dict(path=nm,bytes=n,lba=l,flags=flags,header=head[:40].hex(' '));result['files'].append(item)
   if nm in ('BIGJOB.MOV','BIGJOBCD.PIC','BIGJOBHD.PIC','BIGJOB.SND','BIGVOX.SND'):
    result['media'][nm]=media_index(f,l*2048,n,nm)
   if head[:2]==b'MZ' and n>=64:
    b=read(l,n);off=d(b,60);sig=b[off:off+2]; item['format']='NE' if sig==b'NE' else 'PE' if b[off:off+4]==b'PE\0\0' else 'LE' if sig==b'LE' else 'MZ';item['sha256']=hashlib.sha256(b).hexdigest()
    if item['format']=='NE':result['NE'][nm]=ne(b,off)
    if item['format']=='PE':item['machine']=hex(u(b,off+4));item['magic']=hex(u(b,off+24))
   if '/' not in nm and nm in ['BIGJOB.EXE','BIGJOBNG.EXE','BIGJOB.MMS','BIGJOB.DAT','LEIAME.WRI','BIGJOB.SND','BIGVOX.SND']:
    b=read(l,min(n,4000000)); hints=[]
    if nm.endswith('.WRI') and len(b)>=128:
     stop=d(b,14); result['leiame_text_bytes']=stop
    for m in re.finditer(rb'[\x20-\x7e\x80-\xff]{5,}',b):
     s=m[0]
     if len(s)<180 and re.search(rb'quicktime|qtw|imagebuilder|mmfw|bigjob|portugu|instala|bem-vindo|mciSend|\.mov|\.pic|\.snd|\.mms|cd-rom|winG|[a-z]:\\',s,re.I):hints.append([m.start(),s.decode('cp1252','replace')])
     if len(hints)==100:break
    result['evidence'][nm]=hints
 return result
if __name__=='__main__':
 r=inspect(Path(sys.argv[1])); Path(sys.argv[2]).write_text(json.dumps(r,ensure_ascii=False,indent=1),encoding='utf8')



