function [ExpTable] = ExpTableCalculator(SubSize, Sigma, Pix_Accuracy)
% EXPTABLECALCULATOR - Calculates exponentials for all possible gaussian
% spot offsets for the refined centroid location algorithm. For this case,
% x and mu = 0 in terms of the gaussian exponent.
BetterAccuracy = power(2, -1 + log(Pix_Accuracy) / log(2));
Limit = 1 + ((SubSize + 3) / (2 * Pix_Accuracy));
gaussianExp = -1 / (2 * Sigma * Sigma);
ExpTable = zeros(Limit, Limit);