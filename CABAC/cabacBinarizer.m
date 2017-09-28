function C = cabacBinarizer(v,Nq,method)
%-------------------------------------------------------------------------%
% Binarize integer values v to bin-string C
%
%   Max Bläser, Christian Rohlfing
%   (C)2017 Institut für Nachrichtentechnik, RWTH Aachen University

  if nargin < 1, test(); return; end

  maxVal = Nq-1;
  switch method

    case 'DEC2TU' % Truncated Unary Coding
      C = TUCode(v,maxVal);

    case {'DEC2TR0' 'DEC2TR1' 'DEC2TR2'} % Truncated Rice Coding
      k = str2double(method(end));
      C = TRCode(v,k,maxVal);

    case {'DEC2EG0' 'DEC2EG1' 'DEC2EG2'} % Exp-Golomb Coding
      k = str2double(method(end));
      C = EGCode(v,k);

    case 'DEC2FL32'
      C = FLCode(v, 32);

  end
end

function C = TUCode(v,maxVal)
    if v == maxVal
      C = ones(1,maxVal);
    else
      C = ones(1,v+1);
      C(v+1)=0;
    end
end

function C = TRCode(v,k,maxVal)
    n_p = floor(v/2^k)+1;
    n_s = k;
    v_s = v - (n_p-1)*2^k;
    C = ones(1,n_p+n_s);
    C(n_p)=0;
    v_max = maxVal;
    if v>=v_max
      % TODO:
      % escape symbol, code remainign with exp-golomb
      C(n_p)=0;
    else
      % fixed length binarization with n_s bits
      C(n_p+1:end) = FLCode(v_s, n_s);
    end
end

function C = EGCode(v,k)
    % determine prefix and suffix lengths
    n_p = floor(log2((v/2^k)+1))+1;
    n_s = k+n_p-1;
    
    C = ones(1,n_p+n_s);
    % set the suffix terminating bit
    C(n_p) = 0;
    % suffix is n_s-bit representation of remainder
    v_s = v - 2^k*(2^(n_p-1)-1);
    
    % fixed length binarization with n_s bits
    C(n_p+1:end) = FLCode(v_s, n_s);
end

function C = FLCode(v,nbits)
  i=1:nbits;
  l = floor(v ./ 2.^(nbits-i));
  C = rem(l,2);
end