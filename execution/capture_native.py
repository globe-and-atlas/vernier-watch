"""Serial native SDK captures with foreground, placement, and persistence checks."""
from pathlib import Path
import argparse,json,re,shutil,socket,subprocess,time
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'.tmp/native'

def command(args):
 result=subprocess.run(args,cwd=ROOT/'watchface',capture_output=True,text=True,timeout=100)
 with (OUT/'emulator.log').open('a') as log:log.write(result.stdout+result.stderr)
 if result.returncode:raise RuntimeError(f'{args[:3]} failed; see .tmp/native/emulator.log')

def monitors():
 found={}
 result=subprocess.run(['ps','-axo','pid,comm,args'],capture_output=True,text=True,check=True)
 for line in result.stdout.splitlines():
  if re.match(r'^\s*\d+\s+qemu-pebble\s',line) and '/Pebble SDK/' in line:
   match=re.search(r'-monitor tcp::(\d+)',line)
   if match:found[int(line.split()[0])]=int(match.group(1))
 return found

def close(port):
 with socket.create_connection(('localhost',port),timeout=5) as sock:sock.recv(65536);sock.sendall(b'quit\n')

def capture(peak=255):
 values=monitors()
 if len(values)!=1:raise RuntimeError('Expected one owned SDK emulator')
 path=OUT/'native.ppm';path.unlink(missing_ok=True)
 with socket.create_connection(('localhost',next(iter(values.values()))),timeout=5) as sock:
  sock.recv(65536);sock.sendall(f'screendump "{path}"\n'.encode());deadline=time.monotonic()+5
  while time.monotonic()<deadline:
   sock.recv(65536)
   try:im=Image.open(path).convert('RGB');break
   except (OSError,FileNotFoundError):continue
  else:raise TimeoutError('Native screendump incomplete')
 maximum=max(max(pixel) for pixel in im.get_flattened_data());im=im.point(lambda channel:round(channel/max(maximum,1)*peak/85)*85);path.unlink(missing_ok=True);return im

def cases():
 base={'Palette':0,'Background':0,'TextColor':0xFFFFFF,'HourColor':0x55AAFF,'MinuteColor':0xFF5500,'TimeFormat':2,'DateFormat':0,'Labels':1,'DigitalTime':1,11000:10,11001:10,11002:20}
 variants=[('default',{})]+[(f'palette-{i}',{'Palette':i}) for i in range(6)]
 variants += [('custom',{'Palette':6,'Background':0x000055,'TextColor':0xFFFFAA,'HourColor':0xAAFFAA,'MinuteColor':0xFF55AA}),('clean',{'Labels':0}),('pure-analog',{'Labels':0,'DigitalTime':0}),('orbits-only',{'DigitalTime':0}),('midnight-12',{'TimeFormat':1,11000:0,11001:0,11002:0}),('midnight-24',{11000:0,11001:0,11002:0}),('noon-12',{'TimeFormat':1,11000:12,11001:0,11002:0}),('late-12',{'TimeFormat':1,11000:23,11001:59,11002:59}),('late-24',{11000:23,11001:59,11002:59}),('rollover-before',{11000:11,11001:59,11002:0}),('rollover-after',{11000:12,11001:0,11002:0}),('half-hour',{11000:13,11001:30,11002:0}),('quarter-hour',{11000:13,11001:15,11002:0})]
 variants += [('align-zero',{11002:0}),('align-30',{11002:30}),('align-59',{11002:59})]
 variants += [(f'date-{i}',{'DateFormat':i}) for i in [1,2,3]]
 return [(name,{**base,**change}) for name,change in variants]

