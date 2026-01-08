import sys
import os
import pickle
from types import ModuleType

# Add lib/ddc to path so we can import 'learn'
sys.path.append(os.path.abspath("lib/ddc"))

try:
    import learn.chart
    import learn.beatcalc
except ImportError as e:
    print(f"Import failed: {e}")
    sys.exit(1)

# Mocking infrastructure
m_ddc = ModuleType("ddc_stepmania")
sys.modules["ddc_stepmania"] = m_ddc

m_learn = ModuleType("ddc_stepmania.learn")
sys.modules["ddc_stepmania.learn"] = m_learn
m_ddc.learn = m_learn

# Alias the actual modules
# The pickle likely refers to classes as ddc_stepmania.learn.chart.Chart
sys.modules["ddc_stepmania.learn.chart"] = learn.chart
m_learn.chart = learn.chart

sys.modules["ddc_stepmania.learn.beatcalc"] = learn.beatcalc
m_learn.beatcalc = learn.beatcalc

# Now try loading
try:
    with open("lib/ddc/train.json/_Gravity.pkl", "rb") as f:
        meta, feats, charts = pickle.load(f)
    print("Success loading pickle!")
    print(f"Song title: {meta.get('title')}")
    print(f"Feats shape: {feats.shape if feats is not None else 'None'}")
    print(f"Num charts: {len(charts)}")
    if "audio_fp" in meta:
        print(f"Audio Path: {meta['audio_fp']}")
    elif "music_fp" in meta:
        print(f"Music Path: {meta['music_fp']}")
    else:
        print(f"Keys in meta: {list(meta.keys())}")
except Exception as e:
    print(f"Pickle load failed: {e}")
    import traceback

    traceback.print_exc()
