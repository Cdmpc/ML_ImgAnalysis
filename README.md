# ML_ImgAnalysis

Image Analysis spot location detector written in MATLAB to find and track the brightest pixel in a Pupil plane image and find it's displacement.
For now, this focuses on the displacement in the (x, y) direction of the brightest pixel in the entire image, based on the reference coordinates of the brightest 

Using that reference, each data image's brightest pixel coordinates are tracked and their displacement in the X and Y directions will be calculated in microns, which can be converted by specifying the PixRatio argument in the findBrightestDisp function, which calls on a helper routine called catImgs3 to combine the data images into one 3D matrix, each element or page being a data image in matrix form, each page being compared against the reference displacement.

The arguements to findBrightestDisp are the path to the directory of data images, the path to the calibration image outside the directory as a standalone file, and the ratio of how many microns in real measurement space is in each pixel.


## VERY IMPORTANT:
### In order for catImgs3 to work, ALL data images in the directory MUST HAVE THE EXACT SAME DIMENSIONS. Otherwise, the concatenation will not work.
