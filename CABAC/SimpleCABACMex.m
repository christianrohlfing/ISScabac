% SimpleCABACMex.m Help file for SimpleCABACMex MEX file 
%
% SimpleCABACMex provides an interface the to the CABAC engine
%   varargout = SimpleCABACMex(varargin) allows to run an encoder and a 
%   decoder. Usage as follows:
%   
%   To initialize the encoder or decoder, use
%   handle = SimpleCABACMex('initByProb', fn, ctxInit); 
%   or
%   handle = SimpleCABACMex('initByState', fn, ctxInit);
%   fn is the filename string to write / read the bits.
%   ctxInit is an 1xN array of N numbers of p(0) probabilities for 
%   'initByProb' or
%   ctxInit is an 3xN array of N numbers of mps, state and ctxId values for
%   'initByState'
%   
%   Encoding Steps:
%   1. Start the encoding engine
%   SimpleCABACMex('encodeStart', handle); 
%   2. Encode a bin into a context
%   SimpleCABACMex('encodeBin', handle, binValue, ctxId);
%   3. Code more bits and finally deactivate the coding engine
% 	SimpleCABACMex('encodeFinish', handle);
%   4. Optionally, before you finish encoding, retrieve some statistics
%   for a specific context or get information about the bits written
%   [trace, stats] = SimpleCABACMex('getEncoderStats',handle,ctxId);
%   [bits] = SimpleCABACMex('getNumBits',handle);
%
%   Decoding Steps: 
%   1. Start the decoding engine
%   SimpleCABACMex('decodeStart', handle); 
%   2. Decode a bin from a context
%   [decodedBin] = SimpleCABACMex('decodeBin', handle, ctxId);
%   3. Decode more bits and finally deactivate the coding engine
% 	SimpleCABACMex('decodeFinish', handle);
%   4. Optionally, before you finish decoding, retrieve some statistics
%   for a specific context
%   [trace, stats] = SimpleCABACMex('getEncoderStats',handle,ctxId);
%
%   Created with: 
%   MATLAB R2016b
%   Platform: win64
%   Microsoft Visual C++ 2015 

%   MEX File function.
