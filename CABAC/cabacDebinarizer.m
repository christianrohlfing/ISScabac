function v = cabacDebinarizer(c,Nq,method)
%-------------------------------------------------------------------------%
% De-binarize bin-string
%
%   Max Bläser, Christian Rohlfing
%   (C) 2017 Institut für Nachrichtentechnik, RWTH Aachen University

  if nargin < 1, coder.cabacBinarizer(); return; end

  maxVal = Nq-1;
  switch method
    case 'DEC2TU' % Truncated Unary Coding
      v = rTUCode(c,maxVal);

    case {'DEC2TR0' 'DEC2TR1' 'DEC2TR2'} % Truncated Rice Coding
      k = str2double(method(end));
      v = rTRCode(c,k,maxVal);

    case {'DEC2EG0','DEC2EG1' 'DEC2EG2'} % Exp-Golomb Coding
      k = str2double(method(end));
      v = rEGCode(c,k);

    case 'DEC2FL32' % Fixed-length coding with 32 bits
      v = rFLCode(c,32);
  end
end

function v = rTUCode(c,maxVal)
    v = find(~c,1,'first')-1;
    if isempty(v), v = maxVal; end % check for maxVal
end

function v = rTRCode(c,k,maxVal)
    % determine prefix length
    n_p = find(~c,1,'first');
    if isempty(n_p)
      v = maxVal;
    else
      n_s = k;
      xi = c; xi(1:n_p) = [];

      v = 2^k*(n_p-1) + rFLCode(xi,n_s);
    end
end

function v = rEGCode(c,k)
    % determine prefix length
    n_p = find(~c,1,'first');
    xi = c; xi(1:n_p) = [];
    n_s = length(xi);
    
    v = 2^k*(2^(n_p-1)-1) + rFLCode(xi,n_s);
end

function v = rFLCode(c,nbits)
  v = c*2.^((nbits-1):-1:0)';
end