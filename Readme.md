# CivilRegistryAnalyzer

This tools aims to assist the segmentation of civil registry documents with image processing and machine learning
algorithms.

## Installation

### Dependencies

- OpenCv (Tested with >= 4.0.1)
- Qt (Tested with >= 5.11.2)
- Python 3

You might also need CUDA if you want to use Tensorflow's GPU version.

### Clone and  Build

```
git clone --recurse-submodules -j8 https://github.com/PlathC/CivilRegistryAnalyser.git
cd CivilRegistryAnalyser
pip install -r requirements.txt

mkdir build && cd build
cmake ..
cmake --build .
```

### Based on

- [arthurflor23 / handwritten-text-recognition](https://github.com/arthurflor23/handwritten-text-recognition)
- [arthurflor23 / text-segmentation](https://github.com/arthurflor23/text-segmentation)