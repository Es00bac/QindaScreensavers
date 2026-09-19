#!/usr/bin/env python3
"""Capture the live renderer's full-body animation reel; requires ffmpeg and GL."""
import argparse
import os
import pathlib
import subprocess
import tempfile

root = pathlib.Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--binary', type=pathlib.Path, default=root / 'build/prism-brawl')
parser.add_argument('--output', type=pathlib.Path, default=root / 'previews/Prism_Brawl_Animation.mp4')
args = parser.parse_args()
binary, output = args.binary.resolve(), args.output.resolve()
output.parent.mkdir(parents=True, exist_ok=True)
# Closeups are explicitly authored showcase clips. The first and last excerpts
# are unmodified autonomous combat, including actual hits and reactions.
clips = [
    (8, 4, ['--camera', 'close']),
    (2.4, 2.4, ['--showcase', '0', '--animation-demo', '--no-hud']),
    (2.4, 2.4, ['--showcase', '1', '--animation-demo', '--no-hud']),
    (21.6, 2.4, ['--showcase', '4', '--animation-demo', '--no-hud']),
    (28.8, 2.4, ['--showcase', '2', '--animation-demo', '--no-hud']),
    (33.6, 2.4, ['--showcase', '7', '--animation-demo', '--no-hud']),
    (25, 4, ['--fighters', '8', '--camera', 'close']),
]
with tempfile.TemporaryDirectory(prefix='prism-brawl-animation-') as temporary:
    directory = pathlib.Path(temporary)
    rendered = []
    for index, (start, seconds, options) in enumerate(clips):
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
                str(binary), '--seed', '41', '--start', str(start), '--size', '1280x720',
                '--fps', '60', '--frames', str(round(seconds * 60)), '--raw', str(fifo), *options,
            ], check=True)
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
        rendered.append(movie)
        print(f'Animation excerpt {index + 1}/{len(clips)} complete', flush=True)
    listing = directory / 'concat.txt'
    listing.write_text(''.join(f"file '{path.as_posix()}'\n" for path in rendered))
    subprocess.run([
        'ffmpeg', '-hide_banner', '-loglevel', 'error', '-y', '-f', 'concat', '-safe', '0',
        '-i', str(listing), '-c', 'copy', '-movflags', '+faststart', str(output),
    ], check=True)
print(output)
