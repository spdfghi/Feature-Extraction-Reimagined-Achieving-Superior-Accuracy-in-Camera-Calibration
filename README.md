## What's this?
This is a project focused on feature refinement in camera calibration. The specific paper with details of method will be updated later. We do not provide a camera calibration pipeline; instead, we used project https://github.com/puzzlepaint/camera_calibration for testing. It is recommend for it's BA optimization and generic camera model.

## How to use?
In current version, you should display the video in Pattern/calibration_target.mp4 in a screen and then extract stable frame from the captured video, then use project https://github.com/puzzlepaint/camera_calibration for feature extraction. The pattern file used for this project i s in Pattern/info/.
Data path should be like:
```
├── star8_1
│   ├── features_10px.bin
│   └── images
│       ├── 0.png
│       └── 1.png
└── star8_2
    ├── features_10px.bin
    └── images
        ├── 0.png
        └── 1.png
```
See main.cpp for example.

If you want to calibrate use feature points extracted by other method like OpenCV, you should use
```
comb_refine(img_path+"_",filename,fets);
```
for multi-channel image, where img_path in above is *star8*.
Or 
```
sym_refine(img,fets);
```
for single-channel image.

Change ```#define MULT 8``` to ```#define MULT 0``` in ```sym_refine.h``` and get refinement for single image. 

## Other issue
We welcome any improvements to this project, especially in creating calibration videos and extracting stable frames.

As working for a company now, I will update this project in my free time. If you have questions, connect me with wsfddn@foxmail.com. 