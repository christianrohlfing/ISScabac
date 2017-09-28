function [SDR,sest,permi] = quality(theta,s,x,X,MelFilt,stftParam)
%-------------------------------------------------------------------------%
% Calculate SDR given NTF parameters stored in theta, original sources s
% and mixture x
%
%   Christian Rohlfing 
%   (C) 2017 Institut für Nachrichtentechnik, RWTH Aachen University

  % Inverse Mel-filtering
  Wtmp = MelFilt*theta.W;
  
  % NTF model for mix
  model = zeros(size(X));
  for j=1:size(s,2)
    Vhatj = Wtmp * diag(theta.Q(j,:)) * theta.H';
    model = model + Vhatj;
  end
  
  % Calculation of estimated sources in time domain
  sest = zeros(size(s));
  stftParam.L = size(x,1);
  for j=1:size(s,2)
    % NTF model for source j
    Vhatj = Wtmp * diag(theta.Q(j,:)) * theta.H';
    
    % Wiener filter
    mask = Vhatj ./ (model+eps);
    Sestj = X .* mask;
        
    % ISTFT
    sest(:,j) = ISTFT(Sestj, stftParam);
  end
  
  % SDR calculation
  [permi,SDR] = SolveAlignment(permute(s,[1 3 2]),permute(sest,[1 3 2]));
end



function [permi,SDR_opt]=SolveAlignment(s,s_est,DEBUG)
%-------------------------------------------------------------------------%
% Solve alignment between original sources s and estimated sources s_est
%
%   Optimal alignment found by minimizing SDR
%
%   Martin Spiertz 
%   (C) 2012 Institut für Nachrichtentechnik, RWTH Aachen University

  if nargin<3, DEBUG=0; end
  
  %% Parameter
  M    = size(s,3);       % Number of sources
  Mest = size(s_est,3);   % Number of estimated sources
  P    = perms(1:Mest);   % All possible alignments
  

  %% Energy
  E1 = zeros(1,M);
  E2 = zeros(M,Mest);
  
  for m1=1:M
    E1(m1) = sum(sum(s(:,:,m1).^2));
    for m2=1:Mest
      E2(m1,m2) = sum(sum( (s(:,:,m1)-s_est(:,:,m2)).^2 ));
    end
    if DEBUG, fprintf('.'); end
  end
  
  
  %% SDR and alignment
  SDR_opt = -realmax*ones(1,M);
  SDR     = zeros(1,M);
  permi = P(1,:);
  for row=1:size(P,1) % Iterate over possible alignments
    for m=1:M      
      SDR(m) = 10*log10( E1(m)/E2(m,P(row,m)) );
    end

    if(sum(SDR)>sum(SDR_opt))
      SDR_opt = SDR;
      permi   = P(row,:);
    end
  end

end