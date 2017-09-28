function cabacDemo()
%--------------------------------------------------------------------------
% CABAC Demo for illustration of the provided MATLAB interface
%
%   Max Bläser, Christian Rohlfing, Yingbo Gao 
%   (C) 2017 Institut für Nachrichtentechnik, RWTH Aachen University

  rng(0,'twister');
  
  %% Parameter
  param.N = 10000;
  param.Nq = 4;
  param.binMethod = 'DEC2TU';
  param.filename = 'test.bin';
  param.disableContexts = 0;
  param.fancyInit = 0;

  % check if the SimpleCABAC MEX file exists
  if ~(exist('SimpleCABACMex')==3) %#ok<EXIST>
    mex CXXFLAGS="\$CXXFLAGS -std=c++11" SimpleCABACMex.cpp
  end

  %% Test symbols
  % As a first step, we generate some random Gaussian distributed symbols,
  % which we want to encode
  symbols = abs(randn(1,param.N));

  % We use a simple AR(1) model to increase correlation
  rho = 0.8;
  symbols(2:end) = symbols(2:end)+rho*symbols(1:end-1);


  %% Quantization
  % We quantize our symbols to integer number with range 0 ... Nq-1
  q_mt = @(x,Delta,Jmax)(min(floor(x/Delta+0.5),Jmax)); % mid-tread quantizer
  Delta = quantile(symbols,0.99)/param.Nq; % quantization step size
  symbols = q_mt(symbols,Delta,param.Nq-1);

  
  %% Binarization
  % In the second step, we binarize the symbols using a TU code
  symbolsC = num2cell(symbols);
  binStringsEncode = cellfun(@(x)cabacBinarizer(x,param.Nq,param.binMethod),symbolsC,'UniformOutput',0);

  
  %% CABAC Initialization
  % Now, we initialize three contexts for coding
  if ~param.fancyInit
    ctxInit = 0.5*ones(1,3);
  else
    % Do a more sophisticated initialization
    % Our CABAC implementation needs only the probability of occurence '0'
    % per context
    ctxInit = zeros(1,3);
    
    % For the default context, we just count all zeros available
    allbins = [binStringsEncode{:}];
    ctxInit(1) = sum(allbins==0) / length(allbins);
    
    % For the conditional contexts, we need only the first bin of each
    % symbol and the first bin of the shifted (previous) symbol
    firstbin = cellfun(@(x)x(1),binStringsEncode);
    firstbin_shift = [nan firstbin(1:end-1)];
    
    ctxInit(2) = sum(firstbin==0 & firstbin_shift==1)/length(firstbin); % this is the joint probability
    norm = sum(firstbin_shift==1) / length(firstbin_shift);
    if norm ==0, norm= 1; end
    ctxInit(2) = ctxInit(2)/norm; % now we have the conditional probability
    
    ctxInit(3) = sum(firstbin==0 & firstbin_shift==0)/length(firstbin); % this is the joint probability
    norm = sum(firstbin_shift==0) / length(firstbin_shift);
    if norm ==0, norm= 1; end
    ctxInit(3) = ctxInit(3)/norm; % now we have the conditional probability
  end

  % Map to uint8 for transmittion simulation
  ctxInit0 = uint8( ctxInit*255 );
  ctxInit  = double(ctxInit0)/255; % this should match the dequantization done at decoderside

  % Create and initialize CABAC object
  encoder = cabacWrapper(ctxInit, param.filename);
  
  % Init
  encoder.encodeStart();
  
  % Statistics for debugging
  ctxHist = zeros(1,3); % Histogram of context selection
  H = zeros(size(symbols)); % Heat map
  
  %% CABAC Encoding
  fprintf('CABAC encoding...')
  for it=1:length(binStringsEncode) % symbols
    % Binarized symbol to encode
    g = binStringsEncode{it};
    tmpBits = encoder.getNumBits();

    % Get the left neighboring binarized symbols for contex selection
    if it>1, g_lft = binStringsEncode{it-1}; else, g_lft=[]; end

    % Loop over binarized symbols 
    for n=1:length(g) 

      % Selecting a context for coding:
      % We have three contexts available, a default context and 
      % two conditional contexts
      %
      % If the current bin position is 1 and if the first bin of the 
      % previous symbol was 1, we select the conditional context 1.
      % If the current bin position is 1 and if the first bin of the 
      % pervious symbol was 0, we select the conditional context 2.
      %
      % The decoder has to be able to do the same choice!
      if ~param.disableContexts && n==1 && ~isempty(g_lft)
        if g_lft(1) == 1
          ctxID = 1; % first bin of previous symbol was 1
        elseif g_lft(1) == 0
          ctxID = 2; % first bin of previous symbol was 0
        end
      else
        ctxID = 0; % select the default context if n ~= 1 or the conditional condition was not met
      end

      % Measure, how often each context was selected
      ctxHist(ctxID+1) = ctxHist(ctxID+1) + 1;

      % Encode with CABAC
      encoder.encodeBin(g(n),ctxID);

    end % bin index
    H(it) = encoder.getNumBits()-tmpBits;
  end % symbols
  
  % Visualize statistics
  cabacVisualize(encoder,ctxInit,ctxHist,H,{'ctx $0$' 'ctx $1$' 'ctx $2$'},1);
  
  
  encoder.encodeFinish();
  clear encoder;
  disp('done!')
  
  %% CABAC Decoding
  % we finished encoding, now let us decode 
  decoder = cabacWrapper(ctxInit, param.filename);
  decoder.decodeStart();
  
  % Init
  binStringsDecode = cell(size(symbolsC));
  
  % Decode with CABAC
  fprintf('CABAC decoding...')
  for it=1:length(binStringsDecode) % symbols
    % Get neighboring binarized quantization indices for contex selection
    if it>1, g_lft = binStringsDecode{it-1}; else, g_lft=[]; end
    g = []; n=1; isFinished = false; n_s=-1; n_p=0;
    while ~isFinished

      % Select a context as we did for the encoding;
      if ~param.disableContexts && n == 1 && ~isempty(g_lft)
        if g_lft(1) == 1
          ctxID = 1;
        elseif g_lft(1) == 0
          ctxID = 2;
        end
      else
        ctxID = 0;
      end

      % Decode a bin with CABAC
      g(n) = decoder.decodeBin(ctxID); %#ok<AGROW>

      % Check if all bins of the symbols are decoded
      [isFinished, n, n_p, n_s] = cabacDecodeSymbolFinished(g,n,param.Nq, param.binMethod, n_p, n_s);

      % Increment
      n=n+1;
    end
    binStringsDecode{it} = g;
  end
  
  % Tell engine to finish
  decoder.decodeFinish()
  disp('done!')
  
  %% Debinarization
  % Debinarize the symbols
  symbolsDecodedC = cellfun(@(x)cabacDebinarizer(x,param.Nq,param.binMethod), binStringsDecode, 'UniformOutput',0);
  symbolsDecoded = cell2mat(symbolsDecodedC);
  
  assert(all(symbols(:)==symbolsDecoded(:)),'Mismatch');
  tmp = dir(param.filename);
  nbits = tmp.bytes*8;
  
  fprintf('CABAC needs %d bits for encoding the test sequence.\n', nbits);
end