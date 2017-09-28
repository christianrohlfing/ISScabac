function x = huffDecode(y,N)
  
  % Decode histogram
  z=y(1:N*8);
  y = y(N*8+1:end);
  h1 = coder.flDecode(z,N);
  h1d = double(h1*1.0);
  p = h1d/sum(h1d);
  
  % Decode data
  dict = huffmandict(1:N,p);
  x = huffmandeco(y,dict);
end