# ML_ImgAnalysis

Image Analysis spot location detector written in MATLAB to find and track the brightest pixels in a Pupil plane image and find it's displacement.
Spots in a image, are pixels with a certain brightness above a threshold. 
This will be approximated using the max pixel and any pixel more than 10% of its brightness as a region of interest.
Within this circular region of interest, each spot is always consistently spaced "N" pixels apart. We make boxes that are (N/2, N/2) in dimension.
The brightest pixel in each box in the calibration frame will be the center, so N must be odd. Record each windows coordinates, so that it can be used for the data frame. In the data frames, there is a chance that the brightest pixel will no longer be in the center.


## VERY IMPORTANT:
### In order for catImgs3 to work, ALL data images in the directory MUST HAVE THE EXACT SAME DIMENSIONS. Otherwise, the concatenation will not work.
