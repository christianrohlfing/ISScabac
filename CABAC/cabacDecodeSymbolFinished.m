function [isFinished, n, n_p, n_s] = cabacDecodeSymbolFinished(g,n,Nq, binMethod, n_p, n_s)
%-------------------------------------------------------------------------%
% Detect whether binarized symbol is fully retrieved
%
%   The detection is dependent on the binarization method
%
%   Max Bläser, Christian Rohlfing
%   (C) 2017 Institut für Nachrichtentechnik, RWTH Aachen University

  isFinished = false;
  switch binMethod
    case 'DEC2TU'
      isFinished = g(n) == 0 || n==Nq-1;
      
    case {'DEC2EG0' 'DEC2EG1' 'DEC2EG2'}
      egk = str2double(binMethod(end));               

      if n_s == -1 % decode prefix
        if g(n) == 0
          n_p = n_p + n; 
          n_s = egk+n_p-1;
          if n_s == 0, isFinished = true; end % already done, decoded 1
        end
      else % decode suffix
        if n_s == 1 % finished decoding all suffix bins
          isFinished = true;
        else
          n_s = n_s-1;
        end
      end

  end
end