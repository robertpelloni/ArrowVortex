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


class MockChart:
    def __init__(self, *args, **kwargs):
        pass


ddc_stepmania.learn.chart.Chart = MockChart
ddc_stepmania.learn.chart.OnsetChart = MockChart
ddc_stepmania.learn.chart.SymbolicChart = MockChart

try:
    with open("lib/ddc/train.json/_Gravity.pkl", "rb") as f:
        data = pickle.load(f)
    print(f"Type: {type(data)}")
    if isinstance(data, dict):
        print(f"Keys: {list(data.keys())}")
        if "charts" in data:
            print(f"Number of charts: {len(data['charts'])}")
            if len(data["charts"]) > 0:
                print(f"First chart keys: {list(data['charts'][0].keys())}")
    elif isinstance(data, list):
        print(f"List length: {len(data)}")
        if len(data) > 0:
            print(f"First item type: {type(data[0])}")
            print(f"First item: {data[0]}")
except Exception as e:
    print(f"Error: {e}")
