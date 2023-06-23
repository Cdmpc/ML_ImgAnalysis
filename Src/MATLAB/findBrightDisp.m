function findBrightDisp(dirpath, CalibImg, PixRatio)
%-- ARGUMENTS: 
%   dirpath = The directory containing all the data frame images.
%   CalibImg = The path to the calibration image FILE not matrix.
%   PixRatio = The ratio of one pixel to it's real life size in microns. 
%   For example: If PixRatio = 5.5, that means one pixel = 5.5 microns.

    %Ensure PixRatio is positive:
    if(PixRatio <= 0)
        disp("Please make sure PixRatio is a positive number");
        return;
    end
    
    %-- Read the calibration image and get it's size.
    CalibMatrix = im2gray(imread(CalibImg));
    CalibSize = size(CalibMatrix);
    
    %-- Find the maximum pixel and it's position for the calibration frame.
    [CalMax, CalLoc] = max(CalibMatrix, [], "all");
    [CalMaxRow, CalMaxCol] = ind2sub(CalibSize, CalLoc);
    
    %-- Make cell with these values, and put the values into a table with
    %   the VariableName = [Column titles].
    ReferenceCell = {"Calibration Image File", CalMax, CalMaxRow, CalMaxCol, 0, 0};
    T = cell2table(ReferenceCell, VariableNames=["Name", "Max Value", "Row #", "Column #", "Vertical Disp(μ)", "Horizantal Disp(μ)"]);
    writetable(T, "SpotDisp.csv", Delimiter=",", WriteMode="overwrite");

    %-- Create a 3D matrix, concatenating the data frames as matrices.
    [ImgList, numFiles, File] = catImgs3(dirpath);
    
    %-- Loop comparing each data matrix with the calibration matrix.
    for i = 1:numFiles
        %-- Find the max pixel in each data image and it's location.
        [DataMax, DataLoc] = max(ImgList(:, :, i), [], "all");
        [DataMaxRow, DataMaxCol] = ind2sub(size(ImgList), DataLoc);
        %-- The calibration pixel will be the reference coordinate.
        %-- Pos. displacement = Right in X, Up in Y.
        %-- Neg. displacement = Left in X, Down in Y.
        RowDisp = CalMaxRow - DataMaxRow;
        ColDisp = CalMaxCol - DataMaxCol;
        DataCell = {File(i + 2).name, DataMax, DataMaxRow, DataMaxCol, (RowDisp * PixRatio), (ColDisp * PixRatio)};
        T = cell2table(DataCell);
        writetable(T, "SpotDisp.csv", Delimiter=",", WriteMode="append");
    end
    disp("========================== [DISPLACEMENTS WRITTEN TO .CSV FILE] ==========================");
end
  