# DDC Models

This directory should contain the trained DDC models.

## How to Generate Models

1.  **Download Data**:
    Run `lib/ddc/download_data.ps1` in PowerShell. This will download ~15GB of DDR simfiles to `lib/ddc/data/raw`.

2.  **Train Models**:
    Run the training script from the `lib/ddc` directory:
    ```bash
    python scripts/train_all.py data/raw/ddr_official work_dir
    ```
    This will take several hours (or days) depending on your GPU.

3.  **Install Models**:
    Copy the generated models from `work_dir/models` to this directory (`bin/models`).
    Copy the FFR models from `work_dir/ffr_models` to `bin/models/ffr`.

## Expected Structure

*   `bin/models/onset/`
*   `bin/models/dance-single_Beginner/`
*   `bin/models/dance-single_Easy/`
*   ...
*   `bin/models/ffr/`
