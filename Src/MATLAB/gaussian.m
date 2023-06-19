function gaussian(sigma)
% GAUSSIAN - Plots a finite gaussuan curve from min_x to max_x.
% -- mu = Mean value, the peak or valley of the gaussian will be at x = mu
% -- sigma = Standard deviation, the bigger it is, the more spread the
% curve has.
% -- min_x/max_x = the ranges of x values to plot the gaussian.
% -- step = how much to increment each value in between min_x and max_x.
% The smaller the value the more smooth the curve is.
    if sigma == 0
        disp("Sigma cannot be 0, please enter a non-zero value");
    else
        x = -3:(0.0001):3;
        y = (exp((-1/2)*power((x)/(sigma) , 2))) / (sigma*sqrt(2*pi));
        plot(x, y, "b-", "Color", "Red", "LineWidth", 2);
        xline(0,"-",'x = 0','LineWidth',1.5);
        yline(0,"-",'y = 0','LineWidth',1.5); 
        
    end
end
