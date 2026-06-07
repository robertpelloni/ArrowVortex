# Deployment & Environment Setup

## Prerequisites
- **Compiler**: Visual Studio 2022 (Windows), CMake (Linux/macOS).
- **Python**: 3.12.x recommended.
- **Vcpkg**: Required for managing C++ dependencies on Linux/macOS.

## Python Environment (ML Backend)
1. Navigate to `lib/ddc/`.
2. Create a virtual environment: `python3 -m venv ddc_env`.
3. Activate the environment:
   - Windows: `ddc_env\Scripts\activate`
   - Linux/macOS: `source ddc_env/bin/activate`
4. Install dependencies:
   ```bash
   pip install -r requirements.txt
   pip install torch==2.2.0 torchvision torchaudio --index-url https://download.pytorch.org/whl/cpu
   pip install "numpy<2" "simfile>=2.1"
   ```

## Model Training
1. Download data: `bash lib/ddc/download_data.sh`.
2. Run automated training:
   ```bash
   cd lib/ddc
   ./ddc_env/bin/python scripts/train_all.py data/raw/ddr_official work_dir
   ```

## ArrowVortex Configuration
1. Open ArrowVortex.
2. Go to `Edit -> Preferences`.
3. Set the **Python Path** to the executable within `ddc_env`.
