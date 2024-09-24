# BPM: Blended Piecewise Möbius Maps
C++/OpenGL implementation of [BPM: Blended Piecewise M&ouml;bius Maps](https://mirelabc.github.io/publications/BPM.pdf) by Rorberg, Vaxman & Ben-Chen.

|![BPM interpolated wolf head](images/wolf_head_BPM.png "BPM interpolation")|![Piecewise-linear wolf head](images/wolf_head_PL.png "Piecewise-linear interpolation")|
|:----:|:-----:|
|BPM interpolation|Piecewise-linear interpolation|

Technical report is under `doc` folder.
## Video

https://github.com/user-attachments/assets/32da3296-b41f-47cb-b0fb-f1b43f9ea4f4


## Build Instructions

1. Run the following commands:

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j7
```

2. After building, make sure model.obj is placed in resources/objects dir, and run the code with:
```bash
./BPM model.obj
```

Running:
```bash
./BPM 
```
runs default model.
