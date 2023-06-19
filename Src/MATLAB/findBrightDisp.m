function findBrightDisp(Pupil_Plane, PixRatio)
%-- ARGUMENTS: 
%   Pupil_Plane = The image as a matrix with each element
%   being it's pixel intensity

%   PixRatio = The ratio of one pixel to it's real life size in microns. 
%   For example: If PixRatio = 5.5, that means one pixel = 5.5 microns.
    imgSize = size(Pupil_Plane);
    disp("The Pupil Plane image is: " + imgSize(1) * PixRatio + " microns long and high.");
    
    %-- Crop the image down if it is not a square image. 
    if(imgSize(1) ~= imgSize(2))
        disp("Image is not square, cropping...");
        smallerDim = min(imgSize(1), imgSize(2));
        Pupil_Plane = imcrop(Pupil_Plane, [1, 1, smallerDim - 1, smallerDim - 1]);
        disp("Image dimensions in pixels: ");
        imgSize = size(Pupil_Plane);
        disp(imgSize);
    end

    
end
  