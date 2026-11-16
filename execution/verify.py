"""Verify production identity, target binaries, and absence of fixture controls."""
from pathlib import Path
import hashlib
import json
import subprocess
import zipfile
ROOT=Path(__file__).resolve().parents[1]
def main():
 package=json.loads((ROOT/'watchface/package.json').read_text());target=ROOT/'dist/Vernier-0.1.0.pbw'
 assert package['pebble']['targetPlatforms']==['emery','gabbro']
 with zipfile.ZipFile(target) as bundle:
  info=json.loads(bundle.read('appinfo.json'));assert info['uuid']==package['pebble']['uuid'];assert info['versionLabel']=='0.1.0';assert info['targetPlatforms']==['emery','gabbro']
  expected={key:10000+i for i,key in enumerate(package['pebble']['messageKeys'])};assert info['appKeys']==expected
  for platform in ['emery','gabbro']:assert bundle.read(platform+'/pebble-app.bin')==(ROOT/'watchface/build'/platform/'pebble-app.bin').read_bytes()
 assert 'pebble-js-app.js' in bundle.namelist()
 nm=Path.home()/'Library/Application Support/Pebble SDK/SDKs/current/toolchain/arm-none-eabi/bin/arm-none-eabi-nm'
 for platform in ['emery','gabbro']:
  result=subprocess.run([str(nm),str(ROOT/'watchface/build'/platform/'pebble-app.elf')],capture_output=True,text=True,check=True)
  symbols={line.split()[-1] for line in result.stdout.splitlines() if line.split()}
  assert not symbols.intersection({'fh','fm','fs','proof'}),'Fixture control in production ELF'
 report={'version':'0.1.0','uuid':info['uuid'],'targets':info['targetPlatforms'],'sha256':hashlib.sha256(target.read_bytes()).hexdigest()};(ROOT/'.tmp/production-verification.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS:',json.dumps(report))
if __name__=='__main__':main()
