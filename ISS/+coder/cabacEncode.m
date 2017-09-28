function [nbits, ctxInit0] = cabacEncode(G,Nq,param)
%-------------------------------------------------------------------------%
% Encode integer values (between 0 and Nq-1) stored in G with CABAC
%
%   Max Bläser, Christian Rohlfing
%   (C) 2017 Institut für Nachrichtentechnik, RWTH Aachen University

  if nargin < 1, ISS(); return; end
  addpath('../CABAC')
  param.DEMO = parseinput(param,'DEMO',0);
  
  if nargin < 2, Nq = 2; end % number of quantization intervals
  
  % Binarization
  Gbin = num2cell(G);
  if Nq > 2
    fprintf('CABAC binarization...')
    Gbin = cellfun(@(x)cabacBinarizer(x,Nq,param.binMethod),Gbin,'UniformOutput',0);
    disp('done!')
  end
  
  % Initial probabilities for each context
  [ctxInit] = coder.cabacInitContextModel(Gbin,param.cmTypes,param.Nlbp);
  
  if param.equalProb
    ctxInit = 0.5*ones(size(ctxInit));
  end
  
  % Map to uint8 for transmittion simulation
  ctxInit0 = uint8( ctxInit*255 );
  ctxInit  = double(ctxInit0)/255; % this should match the dequantization done at decoderside
  
  % Create and initialize CABAC object
  c = cabacWrapper(ctxInit, param.fn);
  
  % Init
  c.encodeStart();
  
  % Statistics for debugging
  ctxHist = zeros(1,7*param.Nlbp+3); % Histogram of context selection
  ctxCost = ctxHist;
  H = zeros(size(G)); % Heat map
  
  fprintf('CABAC encoding...')
  for k=1:size(Gbin,2) % components      
    for d=1:size(Gbin,1) % either frequency f or time t
      % Binarized quantization index to encode
      g = Gbin{d,k};
      tmpBits = c.getNumBits();

      % Get neighboring binarized quantization indices for contex selection
      if d>1, g_up1 = Gbin{d-1,k}; else, g_up1=[]; end
      if d>2, g_up2 = Gbin{d-2,k}; else, g_up2=[]; end
      if k>1, g_lft = Gbin{d,k-1}; else, g_lft=[]; end

      % Loop over binarized quantization index  
      for cnt=1:length(g) 
        % Select corresponing contex
        ctxID = coder.cabacContextSelection(cnt,g(1:(cnt-1)),g_up1,g_up2,g_lft,param.cmTypes,param.Nlbp);
        ctxHist(ctxID) = ctxHist(ctxID) + 1;

        % Encode
        tmpBitsBin = c.getNumBits();
        c.encodeBin(g(cnt),ctxID-1);
        ctxCost(ctxID) = ctxCost(ctxID) + c.getNumBits() - tmpBitsBin;
      end % bin index
      H(d,k) = c.getNumBits()-tmpBits;
    end  % either frequency f or time t
    if mod(k,round(size(Gbin,2)/10))==0, fprintf('.'); end
  end % components
  disp('done!')
  
  % DEMO
  if param.DEMO
    % TODO: titleStrings
    % Create fancy titles for each plot
    n=1:param.Nlbp;
    titleStrings = {};
    % Prefix
    titleStrings = [titleStrings arrayfun(@(x)sprintf('$p(b_{%d}^{f,k}=0)$',x),n,'unif',0)]; % ctx_n
    titleStrings = [titleStrings arrayfun(@(x)sprintf('$p(b_{%d}^{f,k}=0 \\mid b_{%d}^{f-1,k}=0)$',x,x),n,'unif',0)]; % ctx_n,up0
    titleStrings = [titleStrings arrayfun(@(x)sprintf('$p(b_{%d}^{f,k}=0 \\mid b_{%d}^{f-1,k}=1)$',x,x),n,'unif',0)]; % ctx_n,up1
    titleStrings = [titleStrings arrayfun(@(x)sprintf('$p(b_{%d}^{f,k}=0 \\mid b_{%d}^{f,k}=1)$',x,x-1),n+1,'unif',0)]; % ctx_n_le1
    
    % Suffix
    titleStrings = [titleStrings arrayfun(@(x)sprintf('$p(b_{%d+N_p}^{f,k}=0)$',x),n,'unif',0)]; % ctx_n
    titleStrings = [titleStrings arrayfun(@(x)sprintf('$p(b_{%d+N_p}^{f,k}=0\\mid b_{%d+N_p}^{f-1,k}=0)$',x,x),n,'unif',0)]; % ctx_n,up0
    titleStrings = [titleStrings arrayfun(@(x)sprintf('$p(b_{%d+N_p}^{f,k}=0\\mid b_{%d+N_p}^{f-1,k}=1)$',x,x),n,'unif',0)]; % ctx_n,up1
    
    % ctx_rst prefix and suffix
    titleStrings = [titleStrings sprintf('$p(b_{n>%d}^{f,k}=0)$',param.Nlbp) sprintf('$p(b_{n+N_p>%d+N_p}^{f,k}=0)$',param.Nlbp)];
    cabacVisualize(c,ctxInit,ctxHist,H,titleStrings,param.Nlbp);
  end % DEMO
  
  
  % Tell engine to finish
  c.encodeFinish();
  
  % Get bits
  tmp = dir(param.fn);
  nbits = tmp.bytes*8;
end