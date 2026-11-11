#!/usr/bin/env python3
"""Render a stereo/mono 16/24/32-bit WAV DI through the real Sundrift engine.

Requires python with numpy and soundfile (libsndfile). Build bridge first:
  c++ -O3 -std=c++17 -fPIC -shared tools/render_di_bridge.cpp -o libsundrift_render.so
Run:
  python tools/render_user_di.py /path/to/clean-DI.wav AudioPreviews/RealDI --all
  python tools/render_user_di.py /path/to/clean-DI.wav AudioPreviews/RealDI --presets 0 3 6 9

The input file stays untouched. A 4-second silent tail exposes reverb/echo behaviour.
These previews deliberately use the BUILT-IN amp (no NAM capture) and do not alter presets.
Preview-only level matching: normalises full-clip RMS to -20 dBFS with a peak ceiling of -1 dBFS.
"""
import argparse
import ctypes
from pathlib import Path
import numpy as np
import soundfile as sf

NAMES = (
    "BALTHVS-Sunday-Forecast", "BALTHVS-Golden-Cumbia", "BALTHVS-Lunar-Coast",
    "Khruangbin-Silk-Lines", "Khruangbin-Desert-Dub", "Khruangbin-Texas-Velvet",
    "Glass-Beams-Mirage-Pulse", "Glass-Beams-Temple-Steps", "Glass-Beams-Copper-Serpent",
    "Yuuf-Afterglow", "Yuuf-Coastal-Float", "Yuuf-Late-Drift",
)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("input",type=Path)
    parser.add_argument("output_dir",type=Path)
    group=parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--all",action="store_true")
    group.add_argument("--presets",type=int,nargs="+")
    parser.add_argument("--tail",type=float,default=4)
    args=parser.parse_args()
    libpath=Path(__file__).resolve().parents[1]/"libsundrift_render.so"
    lib=ctypes.CDLL(str(libpath))
    lib.sundrift_renderer_create.argtypes=[ctypes.c_int,ctypes.c_double]
    lib.sundrift_renderer_create.restype=ctypes.c_void_p
    p=ctypes.POINTER(ctypes.c_float)
    lib.sundrift_renderer_process.argtypes=[ctypes.c_void_p,p,p,ctypes.c_int]
    lib.sundrift_renderer_process.restype=None
    lib.sundrift_renderer_destroy.argtypes=[ctypes.c_void_p]
    lib.sundrift_renderer_destroy.restype=None
    src,sr=sf.read(args.input,dtype="float32",always_2d=True)
    if src.shape[1] not in (1,2): raise SystemExit("Only 1-2 channels supported")
    if src.shape[1]==1: src=np.tile(src,(1,2))
    length=len(src)+round(sr*args.tail)
    indices=range(12) if args.all else args.presets
    args.output_dir.mkdir(parents=True,exist_ok=True)
    for i in indices:
        if not 0<=i<12: raise SystemExit(f"Bad preset index: {i}")
        ptr=lib.sundrift_renderer_create(i,float(sr))
        if not ptr: raise SystemExit("Could not prepare DSP")
        left=np.zeros(length,dtype="float32")
        right=np.zeros(length,dtype="float32")
        left[:len(src)]=src[:,0];right[:len(src)]=src[:,1]
        try: lib.sundrift_renderer_process(ptr,left.ctypes.data_as(p),right.ctypes.data_as(p),length)
        finally: lib.sundrift_renderer_destroy(ptr)
        processed=np.stack((left,right),axis=1)
        if not np.isfinite(processed).all(): raise RuntimeError(f"Non-finite audio in preset {i}")
        # Catch accidental runaway DSP before generating a misleading preview.
        peak=float(np.max(np.abs(processed)))
        if peak > 8.0: raise RuntimeError(f"Abnormal peak {peak:.1f} in preset {i}; check DSP stability")
        # Level-match for listening only. Double precision avoids squaring overflow.
        rms=float(np.sqrt(np.mean(np.square(processed.astype(np.float64)))))
        gain=min(10**(-20/20)/max(rms,1e-9),10**(-1/20)/max(peak,1e-9))
        processed*=gain
        path=args.output_dir/f"{NAMES[i]}-DI.wav"
        sf.write(path,processed,sr,subtype="PCM_24")
        print(f"{path.name}: {len(processed)/sr:.1f}s, peak {20*np.log10(max(np.max(np.abs(processed)),1e-10)):.1f} dBFS, RMS {20*np.log10(max(np.sqrt(np.mean(processed**2)),1e-10)):.1f} dBFS",flush=True)

if __name__=="__main__":main()
