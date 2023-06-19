function [CentRow, CentCol] = makeCentroid(submatrix, subSize)
% MAKECENTROID -- Using an n * n submatrix with the brightest pixel being
% the center or close to the center of the submatrix. It will calculate the
% location of the reference spot.
    disp("================ [GETTING CENTROID SPOT...] ================");
    %-- Given the submatrix, find the location of the max pixel.
    [MaxVal, I] = max(submatrix, [], 'all');
    [Max_Row, Max_Col] = ind2sub(subSize, I);
    disp("The max pixel in the submatrix is == " + MaxVal + " at (Row, Col)" + ...
        " == (" + Max_Row + ", " + Max_Col + ")");
    
    %-- The centroids should be at the exact center of the matrix, whether the
    %-- submatrix itself is odd or even dimensions.
    CentRow = subSize / 2;
    CentCol = CentRow;
    
    CentRow = ceil(CentRow);
    CentCol = ceil(CentCol);
   
    disp("The spot loc in this window will be in (Row, Col) == (" + ...
        CentRow + ", " + CentCol + ")");
    
    %-- Showing the spot on the submatrix image.
    imshow(submatrix, InitialMagnification="fit");
    impixelinfo();
    %-- Visual circle plot on the submatrix image.
    % r = 0.15;
    % c = [CentRow, CentCol];
    % pos = [c-r 2*r 2*r];
    % rectangle(Position = pos, Curvature = [1 1], FaceColor ='green', EdgeColor="none");
end