# Internal Memory & Observations

## Architecture
- **C++/Qt Frontend**: Handles the main editor logic, graphics, and user interaction.
- **Python ML Backend**: Powers the Dance Dance Convolution (DDC) system for automatic chart generation.
- **Submodule Strategy**: Heavy use of submodules (ddc, odcnn, bobcoin) to allow independent development of core components.

## DDC Integration
- **PyTorch Transition**: The system has successfully transitioned from TensorFlow to PyTorch for model training and inference.
- **Environment Stability**: Python 3.12 with `torch 2.2.0` and `numpy 1.26.4` provides the most stable environment for the current ML stack.
- **Model Storage**: Trained models are stored in `lib/ddc/models/` and `lib/ddc/ffr_models/`, with support for both `.pth` (PyTorch) and `.h5` (Legacy) formats.

## Key Constraints
- **Absolute Paths**: The C++ code must resolve absolute paths relative to the executable directory to avoid issues with Python execution.
- **UI Blocking**: Long-running Python processes currently block the UI thread. Asynchronous execution with progress updates is a critical future improvement.
