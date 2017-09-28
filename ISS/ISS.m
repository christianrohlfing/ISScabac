function out = ISS(p)
%-------------------------------------------------------------------------%
% Informed source separation (ISS)
%
%   using non-negative tensor factorization (NTF) for source compression 
%   and context-based adaptive binary arithmetic coding (CABAC) for 
%   encoding the NTF parameters
%
%   Publication:
%     Bläser, M., Rohlfing, C., Gao, Y., & Wien, M. "Adaptive coding of
%     non-negative factorization parameters with application to informed
%     source separation", submitted to ICASSP 2018.
%
%   Publication for reference NTF-based ISS method (without CABAC)
%     Liutkus, A., Pinel, J., Badeau, R., Girin, L., & Richard, G. 
%     "Informed source separation through spectrogram coding and data
%     embedding". Signal Processing, 92(8), 1937-1949, 2012.
%
%   Max Bläser, Christian Rohlfing, Yingbo Gao 
%   (C) 2017 Institut für Nachrichtentechnik, RWTH Aachen University

  out = struct();
  
  %% Parameter
  DEMO = 0;
  if nargin < 1, p=struct(); DEMO = 1; end % Enable demo mode. This includes encoder-decoder match test for CABAC.
  
  % TF transform
  p.stft = parseinput(p,'stft',struct());
  p.stft.ws = parseinput(p.stft,'ws', 2^12); % window size
  p.stft.hs = parseinput(p.stft,'hs', 2^11); % hop size
  p.NMel = parseinput(p,'NMel', 400); % Mel-filtering spectral dimension of spectrogram. Disable by setting NMel to p.stft.ws/2+1

  % NTF
  p.ntf = parseinput(p,'ntf',struct());
  p.ntf.KperSource = parseinput(p.ntf,'KperSource',10); % number of components per source
  p.ntf.beta = parseinput(p.ntf,'beta',1); % beta-divergence parameter. Itakura-saito for beta=0, Kullback-Leibler for beta=1 and Euclidean for beta=2
  p.ntf.nIter = parseinput(p.ntf,'nIter',200); % number of iterations
  
  % Quantization
  p.quant = parseinput(p,'quant',struct());
  p.quant.N = parseinput(p.quant,'N', 8); % used for quantization of W and H
  p.quant.GMM = parseinput(p.quant,'GMM', 1); % use Lloyd-Max for finding optimal centroids
  p.quant.deadzoneQuant = parseinput(p.quant,'deadzoneQuant',0.7); % quantile for deadzone
  p.quant.log = parseinput(p.quant,'log',1); % quantize in linear (log=0) or logarithmic domain (log=1)
  p.quant.eps = parseinput(p.quant,'eps',1e-5);
  
  % CABAC
  p.cabac = parseinput(p, 'cabac', struct());
  p.cabac.binMethod = parseinput(p.cabac,'binMethod', 'DEC2EG0'); % binarization method
  p.cabac.cmTypes = parseinput(p.cabac,'cmTypes', {'cond0' 'cond1' 'conds0' 'conds1'}); % context model types
  p.cabac.Nlbp = parseinput(p.cabac,'Nlbp',3); % position of last bin to be modeled with contexts (rest-context for all other bins)
  p.cabac.equalProb = parseinput(p.cabac,'equalProb',0);
  
  % Random seed
  p.randomseed = parseinput(p,'randomseed',0); % Random seed for consistency
  rng(p.randomseed,'twister');

  % Display parameters
  disp(serialize2(p,'=',',',1,{'ntf' 'quant' 'cabac'}))
  fprintf('\tp.ntf = %s\n',serialize2(p.ntf))
  fprintf('\tp.quant = %s\n',serialize2(p.quant))
  fprintf('\tp.cabac = %s\n',serialize2(p.cabac))
  
  
  %% Read input 
  [s, x, Fs] = ReadInputSignals();
  
  
  %% Time-frequency transform
  % STFT mix
  [X,~,~,~,p.stft] = STFT(x, p.stft);
  p.stft.L = length(x);
  
  % STFT sources
  S = STFT(s, p.stft); Sabs = abs(S);
    
  % Mel filtering sources
  [Vs, MelFilt] = MelFiltering(Sabs,p.NMel,Fs);
  clear Sabs; % save memory!
  
  if p.ntf.beta == 0 % PSD for Itakura-Saito Distance, MSD else
    Vs = Vs.^2;
  end
  
  
  %% Compression
  disp('Coder...')
  p.ntf.K = p.ntf.KperSource*size(Vs,3); % number of total components
  
  % NTF Init
  [W0,H0,Q0] = betaNTFInit(S,p.ntf.K,MelFilt);
  clear S; % save memory!
  
  % NTF
  thetas = struct();
  [thetas.W,thetas.H,thetas.Q] = betaNTF(Vs,p.ntf.K,'beta',p.ntf.beta,'nIter',p.ntf.nIter,...
    'W',W0,'H',H0,'Q',Q0,'display',0);
  
  % Quality of the model at encoder (without quantization!)
  SDRsrc = quality(thetas, s, x, X, MelFilt, p.stft); out.SDRsrc = SDRsrc;
  
  % Quantization and coding
  [~, miscW] = quantizeWrapper(trans(thetas.W,p.quant.eps,p.quant.log), p.quant);
  [~, miscH] = quantizeWrapper(trans(thetas.H,p.quant.eps,p.quant.log), p.quant);
  
  % Side information struct
  % This is all information transmitted from encoder to decoder.
  param = struct();
  param.gW = miscW.group-1;
  param.cW = single(miscW.centroids);
  param.gH = miscH.group-1;
  param.cH = single(miscH.centroids);
  param.Q  = single(thetas.Q);
  disp('done');
  
  
  %% Decoder
  % We assume that all parameters gathered in the param structure are 
  % transmitted lossless here. See function getBits. We are only encoding
  % the parameters and not decoding for speeding up our simulations.
  % However, all decoder functionality is implemented and can be tested if
  % DEMO = 1.
  
  % Decoded matrices
  Wx0 = double(param.cW(param.gW+1)); [~, Wx0] = trans(Wx0,p.quant.eps,p.quant.log);
  Hx0 = double(param.cH(param.gH+1)); [~, Hx0] = trans(Hx0,p.quant.eps,p.quant.log);
  Qx0 = double(param.Q);
  
  % Quality at decoder
  thetatmp.W = Wx0; thetatmp.H = Hx0; thetatmp.Q = Qx0;
  [SDR, sest] = quality(thetatmp, s, x, X, MelFilt, p.stft); out.SDR = SDR;
  
  
  %% Lossless parameter encoding
  % We are omitting decoding for simulations and test encoder-decoder match
  % for a small subset only
  
  % Computation of parameter bitrate (kilobits per second per source)
  cRate = Fs/1000/(length(x))/size(Vs,3); out.cRate = cRate;
  
  % Bitrates obtained by different methods
  methods = {'CABAC' 'GZIP' 'Huffman'};
  for it=1:length(methods)
    method = methods{it};
    fprintf('Encoding with %s...\n',method)
    
    % Call encoding wrapper function
    nbits = getBits(param, method, p.cabac, DEMO); 
    rates.(methods{it}) = nbits*cRate; 
    disp('done!')
  end
  out.rates = rates;
  
  
  % Print SDR and rates
  fprintf('SDR %0.1f\n',mean(SDR))
  disp('Bitrates')
  disp(rates)
  
