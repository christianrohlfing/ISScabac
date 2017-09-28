function [ x ] = flDecode( y, L)
  if nargin < 2, L = 1; end
  
  % Force row vector
  if iscolumn(y), y=y'; end
  
  % Reshape (if vector)
  y = reshape(y,L,[]);

  % Conversion
  x = bin2dec(char(y+'0'))';
      
end

