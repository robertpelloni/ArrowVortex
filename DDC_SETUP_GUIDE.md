# DDC (Dance Dance Convolution) Setup Guide

This guide will help you set up and train the DDC (Dance Dance Convolution) system for automatic StepMania chart generation in ArrowVortex.

## Overview

DDC uses deep learning to automatically generate StepMania charts from audio files. The system consists of:
- **Onset Detection**: Detects beats/onsets in audio (pre-trained PyTorch model)
- **Chart Generation**: Generates note patterns (TensorFlow models - need training)
- **Difficulty Rating**: Rates chart difficulty 1-10 (FFR models - need training)

## Prerequisites

### 1. Python Installation

**Requirement**: Python 3.6 or higher

**Check if Python is installed:**
```bash
python --version
# or
python3 --version
```

**Install Python:**
- Download from [python.org](https://www.python.org/downloads/)
- During installation, check "Add Python to PATH"
- Restart your computer after installation

### 2. Install Python Dependencies

Open a command prompt in the ArrowVortex directory and run:

```bash
# Navigate to ArrowVortex directory
cd C:\path\to\ArrowVortex

# Install required packages
pip install -r lib/ddc/requirements.txt
```

**Key dependencies** (automatically installed):
- TensorFlow 2.x (~500-1000MB)
- PyTorch (~500-1000MB)
- librosa (audio analysis)
- simfile (SM file parsing)
- scikit-learn, pandas, numpy, scipy

**Total download size**: ~2-3GB

### 3. GPU Support (Optional but Recommended)

Training models on CPU can take 12-24 hours. With GPU, it takes 4-8 hours.

**For NVIDIA GPUs:**
1. Install [NVIDIA CUDA Toolkit](https://developer.nvidia.com/cuda-downloads) (11.x or 12.x)
2. Install [cuDNN](https://developer.nvidia.com/cudnn)
3. Install GPU-enabled TensorFlow:
   ```bash
   pip install tensorflow-gpu
   ```

**Check GPU availability:**
```python
python -c "import tensorflow as tf; print('GPU Available:', tf.config.list_physical_devices('GPU'))"
```

## Step 1: Download Training Data

DDC needs official DDR (Dance Dance Revolution) charts to learn from.

**Option A: Automatic Download (Recommended)**

Run the PowerShell script:
```powershell
cd lib/ddc
.\download_data.ps1
```

This downloads ~12 DDR official packs from Zenius-I-Vanisher:
- DDR SuperNOVA, SuperNOVA2
- DDR X, X2, X3 VS 2ndMIX
- DDR 2013, 2014
- DDR A, A20, A20 PLUS, A3, WORLD

**Size**: Several GB
**Time**: 30-60 minutes depending on internet speed
**Location**: `lib/ddc/data/raw/ddr_official/`

**Option B: Manual Download**

1. Visit https://zenius-i-vanisher.com/
2. Download official DDR song packs
3. Extract to `lib/ddc/data/raw/ddr_official/`
4. Ensure each song has:
   - `.sm` file (StepMania chart)
   - Audio file (`.mp3`, `.ogg`, or `.wav`)

**Verify download:**
```bash
# Check that folders exist
ls lib/ddc/data/raw/ddr_official/
# Should show multiple song folders
```

## Step 2: Train DDC Models

### Quick Training (Recommended)

Run the complete training pipeline:

```bash
cd lib/ddc
python scripts/train_all.py data/raw/ddr_official work_dir
```

**Parameters:**
- `data/raw/ddr_official`: Input directory with downloaded DDR packs
- `work_dir`: Working directory for intermediate files and output models

**Time estimates:**
- **GPU**: 4-8 hours
- **CPU**: 12-24 hours

**What it does:**
1. Extracts chart data from .sm files → JSON
2. Filters into 10 difficulty buckets (dance-single/double × 5 difficulties)
3. Extracts audio features (Mel-spectrograms)
4. Trains 1 onset detection model (binary classification)
5. Trains 10 SymNet models (note pattern generation)
6. Trains FFR difficulty rating models

**Output:**
```
work_dir/
├── models/
│   ├── onset/
│   │   └── model.h5
│   ├── dance-single_Beginner/
│   │   ├── model.h5
│   │   └── vocab.json
│   ├── dance-single_Easy/
│   │   ├── model.h5
│   │   └── vocab.json
│   ├── dance-single_Medium/
│   │   ├── model.h5
│   │   └── vocab.json
│   ├── dance-single_Expert/
│   │   ├── model.h5
│   │   └── vocab.json
│   ├── dance-single_Challenge/
│   │   ├── model.h5
│   │   └── vocab.json
│   ├── dance-double_Beginner/
│   │   ├── model.h5
│   │   └── vocab.json
│   ├── dance-double_Easy/
│   │   ├── model.h5
│   │   └── vocab.json
│   ├── dance-double_Medium/
│   │   ├── model.h5
│   │   └── vocab.json
│   ├── dance-double_Expert/
│   │   ├── model.h5
│   │   └── vocab.json
│   └── dance-double_Challenge/
│       ├── model.h5
│       └── vocab.json
└── ffr_models/
    └── (FFR difficulty models)
```

### Manual Training (Advanced)

If you want more control over the training process:

#### Step 2.1: Extract JSON from .sm files
```bash
cd lib/ddc
python dataset/extract_json.py data/raw/ddr_official work_dir/json_raw
```

#### Step 2.2: Filter into difficulty buckets
```bash
python dataset/filter_json.py work_dir/json_raw work_dir/json_filtered
```

#### Step 2.3: Extract audio features
```bash
python learn/extract_feats_v2.py work_dir/json_filtered work_dir/feats
```

#### Step 2.4: Train models
```bash
# Train onset model
python scripts/train_v2.py work_dir/json_filtered/dance-single_Expert work_dir/feats work_dir/models/onset --model_type onset --epochs 5

# Train each SymNet model (repeat for each difficulty)
python scripts/train_v2.py work_dir/json_filtered/dance-single_Beginner work_dir/feats work_dir/models/dance-single_Beginner --model_type sym --epochs 10
python scripts/train_v2.py work_dir/json_filtered/dance-single_Easy work_dir/feats work_dir/models/dance-single_Easy --model_type sym --epochs 10
# ... (repeat for all 10 buckets)
```

#### Step 2.5: Train FFR difficulty models
```bash
cd lib/ddc/ffr-difficulty-model
python scripts/make_dataset_from_sm.py ../../data/raw/ddr_official ../../work_dir/ffr_dataset
python scripts/train_model.py ../../work_dir/ffr_dataset ../../work_dir/ffr_models
```

## Step 3: Install Models

After training completes, copy models to the expected location:

```bash
# Copy DDC models
cp -r work_dir/models/* lib/ddc/models/

# Copy FFR models
cp -r work_dir/ffr_models/* lib/ddc/ffr_models/
```

**Verify installation:**
```bash
ls lib/ddc/models/onset/model.h5
ls lib/ddc/models/dance-single_Expert/model.h5
ls lib/ddc/ffr_models/
```

## Step 4: Configure ArrowVortex

### 4.1: Set Python Path

1. Open ArrowVortex
2. Go to **Edit → Preferences**
3. Set **Python Path** to one of:
   - `python` (if Python is in your PATH)
   - `python3` (on Linux/Mac)
   - `C:\Python39\python.exe` (full path to python.exe)
4. Click **OK**

### 4.2: Verify Configuration

The Batch DDC dialog should now show default paths:
- **Output Directory**: `C:\path\to\ArrowVortex\output`
- **DDC Models Directory**: `C:\path\to\ArrowVortex\lib\ddc\models`
- **FFR Models Directory**: `C:\path\to\ArrowVortex\lib\ddc\ffr_models`

These paths are automatically set to use the executable directory.

## Step 5: Generate Charts

### Using the Batch DDC Dialog

1. Open ArrowVortex
2. Go to **File → Batch DDC Generation...**
3. Click **Add Files...** to add individual audio files (.mp3, .ogg, .wav)
   - OR click **Add Folder...** to process all audio files in a folder
4. (Optional) Change output directory
5. (Optional) Change model directories if models are elsewhere
6. Click **GENERATE CHARTS**

### Validation Checks

The dialog will automatically verify:
- ✓ Python is configured and exists
- ✓ DDC script exists (`lib/ddc/autochart.py`)
- ✓ Model directory exists
- ✓ At least onset model is present
- ✓ Output directory exists or can be created
- ✓ Input files exist and have valid extensions

If any check fails, you'll see a detailed error message.

### What Happens During Generation

1. **Audio Analysis**: Loads audio, detects BPM and onsets
2. **Chart Generation**: Creates charts for all difficulty levels using trained models
3. **Difficulty Rating**: Analyzes generated charts and assigns difficulty (1-10)
4. **File Creation**: Creates .sm file with all charts, copies audio, downloads artwork

### Output Structure

```
output/
└── Album Name/
    └── Artist - Title/
        ├── Artist - Title.sm     (StepMania file with all charts)
        ├── audio.mp3             (Copied from input)
        ├── banner.png            (Downloaded or extracted)
        └── bg.png                (Downloaded or extracted)
```

### Using the Command Line (Alternative)

You can also run DDC directly from command line:

```bash
cd lib/ddc
python autochart.py --models_dir models --ffr_dir ffr_models --out_dir output "path/to/song.mp3"
```

**Batch processing:**
```bash
python autochart.py --models_dir models --ffr_dir ffr_models --out_dir output "path/to/music/folder"
```

## Troubleshooting

### "Python path not configured"
**Solution**: Set Python path in Edit → Preferences

### "DDC script not found"
**Solution**: Ensure `lib/ddc/autochart.py` exists in the ArrowVortex directory

### "Model directory not found"
**Solution**: Train models first using Step 2, then copy to `lib/ddc/models/`

### "Onset model not found"
**Solution**: Training incomplete or failed. Check training logs.

### "ModuleNotFoundError: No module named 'tensorflow'"
**Solution**: Install Python dependencies:
```bash
pip install -r lib/ddc/requirements.txt
```

### "Could not create a cudart64_XX.dll library"
**Solution**: Either:
- Install CUDA toolkit (for GPU support)
- OR use CPU-only TensorFlow (slower but works):
  ```bash
  pip install tensorflow-cpu --force-reinstall
  ```

### Charts are low quality / don't match music
**Possible causes:**
- Models undertrained (train for more epochs)
- Insufficient training data (add more song packs)
- Bad BPM detection (manually set BPM in .sm file after generation)

**Solution**: Retrain with more data or adjust training parameters in `scripts/train_v2.py`

### Generation is very slow
**Solution**:
- Use GPU acceleration (see Section 3)
- Process fewer files at once
- Use faster difficulty settings (beginner charts generate faster)

### "ERROR: Command failed to execute"
**Check**:
1. Python is installed and in PATH
2. All required packages are installed
3. Model files exist in specified directories
4. Input files are valid audio formats
5. Output directory is writable

**View detailed error log**: Check `ddc_log.txt` in the ArrowVortex directory

## Advanced Configuration

### Customizing Training Parameters

Edit `scripts/train_v2.py`:

```python
# Adjust epochs (more = better quality but slower)
ONSET_EPOCHS = 5      # Default: 5
SYMNET_EPOCHS = 10    # Default: 10

# Adjust batch size (larger = faster but needs more RAM)
BATCH_SIZE = 32       # Default: 32

# Adjust learning rate
LEARNING_RATE = 0.001 # Default: 0.001
```

### Using Your Own Training Data

1. Organize your .sm files and audio in a folder structure:
   ```
   my_training_data/
   ├── Song 1/
   │   ├── song.sm
   │   └── audio.mp3
   ├── Song 2/
   │   ├── song.sm
   │   └── audio.ogg
   └── ...
   ```

2. Run training pipeline:
   ```bash
   python scripts/train_all.py my_training_data work_dir
   ```

### Model Ensemble (Advanced)

Train multiple models with different parameters and combine their predictions for better results.

## Performance Benchmarks

| Hardware | Training Time | Generation Speed |
|----------|--------------|------------------|
| RTX 3080 | 4-6 hours | ~30 seconds/song |
| GTX 1060 | 8-12 hours | ~60 seconds/song |
| CPU only | 20-30 hours | ~5 minutes/song |

*Training time for complete pipeline with 12 DDR packs*

## System Requirements

### Minimum
- **OS**: Windows 10/11
- **RAM**: 8GB
- **Storage**: 20GB free space
- **CPU**: Quad-core 2.5GHz
- **Python**: 3.6+

### Recommended
- **OS**: Windows 10/11
- **RAM**: 16GB+
- **Storage**: 50GB+ free space (SSD preferred)
- **GPU**: NVIDIA with 4GB+ VRAM
- **CPU**: 8-core 3.0GHz+
- **Python**: 3.8-3.10

## Additional Resources

- **DDC GitHub**: https://github.com/chrisdonahue/ddc
- **StepMania**: https://www.stepmania.com/
- **Zenius-I-Vanisher**: https://zenius-i-vanisher.com/ (DDR song database)
- **TensorFlow Documentation**: https://www.tensorflow.org/
- **ArrowVortex Issues**: https://github.com/uvcat7/ArrowVortex/issues

## Credits

- **DDC System**: Chris Donahue et al.
- **FFR Difficulty Model**: Flash Flash Revolution community
- **ArrowVortex Integration**: ArrowVortex contributors

## License

DDC is released under the MIT License. See `lib/ddc/LICENSE` for details.
