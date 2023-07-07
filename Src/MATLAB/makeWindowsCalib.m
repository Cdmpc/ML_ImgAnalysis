function CalibCenters = makeWindowsCalib(CalibFile, SpotSpacing, Sigma)
% MAKEWINDOWS --> Creates pitch * pitch windows around a spot in the pupil
% plane image. It will center boxes around pixels in a calibration image,
% so that each center is the brightest. These spots are always evenly
% spaced apart. 

% ARGUEMENTS:
% CalibFile = The calibration image path.
% SpotSpacing = How many pixels each spot is spaced apart, both in X and Y.
% Sigma = How spread out from the center the square ROI is.

% RETURNS:
% CalibCenters = The reference coordinates for the spots, with respect to
% the whole image.
% ======================================================================= %
    CalibMatrix = im2gray(imread(CalibFile));
    CalibSize = size(CalibMatrix);
    imshow(CalibMatrix, InitialMagnification="fit");
    impixelinfo();

    CalibCenters = cat(3);
    % The pitch of the boxes, should be SpotSpacing / 2, can be a double.
    Pitch = SpotSpacing / 2;

    % As a starting point, find the entire max pixel in the image and
    % its location.
    [~, TotalLI] = max(CalibMatrix, [], "all");
    [Max_Row, Max_Col] = ind2sub(CalibSize, TotalLI);
    
    %-- Given the row and col of total max, subtract them by step to get the first row and column instance
    % of a possible spot: ROI will be 1/8 of the image dimensions.
    minRow = Max_Row; minCol = Max_Col;
    while minRow > floor(CalibSize(1) / Sigma)
        if(minRow - SpotSpacing <= floor(CalibSize(1) / Sigma))
            break;
        else
            minRow = minRow - SpotSpacing;
        end
    end
    while minCol > floor(CalibSize(2) / Sigma) 
        if(minCol - SpotSpacing <= floor(CalibSize(2) / Sigma))
            break;
        else
            minCol = minCol - SpotSpacing;
        end
    end
    disp("The first spot will be in (Y, X) = (" + minRow + ", " + minCol + ")"); 
    TopLeftY = minRow - floor(Pitch / 2);
    TopLeftX = minCol - floor(Pitch / 2);
    writematrix([], "CalibDisp.csv", WriteMode="overwrite");
    
    currentRow = minRow;
    currentCol = minCol;

    maxRow = CalibSize(1) - floor(minRow / 2);
    maxCol = CalibSize(2) - floor(minCol / 2);

    while 1
        %-- CASE 1: Reached edge in the X direction, the column
        if(currentCol + SpotSpacing > maxCol && currentRow + SpotSpacing <= maxRow)
            subimg = imcrop(CalibMatrix, [TopLeftX, TopLeftY, Pitch - 2, Pitch - 2]);
            [~, li] = max(subimg, [], "all");
            [mr, mc] = ind2sub(size(subimg), li);
            OffSetX = 0; OffSetY = 0;
            if(mc > floor(Pitch - floor(Pitch / 2)) && currentCol > 1)
                OffSetX = 1;
            elseif(mc < floor(Pitch - floor(Pitch / 2)) && currentCol > 1)
                OffSetX = -1;
            end
            if(mr > floor(Pitch - floor(Pitch / 2)) && currentRow > 1)
                OffSetY = 1;
            elseif(mr < floor(Pitch - floor(Pitch / 2)) && currentRow > 1)
                OffSetY = -1;
            end
            rectangle(Position=[TopLeftX - 0.5 + OffSetX, TopLeftY - 0.5 + OffSetY, Pitch - 0.5, Pitch - 0.5], EdgeColor="red", LineWidth=1);
            CalibCell = {currentCol + OffSetX, currentRow + OffSetY, 0, 0};
            T = cell2table(CalibCell);
            writetable(T, "CalibDisp.csv", Delimiter=",", WriteMode="append");
            CalibCenters = cat(3, CalibCenters, [currentRow + OffSetX, currentCol + OffSetY]);
            currentCol = minCol;
            currentRow = currentRow + SpotSpacing;
            TopLeftX = currentCol - floor(Pitch / 2);
            TopLeftY = currentRow - floor(Pitch / 2);
        %-- CASE 2: Reached bottom corner, cannot go in X and Y further.
        elseif(currentRow + SpotSpacing > maxRow && currentCol + SpotSpacing > maxCol)
            subimg = imcrop(CalibMatrix, [TopLeftX, TopLeftY, Pitch - 2, Pitch - 2]);
            [~, li] = max(subimg, [], "all");
            [mr, mc] = ind2sub(size(subimg), li);
            OffSetX = 0; OffSetY = 0;
            if(mc > floor(Pitch - floor(Pitch / 2)) && currentCol > 1)
                OffSetX = 1;
            elseif(mc < floor(Pitch - floor(Pitch / 2)) && currentCol > 1)
                OffSetX = -1;
            end
            if(mr > floor(Pitch - floor(Pitch / 2)) && currentRow > 1)
                OffSetY = 1;
            elseif(mr < floor(Pitch - floor(Pitch / 2)) && currentRow > 1)
                OffSetY = -1;
            end
            rectangle(Position=[TopLeftX - 0.5 + OffSetX, TopLeftY - 0.5 + OffSetY, Pitch - 0.5, Pitch - 0.5], EdgeColor="red", LineWidth=1);
            CalibCell = {currentCol + OffSetX, currentRow + OffSetY, 0, 0};
            T = cell2table(CalibCell);
            writetable(T, "CalibDisp.csv", Delimiter=",", WriteMode="append");
            CalibCenters = cat(3, CalibCenters, [currentRow + OffSetX, currentCol + OffSetY]);
            break;
        %-- CASE 3: Normal case, space in both X and Y available
        else
            subimg = imcrop(CalibMatrix, [TopLeftX, TopLeftY, SpotSpacing - 1, SpotSpacing - 1]);
            [~, li] = max(subimg, [], "all");
            [mr, mc] = ind2sub(size(subimg), li);
            OffSetX = 0; OffSetY = 0;
            if(mc > floor(Pitch - floor(Pitch / 2)) && currentCol > 1)
                OffSetX = 1;
            elseif(mc < floor(Pitch - floor(Pitch / 2)) && currentCol > 1)
                OffSetX = -1;
            end
            if(mr > floor(Pitch - floor(Pitch / 2)) && currentRow > 1)
                OffSetY = 1;
            elseif(mr < floor(Pitch - floor(Pitch / 2)) && currentRow > 1)
                OffSetY = -1;
            end
            rectangle(Position=[TopLeftX - 0.5 + OffSetX, TopLeftY - 0.5 + OffSetY, Pitch - 0.5, Pitch - 0.5], EdgeColor="red", LineWidth=1);
            CalibCell = {currentCol + OffSetX, currentRow + OffSetY, 0, 0};
            T = cell2table(CalibCell, VariableNames=["Column #", "Row #", "Vertical Disp", "Horizantal Disp"]);
            writetable(T, "CalibDisp.csv", Delimiter=",", WriteMode="append");
            CalibCenters = cat(3, CalibCenters, [currentRow + OffSetX, currentCol + OffSetY]);
            currentCol = currentCol + SpotSpacing;
            TopLeftX = currentCol - floor(Pitch / 2);
        end 
    end
    disp("CALIBRATION FILE WRITTEN TO .CSV FILE");
    fclose("all");
end
