import pickle
import sys

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
