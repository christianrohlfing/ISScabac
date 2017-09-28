function [y, bits] = flEncode(x, R)
  % Encode
  y = logical(dec2bin(x, R)-'0');
  y = y(:)';
  
  % Count bits
  if nargout > 1, bits = length(y); end
end
