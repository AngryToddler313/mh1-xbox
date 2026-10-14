#!/usr/bin/env python3
"""gsheet.py TAG T1 T2 ... : contact sheet build/D/g/TAG_sheet.png of the RT_SHOTS pictures build/D/g/TAG_<tick>.png (agent D tooling)"""
import sys
from PIL import Image
tag, ts = sys.argv[1], sys.argv[2:]
cols = 2 if len(ts) <= 4 else 3
rows = (len(ts) + cols - 1) // cols
W, H = (640, 360) if cols == 2 else (640, 360)
im = Image.new('RGB', (W * cols, H * rows))
for i, t in enumerate(ts):
    a = Image.open('build/D/g/%s_%s.png' % (tag, t)).convert('RGB').resize((W, H))
    im.paste(a, ((i % cols) * W, (i // cols) * H))
im.save('build/D/g/%s_sheet.png' % tag)
