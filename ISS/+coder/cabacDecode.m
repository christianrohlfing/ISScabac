function G = cabacDecode(Nq,param,ctxInit,siz)
%-------------------------------------------------------------------------%
% Decode integer values (between 0 and Nq-1) stored in G with CABAC
%
%   Max Bläser, Christian Rohlfing
%   (C) 2017 Institut für Nachrichtentechnik, RWTH Aachen University

  if nargin < 1, ISS(); return; end
  addpath('../CABAC')
  if nargin < 2, Nq = 2; end % number of quantization intervals
    
  % Dequantize initial ctx probs
  ctxInit = double(ctxInit)/255;
  
  % Create and initialize CABAC object
  c = cabacWrapper(ctxInit, param.fn);
  
  % Decode with CABAC
  c.decodeStart();
  
  % Init
  Gbin = cell(siz);
  
  % Statistics for debugging
  ctxHist = zeros(1,7*param.Nlbp+2);
  
  % Decode with CABAC
  fprintf('CABAC decoding...')
  for k=1:size(Gbin,2) % components
    for d=1:size(Gbin,1) % either frequency f or time t
      % Get neighboring binarized quantization indices for contex selection
      if d>1, g_up1 = Gbin{d-1,k}; else, g_up1=[]; end
      if d>2, g_up2 = Gbin{d-2,k}; else, g_up2=[]; end
      if k>1, g_lft = Gbin{d,k-1}; else, g_lft=[]; end
      g = []; n=1; isFinished = false; n_s=-1; n_p=0;
      while ~isFinished
        % Select corresponing contex
        ctxID = coder.cabacContextSelection(n,g(1:(n-1)),g_up1,g_up2,g_lft,param.cmTypes,param.Nlbp);
        ctxHist(ctxID) = ctxHist(ctxID) + 1;

        % Decode
        g(n) = c.decodeBin(ctxID-1);

        % Check if binarized quantization index is fully retrieved        
        [isFinished, n, n_p, n_s] = cabacDecodeSymbolFinished(g,n,Nq, param.binMethod, n_p, n_s);

        % Increment
        n=n+1;
      end
      Gbin{d,k} = g;

    end

    if mod(k,round(size(Gbin,2)/10))==0, fprintf('.'); end
  end
  disp('done!')
  
  % Tell engine to finish
  c.decodeFinish()
  
  % De-binarization
  if Nq > 2
    G = cellfun(@(x)cabacDebinarizer(x,Nq,param.binMethod), Gbin, 'UniformOutput',0);
  else
    G = Gbin;
  end
  G = cell2mat(G);
end