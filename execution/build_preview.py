"""Assemble inspected native PNGs without altering their pixels."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
ROOT=Path(__file__).resolve().parents[1]
def main():
 canvas=Image.new('RGB',(1000,640),'#192124');draw=ImageDraw.Draw(canvas)
 font=ImageFont.truetype('/System/Library/Fonts/Supplemental/Arial.ttf',18)
 for column,platform in enumerate(['emery','gabbro']):
  for row,case in enumerate(['default','palette-1']):
   im=Image.open(ROOT/f'media/{platform}-{case}.png').convert('RGB')
   assert im.size==((200,228) if platform=='emery' else (260,260))
   x=column*500+(500-im.width)//2;y=row*310+40;canvas.paste(im,(x,y))
   draw.text((column*500+25,row*310+10),f'{"Time 2" if platform=="emery" else "Round 2"} / {"Night" if row==0 else "Paper"}',font=font,fill='white')
 canvas.save(ROOT/'media/native-comparison.png')
if __name__=='__main__':main()
