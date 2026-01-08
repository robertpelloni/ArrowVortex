import pickle
import sys
import os
import types

# Create a full mock package structure
ddc_stepmania = types.ModuleType("ddc_stepmania")
sys.modules["ddc_stepmania"] = ddc_stepmania
ddc_stepmania.learn = types.ModuleType("ddc_stepmania.learn")
sys.modules["ddc_stepmania.learn"] = ddc_stepmania.learn
ddc_stepmania.learn.chart = types.ModuleType("ddc_stepmania.learn.chart")
sys.modules["ddc_stepmania.learn.chart"] = ddc_stepmania.learn.chart
ddc_stepmania.learn.beatcalc = types.ModuleType("ddc_stepmania.learn.beatcalc")
sys.modules["ddc_stepmania.learn.beatcalc"] = ddc_stepmania.learn.beatcalc


class MockChart:
    def __init__(self, *args, **kwargs):
        pass


class MockBeatCalc:
    def __init__(self, *args, **kwargs):
        pass


ddc_stepmania.learn.chart.Chart = MockChart
ddc_stepmania.learn.chart.OnsetChart = MockChart
ddc_stepmania.learn.chart.SymbolicChart = MockChart
ddc_stepmania.learn.beatcalc.BeatCalc = MockBeatCalc

try:
    with open("lib/ddc/train.json/_Gravity.pkl", "rb") as f:
        data = pickle.load(f)
    print(f"Type: {type(data)}")
    print(f"Length: {len(data)}")
    for i, item in enumerate(data):
        print(f"Item {i} type: {type(item)}")
        if hasattr(item, "shape"):
            print(f"Item {i} shape: {item.shape}")
        else:
            print(f"Item {i} str: {str(item)[:100]}")
except Exception as e:
    print(f"Error: {e}")
