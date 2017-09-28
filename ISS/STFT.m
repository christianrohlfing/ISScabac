function [X,xw,w1,w2,param,xw2] = STFT(x,param,w1)
%-------------------------------------------------------------------------%
% Calculate STFT of time-domain signal x
%
%   Christian Rohlfing, Martin Spiertz
%   (C) 2012-2017 Institut für Nachrichtentechnik, RWTH Aachen University

  %% Parameter
  if nargin < 2, param = struct(); end
  
  param.ws = parseinput(param,'ws',2^12); % Window size  
  param.hs = parseinput(param,'hs',2^10); % Hop size
  param.fftlen = parseinput(param,'fftlen',param.ws); % FFT length
  param.NI = round(param.fftlen/2)+1;
  appendPrepend = parseinput(param,'appendPrepend',1);
  
  DEBUG = parseinput(param,'DEBUG',1);
  
  if isa(x,'char'), X = param; return; end % UGLY!
  
  if nargin<3 || isempty(w1)
      % Generate window functions
    [w1,w2] = STFTWindow(param.ws,param.hs);
  else
    w2 = [];
  end
    
  if DEBUG, fprintf('Perform STFT %s...',serialize2(param)); end
  
  
  %% Initialization
  if appendPrepend
    append  = zeros(param.ws-param.hs,size(x,2));
    prepend = zeros(param.ws,size(x,2));
    x       = [append; x; prepend];
  end
  
  NumOfCols = floor((size(x,1)-param.ws)/param.hs)+1;
  X         = zeros(param.fftlen,NumOfCols,size(x,2));
  xw        = zeros(param.ws,NumOfCols,size(x,2));
  xw2       = zeros(param.ws,NumOfCols,size(x,2));

  
  %% Run STFT
  for channel=1:size(x,2)
    for col=1:NumOfCols
      startindex = floor((col-1)*param.hs+1);
      stopindex  = startindex+param.ws-1;
      
      % Segment and window
      xw2(:,col,channel) = x(startindex:stopindex,channel);
      xw(:,col,channel) = xw2(:,col,channel).*w1;
      
      % FFT
      X (:,col,channel) = fft(xw(:,col,channel),param.fftlen);
    end
  end
  
  
  %% Cut to nyquist index
  if isreal(x)
    X = X(1:param.NI,:,:);
  end
  if DEBUG, disp('done!'); end
end