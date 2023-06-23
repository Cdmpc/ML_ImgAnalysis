# ML_ImgAnalysis

Image Analysis spot location detector written in MATLAB to find and track the brightest pixel in a Pupil plane image and find it's displacement.
For now, this focuses on the displacement in the (x, y) direction of the brightest pixel in the entire image, based on the reference coordinates of the brightest pixel
in the calibration image. Using that reference, the data image's brightest pixel coordinates are tracked and their displacement in the X, Y direction will be calculated in microns, which can be converted by specifying the PixRatio argument in the findBrightestDisp function, which calls on a helper routine called catImgs3 to combine the data images into one massive 3D matrix, so that it can be compared against the calibration matrix.
