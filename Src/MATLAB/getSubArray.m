function [submatrix, NewMaxRow, NewMaxCol] = getSubArray(Pupil_Plane, WinSpace, SubSize)
%-- GETSUBARRAY: This routine returns a subarray, as a matrix. Using the
%-- coordinates in from the top-left corner of the window as a starting
%-- point denoted by Winspace.It scans the window for the maximum pixel 
%-- intensity, and then it will center (or center closely if N is even) 
%-- the subarray matrix.
    %SubSize MUST BE ODD:
    if(mod(SubSize, 2) == 0)
        disp("SubSize must be odd!");
        submatrix = -1;
        return
    end
    
    %-- ADD ; AFTER DONE TESTING
    %disp("================ [GETTING SUBARRAY SPOT...] ================");
    Window = imcrop(Pupil_Plane, [WinSpace(1), WinSpace(2), WinSpace(3)-1, WinSpace(4)-1]);
    WindowSize = size(Window);
    [MaxVal, I] = max(Window, [], 'all');
    [Max_Row, Max_Col] = ind2sub(size(Window), I);
    %disp("The max pixel in the current window is == " + MaxVal + " at (Row, Col) == (" + Max_Row + ", " + Max_Col + ")");
    TopLeftRow = Max_Row - floor(SubSize / 2);
    TopLeftCol = Max_Col - floor(SubSize / 2);
    BottomRightRow = SubSize + TopLeftRow - 1;
    BottomRightCol = SubSize + TopLeftCol - 1;
    
    %-- Check to ensure the subarray does not fall off the bounds of WinSpace.
    %-- If so, the array may not be centered around the max pixel.
    if(TopLeftRow < 1)
        LeftDisp = length(TopLeftRow:1) - 1;
        TopLeftRow = 1;
        %disp("Pushing forward TLX");
        BottomRightRow = BottomRightRow + LeftDisp;
    end
    if(TopLeftCol < 1)
        DownDisp = length(TopLeftCol:1) - 1;
        TopLeftCol = 1;
        %disp("Pushing down TLY");
        BottomRightCol = BottomRightCol + DownDisp;
    end
    if(BottomRightRow > WindowSize(1))
        RightDisp = length(WindowSize(1):BottomRightRow) - 1;
        BottomRightRow = WindowSize(1);
        %disp("Pushing back BRX");
        TopLeftRow = TopLeftRow - RightDisp;
    end
    if(BottomRightCol > WindowSize(2))
        UpDisp = length(WindowSize(2):BottomRightCol) - 1;
        BottomRightCol = WindowSize(2);
        %disp("Pushing up BRY");
        TopLeftCol = TopLeftCol - UpDisp;
    end
    
    %disp("The top left coordinate of subarray: (Row, Col) == (" + TopLeftRow + ", " + TopLeftCol + ")");
    %disp("The bottom right coordinate of subarray: (Row, Col) == (" + BottomRightRow + ", " + BottomRightCol + ")");
    
    Row_Range = TopLeftRow:BottomRightRow; Col_Range = TopLeftCol:BottomRightCol;
    %-- ADD ; AFTER DONE TESTING
    submatrix = Window(Row_Range, Col_Range);
    [Max, LI] = max(submatrix, [], "all");
    [NewMaxRow, NewMaxCol] = ind2sub(size(submatrix), LI);
end