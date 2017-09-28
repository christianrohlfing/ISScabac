function [Xmel, MelFilt] = MelFiltering(X, NMel, Fs, MelFilt)
%-------------------------------------------------------------------------%
% Mel-filtering of spectrogram
%
%   Christian Rohlfing, Martin Spiertz
%   (C) 2012-2017 Institut für Nachrichtentechnik, RWTH Aachen University

  if NMel < size(X,1)
    if nargin < 4
      % Create filterbank
      MelFilt = CreateFilterBank(size(X,1)*2-2,NMel,Fs); MelFilt = sparse(MelFilt');
    end
    
    % Filtering
    Xmel = zeros(size(MelFilt,2),size(X,2),size(X,3));
    for m=1:size(Xmel,3)
      Xmel(:,:,m) = MelFilt'*X(:,:,m);
    end
  else
    if nargin < 4
      MelFilt = sparse(1:size(X,1),1:size(X,1),1); end
    Xmel = X;
  end

end


function [H,f] = CreateFilterBank( numFFT, numFilter, Fs, DEBUG)
%-------------------------------------------------------------------------%
% Create Mel-filterbank H
%
%   with numFilter triangular filters.
%
%   Christian Rohlfing, Martin Spiertz
%   (C) 2012-2017 Institut für Nachrichtentechnik, RWTH Aachen University

  %% Parameter
  if nargin < 4, DEBUG = 0; end
      
  % Demo mode
  if nargin < 1, numFFT = 2^12; numFilter = 40; Fs = 44100; end
  
  %% Variables
  % Nyquist index
  K = round(numFFT/2)+1;
  if(K < numFilter)
    error('Number of mel filters should be lower than FFT length');
  end
    
  % Minimum and maximum frequency values
  fMinHz = 0;
  fMaxHz = 0.5*Fs;
  
  % Number of center/corner points
  numCenter = numFilter;
  
  %% Create center/corner points and frequency axis for comparison
  % Frequency axis in Hz
  f = linspace( fMinHz, fMaxHz, K );

  % Center/corner points which define triangluar filters in Hz
  % (The center points have linear distance in the Mel domain)
  fMinMel = f2m(fMinHz);
  fMaxMel = f2m(fMaxHz);
  cMel = m2f( linspace(fMinMel,fMaxMel, numCenter) );
  
  % Reshape last center
  if (f(end)-cMel(end))^2 < eps
    cMel(end) = f(end); 
  end
  
  % Set upper-slope of the first and lower-slope of the last filter to
  % infinity (by setting the respective line interval to zero)
  cMel = [cMel(1), cMel, cMel(end)];
  
  % Slope interval widths
  dc = diff(cMel);
  
  %% Create filterbank
  if DEBUG, fprintf('Create Mel Filterbank (NMel=%d)...', numFilter);  end
  
  H = zeros( numFilter, K ); 
  for m = 1:numFilter 

    % Up slope
    k = (f>=cMel(m) & f<=cMel(m+1));
    if(abs(dc(m)) > eps)
      H(m,k) = (f(k)-cMel(m))/dc(m);
    else
      % Divison 0 by 0 is set to 1
      H(m,k) = 1;
    end

    % Down slope
    k = (f>=cMel(m+1) & f<cMel(m+2));
    if(abs(dc(m+1)) > eps)
      H(m,k) = (cMel(m+2)-f(k))/dc(m+1);
    else
      % Divison 0 by 0 is set to 1
      H(m,k) = 1;
    end
  end
  
  
  %% Avoid frequency spreading 
  % in low frequencies (for small FFT sizes)
  for n=1:min(size(H))
    H(n,n) = H(n,n) + sum(H((n+1):end,n));
    H((n+1):end,n) = 0;
  end
  
  
  %% Normalization
  % H' * H * ones(K,1) = const * ones(K,1) with const = 3/2

  % Energy per filter
  E_filter = sqrt(diag(H*H')); % same as E_filter = sqrt(sum(H.^2,2));

  E_filter(abs(E_filter)<eps) = 1;
  W = diag(1./E_filter);

  % Normalize each filter to energy = 1
  H = W*H;

  % Print out for debug
  if DEBUG, disp('done!');  end
  
end

%-------------------------------------------------------------------------%
% Hertz to Mel
function m = f2m(f)
  C = 1127;
  m = C*log(1+f./700);
end

%-------------------------------------------------------------------------%
% Mel to Hertz
function f = m2f(m)
  C = 1127;
  f = exp(m/C); 
  f = 700*(f-1);
end
