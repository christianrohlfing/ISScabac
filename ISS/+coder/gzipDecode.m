function xz = gzipDecode(fn,rmFlag)
  if nargin < 2, rmFlag = 1; end
  
  %% Load mat file
  tmp = load(fn);
  xz = tmp.x;
  
  %% Delete file
%   if rmFlag, delete(fn); end
end