def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--platform',choices=['emery','gabbro'],required=True);parser.add_argument('--reset-emulator',action='store_true');parser.add_argument('--default-only',action='store_true');parser.add_argument('--typography-only',action='store_true');args=parser.parse_args();OUT.mkdir(parents=True,exist_ok=True)
 platform=args.platform;size=(200,228) if platform=='emery' else (260,260);w,h=size
 existing=monitors()
 if existing and not args.reset_emulator:raise RuntimeError('Another emulator is active; inspect ownership before --reset-emulator')
 # The explicit reset option is only used after checking no other task owns this emulator.
 if args.reset_emulator:
  sdk=Path.home()/'Library/Application Support/Pebble SDK'
  for flash in sdk.glob('*/*/*flash*.bin'):
   backup=OUT/'emulator-backups'/flash.relative_to(sdk);backup.parent.mkdir(parents=True,exist_ok=True)
   if not backup.exists():shutil.copy2(flash,backup)
  for port in existing.values():close(port)
  command(['pebble','kill']);time.sleep(.3);command(['pebble','wipe'])
 fixture=OUT/'fixture.pbw';assert fixture.exists()
 install=['pebble','install','--emulator',platform,str(fixture)]
 command(install);time.sleep(1);command(install)
 keys=json.loads((OUT/'message_keys.json').read_text());uuid=json.loads((ROOT/'watchface/package.json').read_text())['pebble']['uuid']
 def send(values):command(['pebble','send-app-message','--emulator',platform,'--app-uuid',uuid,'--int']+[f'{keys.get(key,key)}={value}' for key,value in values.items()])
 def take(values):
  send({**values,11022:1})
  for attempt in range(30):
   im=capture()
   if im.size==size and im.getpixel((w//2-3,20))==(255,0,255):
    assert im.getpixel((w//2+3,20))==(0,255,0),'Native placement failed';break
   time.sleep(.1)
  else:raise AssertionError('Expected Vernier fixture is not in foreground')
  send({11022:0})
  for attempt in range(20):
   im=capture()
   if im.getpixel((w//2-3,20))!=(255,0,255):break
   time.sleep(.1)
  else:raise AssertionError('Proof marker did not clear')
  # Clearing the marker can be observed at the start of an LCD transfer.
  # Require three equal complete captures before accepting a native frame.
  previous=None;stable=0
  for attempt in range(30):
   time.sleep(.12);im=capture(255);pixels=im.tobytes()
   stable=stable+1 if pixels==previous else 0;previous=pixels
   if stable>=2:
    heading=im.crop((w//2-31,5,w//2+31,18))
    bg=im.getpixel((0,0))
    assert sum(pixel!=bg for pixel in heading.get_flattened_data())>=10,'Incomplete heading in stable native frame'
    return im
  raise AssertionError('Native frame never settled')
 evidence=[];selected=[row for row in cases() if row[0] in {'default','palette-1','midnight-24','noon-12','late-12','late-24'}] if args.typography_only else cases()[:1] if args.default_only else cases()
 for name,values in selected:
  im=take(values);path=ROOT/f'media/{platform}-{name}.png';im.save(path)
  evidence.append({'case':name,'path':str(path),'size':list(im.size),'foreground':True,'placement':True});print(platform,name,flush=True)
 if not args.default_only and not args.typography_only:
  # Deliver the actual normalized output of a real Clay Save to the native inbox.
  saved=json.loads((ROOT/'.tmp/clay-save.json').read_text());values={**dict(cases()[0][1]),**saved};im=take(values);im.save(ROOT/f'media/{platform}-clay-save.png')
  command(install);im2=take({11000:10,11001:10,11002:20});assert im.tobytes()==im2.tobytes(),'Settings did not survive reinstall';im2.save(ROOT/f'media/{platform}-persisted.png')
  # Invalid native values cannot overwrite the saved state.
  bad={'Palette':7,'TimeFormat':9,'Labels':2,'TextColor':0x1000000};im3=take(bad);assert im2.tobytes()==im3.tobytes(),'Invalid native setting changed appearance'
  evidence.append({'case':'clay-save-reinstall-invalid-values','passed':True})
  take(dict(cases()[0][1]))
 (OUT/(f'{platform}-typography-verification.json' if args.typography_only else f'{platform}-verification.json')).write_text(json.dumps(evidence,indent=2)+'\n')
 print(platform,'native suite PASS',flush=True)
if __name__=='__main__':main()
