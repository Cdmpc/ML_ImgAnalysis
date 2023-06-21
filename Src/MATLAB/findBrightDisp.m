function findBrightDisp(dirpath, CalibFrame, PixRatio)
%-- ARGUMENTS: 
%   dirpath = The directory containing all the data frame images.
%   CalibFrame = Matrix representation of the Calibration image. This is 1
%   image being compared to a directory of many Data frame images, all the
%   same size.

%   PixRatio = The ratio of one pixel to it's real life size in microns. 
%   For example: If PixRatio = 5.5, that means one pixel = 5.5 microns.
    calibSize = size(CalibFrame);
    
    %-- Find the maximum pixel and it's position for the calibration frame.
    [CalMax, CalLoc] = max(CalibFrame, [], "all");
    [CalMaxRow, CalMaxCol] = ind2sub(calibSize, CalLoc);
    %-- Make cell with these values, and put the values into a table with
    %   the VariableName = [headers].
    ReferenceCell = {"Calibration Image File", CalMax, CalMaxRow, CalMaxCol, 0, 0};
    T = cell2table(ReferenceCell, VariableNames=["Name", "Max Value", "Row #", "Column #", "Row Displacement", "Column Displacement"]);
    writetable(T, 'DisplacementsOfMax.csv', Delimiter=",", WriteMode="overwrite");

    %-- Create a 3D matrix, concatenating the data frames as matrices.
    [ImgList, numFiles, File] = catImgs3(dirpath);
    
    %-- Loop comparing each data matrix with the calibration matrix.
    for i = 1:numFiles
        %-- Find the max pixel in each data image and it's location.
        [DataMax, DataLoc] = max(ImgList(:, :, i), [], "all");
        [DataMaxRow, DataMaxCol] = ind2sub(size(ImgList), DataLoc);
        %-- The calibration pixel will be the reference displacement,
        %   a negative value means the pixel has moved left in the X
        %   direction, and/or down in the Y direction. Vice Versa if the
        %   displacement values are positive.
        RowDisp = CalMaxRow - DataMaxRow;
        ColDisp = CalMaxCol - DataMaxCol;

        DataCell = {File(i + 2).name, DataMax, DataMaxRow, DataMaxCol, RowDisp, ColDisp};
        T = cell2table(DataCell, VariableNames=["Name", "Max Value", "Row #", "Column #", "Row Displacement", "Column Displacement"]);
        writetable(T, 'DisplacementsOfMax.csv', Delimiter=",", WriteMode="append");
    end
    disp("========================== [DISPLACEMENTS WRITTEN TO .CSV FILE] ==========================");
end
  