import argparse
import os
import pickle
import numpy as np
import librosa
import json
import sys

SR = 44100
HOP_LENGTH = 512
N_MELS = 80


def extract_features(audio_fp):
    print(f"Loading audio: {audio_fp}")
    try:
        y, sr = librosa.load(audio_fp, sr=SR)
    except Exception as e:
        raise ValueError(f"Failed to load audio {audio_fp}: {e}")

    mel_spec = librosa.feature.melspectrogram(
        y=y, sr=sr, hop_length=HOP_LENGTH, n_mels=N_MELS
    ).T
    mel_spec_db = librosa.power_to_db(mel_spec, ref=np.max)

    delta = librosa.feature.delta(mel_spec_db)
    delta2 = librosa.feature.delta(mel_spec_db, order=2)

    feats = np.stack([mel_spec_db, delta, delta2], axis=-1)

    return feats, y


def create_dummy_chart(audio_fp, feats):
    n_frames = feats.shape[0]
    duration = n_frames * HOP_LENGTH / SR

    onset_frames = range(10, n_frames - 10, 43)

    notes = []
    for frame in onset_frames:
        time = frame * HOP_LENGTH / SR
        # Format: [pulse, beat, time, type]
        # OnsetChart unpacks 4 items and uses time (index 2).
        # Pulse and beat are not used by OnsetChart for onset detection itself, but Chart class might rely on structure.
        # We provide dummy values.
        notes.append([[0, 4, 0], 0.0, time, "1000"])

    chart_meta = {
        "difficulty_coarse": "Expert",
        "difficulty_fine": 10,
        "type": "dance-single",
        "desc_or_author": "DDC Sanity",
        "notes": notes,
    }

    meta = {
        "title": "Sanity Check",
        "artist": "DDC",
        "audio_fp": audio_fp,
        "music_fp": audio_fp,
        "duration": duration,
        "offset": 0.0,
        "bpms": [[0.0, 120.0]],
        "stops": [],
        "charts": [chart_meta],
    }

    return meta


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--audio_fp", type=str, required=True, help="Path to audio file"
    )
    parser.add_argument(
        "--out_dir", type=str, required=True, help="Output directory for pkl and npy"
    )
    args = parser.parse_args()

    if not os.path.exists(args.out_dir):
        os.makedirs(args.out_dir)

    feats, y = extract_features(args.audio_fp)
    print(f"Extracted features shape: {feats.shape}")

    song_name = os.path.splitext(os.path.basename(args.audio_fp))[0]
    feat_fp = os.path.join(args.out_dir, song_name + ".npy")
    np.save(feat_fp, feats)
    print(f"Saved features to {feat_fp}")

    meta = create_dummy_chart(args.audio_fp, feats)

    json_fp = os.path.join(args.out_dir, song_name + ".json")
    with open(json_fp, "w") as f:
        json.dump(meta, f, indent=2)
    print(f"Saved metadata to {json_fp}")

    pkl_fp = os.path.join(args.out_dir, song_name + ".pkl")
    with open(pkl_fp, "wb") as f:
        pickle.dump((meta, feats, []), f)
    print(f"Saved pickle (backup) to {pkl_fp}")


if __name__ == "__main__":
    main()