%   soundsc(sest(:,1),Fs); % listen to first estimated source
end


function nbits = getBits(data, method, cabacParam, DEMO)
%-------------------------------------------------------------------------%
% Evaluate bit-encoding methods
%   nbits = GEBITS(data, method)
%   returns the length of the bitstream obtained by using a bit-encoding 
%   algorithm specified by method
%
%   Input data is a structure with fields
%   - gW and gH quantization interval (group) indices between 0 and Nq-1 with Nq
%     quantization intervals
%   - cW and cH quantization centroids

  if nargin < 1, ISS(); return; end
  if nargin < 2, method = 'GZIP'; end % specifies the bit-encoding algorithm
  if nargin < 3, cabacParam = struct(); end % additional parameter struct for CABAC
  if nargin < 4, DEMO = 1; end % test encoder-decoder match
  
  
  switch method
    case 'CABAC'
      % Create random filenames
      fn = tempname;
      fnW = sprintf('%s_W.bin',fn);
      fnH = sprintf('%s_H.bin',fn);
            
      % Encode group indices
      cabacParam.fn = fnW; cabacParam.DEMO = DEMO;
      [bitsW, ctxInitW] = coder.cabacEncode(data.gW, length(data.cW), cabacParam);
      cabacParam.fn = fnH; cabacParam.DEMO = DEMO;
      [bitsH, ctxInitH] = coder.cabacEncode(data.gH, length(data.cH), cabacParam);
      
      % Encode rest (centroids and Q)
      paramRest=struct(); paramRest.cW = data.cW; paramRest.cH = data.cH; paramRest.Q = data.Q;
      if ~cabacParam.equalProb
        paramRest.xw = ctxInitW; paramRest.xh = ctxInitH; % transmit initial ctx probabilities
      end
      nbits = bitsW + bitsH + getBits(paramRest,'GZIP0');
      
      if DEMO % Test decoder match
        cabacParam.fn = fnW;
        gW = coder.cabacDecode(length(paramRest.cW), cabacParam, ctxInitW, size(data.gW));
        cabacParam.fn = fnH;
        gH = coder.cabacDecode(length(paramRest.cH), cabacParam, ctxInitH, size(data.gH));
        
        % Detect mismatch error
        assert(all(gW(:)==data.gW(:)),'Mismatch for W'); assert(all(gH(:)==data.gH(:)),'Mismatch for H');
      end
      
      % Clean up filenames
      delete(fnW); delete(fnH);
      
    case 'GZIP'
      % Encode gW and gH independently
      bitsW = getBits(data.gW,'GZIP0'); 
      bitsH = getBits(data.gH,'GZIP0');
      
      % Encode rest
      paramRest=struct(); paramRest.cW = data.cW; paramRest.cH = data.cH; paramRest.Q = data.Q;
      nbits = bitsW+bitsH+getBits(paramRest,'GZIP0');
      
    
    case 'GZIP0' % use MATLAB's save method
      % Create random filename
      fn = sprintf('%s.mat',tempname);

      % Save data to filename (MATLAB uses GZIP)
      save(fn,'data')

      % Get number of bits
      tmp = dir(fn);
      nbits = tmp.bytes*8;

      % Cleanup
      delete(fn);
      
      
    case 'Huffman'
      % Encode group indices
      [streamW,bitsW] = coder.huffEncode(data.gW(:)+1,length(data.cW));
      [streamH,bitsH] = coder.huffEncode(data.gH(:)+1,length(data.cH));
      
      % Encode rest
      paramRest=struct(); paramRest.cW = data.cW; paramRest.cH = data.cH; paramRest.Q = data.Q;
      nbits = bitsW+bitsH+getBits(paramRest,'GZIP0');
      
      if DEMO % Test decoder match
        gW = coder.huffDecode(streamW,length(paramRest.cW))-1;
        gH = coder.huffDecode(streamH,length(paramRest.cH))-1;
        
        % Detect mismatch error
        assert(all(gW(:)==data.gW(:)),'Mismatch for W'); assert(all(gH(:)==data.gH(:)),'Mismatch for H');
      end
      
  end
end


function [y,z] = trans(x,c,doLog)
%-------------------------------------------------------------------------%
% Transformation prior to quantization

  if nargin < 2, c = 0; end
  if nargin < 3, doLog = 1; end
  
  if doLog
    y = log(x+c);
  else
    y = x;
  end

  % Inverse
  if nargout > 1
    if doLog
      z = exp(x)-c;
    else
      z = x;
    end
  end
end


function [s,x,Fs] = ReadInputSignals()
%-------------------------------------------------------------------------%
% Read in test files

  [s1,Fs] = audioread('audio/git.wav');
  s2 = audioread('audio/drums.wav');
  
  s(:,1) = s1; s(:,2) = s2;
  x = sum(s,2);
end

