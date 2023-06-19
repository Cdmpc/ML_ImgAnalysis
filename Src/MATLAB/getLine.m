function maxPixLine = getLine(Pupil_Plane, WinSpace, SubSize)
%-- getLine = Get row of pixels where the max pixel is located.
%-- Given the Pupil_Plane matrix, a Window Space denoted by coordinates,
%-- and the Square Subarray dimension.
    Window = imcrop(Pupil_Plane, [WinSpace(1), WinSpace(2), WinSpace(3)-1, WinSpace(4)-1]);
    
    WinSize = size(Window);
    [MaxVal, I] = max(Window, [], 'all');
    [Max_Row, Max_Col] = ind2sub(size(Window), I);
    disp("The max pixel in the current window is == " + MaxVal + " at (Row, Col) == (" + Max_Row + ", " + Max_Col + ")");
    StartCol = Max_Col - floor(SubSize / 2);
    EndCol = SubSize + StartCol - 1;
    
    if(StartCol < 1)
        LeftDisp = length(StartCol:1) - 1;
        StartCol = 1;
        disp("Pushing forward TLX");
        EndCol = EndCol + LeftDisp;
    end
    if(EndCol > WinSize(1))
        RightDisp = length(WinSize(1):EndCol) - 1;
        EndCol = WinSize(1);
        disp("Pushing back BRX");
        StartCol = StartCol - RightDisp;
    end

    disp("Start of linear subarray: (Row, Col) == (" + Max_Row + ", " + StartCol + ")");
    disp("End of linear subarray: (Row, Col) == (" + Max_Row + ", " + EndCol + ")");
    Col_Range = StartCol:EndCol;
    maxPixLine = Window(Max_Row, Col_Range);
end