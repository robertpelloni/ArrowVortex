import pickle
import sys
import os

# Mock the ddc_stepmania module
import types

ddc_stepmania = types.ModuleType("ddc_stepmania")
sys.modules["ddc_stepmania"] = ddc_stepmania
ddc_stepmania.smdataset = types.ModuleType("ddc_stepmania.smdataset")
ddc_stepmania.smdataset.parse = types.ModuleType("ddc_stepmania.smdataset.parse")


class MockChartParser:
    def __init__(self, *args, **kwargs):
        pass


ddc_stepmania.smdataset.parse.ChartParser = MockChartParser

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
except Exception as e:
    print(f"Error: {e}")
