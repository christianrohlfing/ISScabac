function [y,bits] = huffEncode(x,N)
  if nargin < 2, N = max(x); end
  
  % Histogram
  h = histcounts(x,N);
  h1 = uint8(h/sum(h)*255);
  h1 = max(h1,1);
  if min(x) < eps, x = x+1; end
  
  % Code data
  h1d = double(h1*1.0);
  p = h1d/sum(h1d);
  dict = huffmandict(1:N,p);
  y = huffmanenco(x,dict);
  
  % Code histogram
  h2 = coder.flEncode(h1,8)';
  h2 = h2(:);
  
  y = [h2;y];
  bits = length(y);
end
