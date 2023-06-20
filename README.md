# ML_ImgAnalysis

Image Analysis spot location detector written in MATLAB to find and track the brightest pixel in a Pupil plane image and find it's displacement.
For now, this focuses on the displacement in the (x, y) direction of the brightest pixel in the entire image, each image has a data frame and a calibration frame.
The program will compare these two images, hunt for the brightest pixel, store it's location and calculate it's horizantal and verical displacement.
