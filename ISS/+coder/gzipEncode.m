function [fnBitstream,bits] = gzipEncode(x,dirBitstream,fnBitstream) %#ok<INUSL>
  if nargin < 2, dirBitstream = ''; end
  if nargin < 3, fnBitstream = [tempname '.mat']; end
  
  
  if ~isempty(dirBitstream)
    [~,fnBitstream,c]=fileparts(fnBitstream);
    fnBitstream = [dirBitstream filesep fnBitstream c];
  end
  
  %% Save mat file
  save(fnBitstream,'x','-v7');
  
  %% Get bits
  fileStruct = dir(fnBitstream);
  bits = fileStruct.bytes*8;
end