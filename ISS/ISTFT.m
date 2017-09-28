function [x,w1,w2] = ISTFT(X,param,w2)
%-------------------------------------------------------------------------%
% Calculate STFT of time-domain signal x
%
%   Christian Rohlfing, Martin Spiertz
%   (C) 2012-2017 Institut für Nachrichtentechnik, RWTH Aachen University

  %% Parameter 
  ws = parseinput(param,'ws',2^12);
  hs = parseinput(param,'hs',2^10);
  
  if nargin<3
    % Generate window functions
    [w1,w2] = STFTWindow(ws,hs);
  end
  
  if size(X,1) == param.NI
    X = cat(1, X, conj( X(end-1:-1:2,:,:) ));
  end
  
  
  %% Start ISTFT
  x = zeros((size(X,2)-1)*hs+ws,size(X,3));
  for channel=1:size(X,3) 
    X(:,:,channel) = ifft(X(:,:,channel), 'symmetric');
    for col=1:size(X,2)
        startindex = (col-1)*hs+1;
        stopindex  = (col-1)*hs+ws;
        x(startindex:stopindex,channel) = x(startindex:stopindex,channel) + X(1:length(w2),col,channel).*w2;
    end
  end
  x = x(ws-hs+1:end,:);
  
  if isfield(param,'L')
    x = x(1:param.L,:);
  end
end