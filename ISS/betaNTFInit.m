function [W0, H0, Q0] = betaNTFInit(X,K,MelFilt)
%-------------------------------------------------------------------------%
% Complex SVD as initialization for NTF. 
%
%   Original publication
%     J. Becker, M. Menzel, and C. Rohlfing, "Complex SVD initialization 
%     for NMF source separation on audio spectrograms," in Fortschritte der 
%     Akustik DAGA '15, (Nürnberg, Germany), Mar. 2015.
%     http://www.ient.rwth-aachen.de/services/bib2web/pdf/BeMeRo15.pdf
%
%   Julian Becker, Christian Rohlfing 
%   (C) 2017 Institut für Nachrichtentechnik, RWTH Aachen University
  
  % Call complex SVD
  [U,S,V] = svds(sum(X,3), K);
  
  % Calculate initial NTF parameters
  W0 = abs(U)*sqrt(S);
  H0 = abs(V)*sqrt(S);
  Q0 = ones(size(X,3),K);
  
  % Apply Mel-filtering on W0 if needed
  if nargin > 2, W0 = MelFilt'*W0; end
end