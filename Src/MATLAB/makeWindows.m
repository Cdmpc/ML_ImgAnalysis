function Windows = makeWindows(Pupil_Plane, StartX, StartY, Pitch, PixRatio)
% CREATES A 3D MATRIX, EACH ELEMENT BEING A PITCH BY PITCH MATRIX DENOTING
% A WINDOW, ONE AFTER THE OTHER. WHEN COMBINED THE 3D MATRIX SHOULD STILL
% HAVE THE SAME PROPERTIES.
%-- Pupil_Plane = The image as a matrix.
%-- TopLeftX, TopLeftY = The X and Y of the top left corner of the window.
%-- Pitch = The square window dimension
%-- PixRatio = The ratio of the spatial image size in microns to pixels.
% For example, PixRatio = 5.5, means one pixel = 5.5 microns.
    imgSize = size(Pupil_Plane);
    %-- Initialize empty 3D matrix, loop stopping flag and the right side
    %   of the boxes
    Windows = cat(3);
    scanComplete = 0;
    EndX = StartX + Pitch; 
    EndY = StartY + Pitch;
    disp("Each window will have a spatial measurement of: " + Pitch * PixRatio + " microns long and high");
    %-- Loop to concatenate other matrices as a 3D matrix.
    while 1
        %-- CASE 1: Right Side of square is on edge but still has space
        %   below.
        if(EndX > imgSize(2) && EndY <= imgSize(1))
            rectangle(Position = [StartX, StartY, Pitch - 1, Pitch - 1], EdgeColor = "#1ef7cc", LineWidth = 1.3);
            Window = imcrop(Pupil_Plane, [StartX, StartY, Pitch - 1, Pitch - 1]);
            Windows = cat(3, Windows, Window);
            StartX = 1; 
            EndX = StartX + Pitch;
            StartY = StartY + Pitch;  
            EndY = StartY + Pitch;
        %-- CASE 2: Right Side of Square is on edge and cannot go down
        %   further.
        elseif(EndX > imgSize(2) && EndY > imgSize(1))
            rectangle(Position = [StartX, StartY, Pitch - 1, Pitch - 1], EdgeColor = "#1ef7cc", LineWidth = 1.3);
            Window = imcrop(Pupil_Plane, [StartX, StartY, Pitch - 1, Pitch - 1]);
            Windows = cat(3, Windows, Window);
            disp("Window creation complete, Exiting loop.....");
            scanComplete = 1;
        %-- CASE 3: Window is in space on both X and Y
        else
            rectangle(Position = [StartX, StartY, Pitch - 1, Pitch - 1], EdgeColor = "#1ef7cc", LineWidth = 1.3);
            Window = imcrop(Pupil_Plane, [StartX, StartY, Pitch - 1, Pitch - 1]);
            Windows = cat(3, Windows, Window);
            StartX = StartX + Pitch;  
            EndX = EndX + Pitch;
        end
    
        %-- BREAK OUT OF WHILE LOOP ONCE FLAG TO STOP HAS BEEN SET.
        if(scanComplete == 1)
            break;
        end
    end
    WindowList = size(Windows);
    disp("Scan Complete, " + WindowList(3) + " windows were created.");
    %-- Once it has the windows it should call getSubArray on each window
    %   to get one smaller subarray on each window.

    for i = 1:WindowList(3)
        
    end

end