function calcRefSpots(Pupil_Plane, Pitch, PixRatio)
%-- ARGUMENTS: 
%   Pupil_Plane = The image as a matrix with each element
%   being it's pixel intensity

%   Pitch = The size of the window, THIS MUST BE evenly divisble so
%   that the concatenation of matrices can work. For example: If the
%   image is 1024 x 1024, the best window sizes would be 512, 256, 64
%   etc. The worst case is if the image dimensions are prime numbers,
%   so only a pitch of 1 would work.

%   PixRatio = The ratio of one pixel to it's realife size in microns. 
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
    if(mod(imgSize(1), Pitch) ~= 0)
        disp("Windows won't be evenly distributed, make sure that the quotient " + ...
            "of the image over the Pitch is nicely divisible...");
        return;
    end
    imshow(Pupil_Plane);
    impixelinfo();
    %-- Extract a 3D matrix of the Windows, when combined it should still
    %   form the original matrix.
    Windows = makeWindows(Pupil_Plane, 1, 1, Pitch, PixRatio);
end
  