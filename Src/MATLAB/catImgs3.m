function [ImgList, numFiles, filesInDir] = catImgs3(dirpath)
%-- READIMAGES3 -- Reads a set of images from a directory and concatenates
%   them into a 3D matrix where each element is the image matrix turned to
%   a grayscale image thanks to im2gray(imread("image-file.png")); 

%-- ARGUMENTS:
%   dirpath = The directory storing the data images.
%   CalibMatrix = Matrix representation of the calibration image.

%-- RETURNS:
%   ImgList = The 3D matrix of concatenated grayscale images.
%   numFiles = The number of files found in the dirpath.

%   VERY IMPORTANT: The images in the directory MUST ALL BE THE EXACT SAME
%   SIZE, OTHERWISE THE CONCATENATION WILL NOT WORK.

    %-- String array of the files in the directory.
    filesInDir = dir(dirpath);
    %-- Count files in the directory, excluding current and parent dirs.
    fileStruct = size(filesInDir);
    numFiles = fileStruct(1) - 2;

    %-- Initialize an empty 3D matrix to prepare for the concatenation.
    ImgList = cat(3);

    %-- The loop starts at 3 and ends at N + 2 to account for the exclusion
    %   of the current and parent directory items from the dir() function.
    for i = 3:(numFiles + 2)
        newMatrix = im2gray(imread(dirpath + "/" + filesInDir(i).name));
        ImgList = cat(3, ImgList, newMatrix);
    end
    disp("Concatenation complete........");
end