#!/usr/bin/env python3
"""Record all seven native arenas. Requires Pillow, ffmpeg and a GL display."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
STAGES = [
    ('prism', '01 / PRISM TERMINAL', 'Obsidian orbit'),
    ('garden', '02 / REACTOR GARDEN', 'Living circuitry'),
    ('rooftop', '03 / AFTERGLOW ROOFTOP', 'Neon rain'),
    ('bliss', '04 / BLISS CIRCUIT', 'Reclaimed green hills'),
    ('compile', '05 / COMPILE CLUB', 'The midnight workshop'),
    ('aurora', '06 / AURORA GLACIER', 'Ice, crystal and northern light'),
    ('azure', '07 / AZURE FOLD', 'Flowing blue mineral'),
]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--binary', type=Path, default=ROOT / 'build/prism-brawl')
parser.add_argument('--stills-only', action='store_true')
args = parser.parse_args()
binary = args.binary.resolve()
validation = ROOT / 'validation/stages'
previews = ROOT / 'previews'
validation.mkdir(parents=True, exist_ok=True)
previews.mkdir(parents=True, exist_ok=True)

def font(size):
    try:
        return ImageFont.truetype('DejaVuSans.ttf', size)
    except OSError:
        return ImageFont.load_default()

with (validation / 'captures.log').open('w') as log:
    for key, _, _ in STAGES:
        subprocess.run([
            str(binary), '--stage', key, '--seed', '41', '--start', '12',
            '--size', '1280x720', '--mute', '--snapshot', str(validation / f'{key}.png'),
        ], stdout=log, stderr=subprocess.STDOUT, check=True)

sheet = Image.new('RGB', (1600, 1836), '#090f19')
draw = ImageDraw.Draw(sheet)
draw.text((32, 22), 'PRISM BRAWL / SEVEN WORLDS', font=font(36), fill='#e5f5f4')
draw.text((34, 74), 'Pengu + Ducke wallpaper-inspired arenas / actual OpenGL captures',
          font=font(18), fill='#70ada8')
for index, (key, title, subtitle) in enumerate(STAGES):
    x, y = 32 + index % 2 * 784, 120 + index // 2 * 422
    frame = Image.open(validation / f'{key}.png').convert('RGB')
    frame.thumbnail((752, 360), Image.Resampling.LANCZOS)
    # Preserve the original image ratio; do not crop gameplay or its interface.
    sheet.paste(frame, (x + (752 - frame.width) // 2, y))
    draw.text((x + 12, y + 364), title, font=font(20), fill='#e5f5f4')
    draw.text((x + 12, y + 392), subtitle, font=font(16), fill='#8daaa9')
draw.text((830, 1520), 'FOUR NEW ARENAS', font=font(28), fill='#80e2cb')
draw.text((830, 1566), 'Three rebuilt favorites.', font=font(23), fill='#e5f5f4')
draw.text((830, 1605), 'Full-body fights, taunts and reactions.', font=font(18), fill='#8daaa9')
draw.text((830, 1635), 'Automatic stage rotation after each match.', font=font(18), fill='#8daaa9')
sheet.save(previews / 'Prism_Brawl_Stages.png')
sheet.save(validation / 'arena-review.jpg', quality=90)
print('Seven arena stills and gallery complete', flush=True)

if not args.stills_only:
    with tempfile.TemporaryDirectory(prefix='prism-brawl-stages-') as temporary, \
            (validation / 'video.log').open('w') as log:
        directory = Path(temporary)
        clips = []
        for index, (key, _, _) in enumerate(STAGES):
            fifo, movie = directory / f'{index}.rgb', directory / f'{index}.mp4'
            os.mkfifo(fifo)
            encoder = subprocess.Popen([
                'ffmpeg', '-hide_banner', '-loglevel', 'error', '-y',
                '-f', 'rawvideo', '-pixel_format', 'rgb24', '-video_size', '1280x720',
                '-framerate', '60', '-i', str(fifo), '-an', '-c:v', 'libx264',
                '-preset', 'fast', '-crf', '19', '-pix_fmt', 'yuv420p', str(movie),
            ])
            try:
                subprocess.run([
                    str(binary), '--stage', key, '--seed', '41', '--start', '10',
                    '--size', '1280x720', '--mute', '--fps', '60', '--frames', '180',
                    '--raw', str(fifo),
                ], stdout=log, stderr=subprocess.STDOUT, check=True)
                if encoder.wait(timeout=60):
                    raise RuntimeError('ffmpeg encoding failed')
            except BaseException:
                encoder.terminate()
                try:
                    encoder.wait(timeout=10)
                except subprocess.TimeoutExpired:
                    encoder.kill()
                    encoder.wait()
                raise
            clips.append(movie)
            print(f'Arena film {index + 1}/{len(STAGES)} complete', flush=True)
        listing = directory / 'concat.txt'
        listing.write_text(''.join(f"file '{clip.as_posix()}'\n" for clip in clips))
        subprocess.run([
            'ffmpeg', '-hide_banner', '-loglevel', 'error', '-y', '-f', 'concat',
            '-safe', '0', '-i', str(listing), '-c', 'copy', '-movflags', '+faststart',
            str(previews / 'Prism_Brawl_Stages.mp4'),
        ], check=True)
    print(previews / 'Prism_Brawl_Stages.mp4')
