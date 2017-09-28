classdef cabacWrapper < handle
%-------------------------------------------------------------------------%
% MATLAB class wrapper to an underlying mex function which encapsulates the 
% CABAC C++ class
%
%   Max Bläser, Christian Rohlfing, Yingbo Gao
%   (C) 2016-2017 Institut für Nachrichtentechnik, RWTH Aachen University

	properties
		bitStreamName;
		cabac_handle;
		contextModelInitOptions; % [ctxID mps probIndex; ctxID mps probIndex; ...]
	end
	methods
		function obj = cabacWrapper(cm,fn,initByProb)
			% contructor, set bitStreamName, context model
      if nargin < 3, initByProb=1; end
      
      if ischar(fn)
        obj.bitStreamName = fn;
      else
        error('bitStreamName should be string')
      end
      if ismatrix(cm)
        obj.contextModelInitOptions = cm;
      else
        error('context model initialization options should be like this: [ctxID mps probIndex, ctxID mps probIndex, ...]')
      end
      obj.init(initByProb)
		end
		function init(obj,initByProb)
      if nargin < 2, initByProb=1; end
			% initialize the cabac class in cpp
      if initByProb
        obj.cabac_handle = SimpleCABACMex('initByProb', obj.bitStreamName,obj.contextModelInitOptions);
      else
        obj.cabac_handle = SimpleCABACMex('initByState', obj.bitStreamName,obj.contextModelInitOptions);
      end
            
		end
		function encodeStart(obj)
			% encode start
			SimpleCABACMex('encodeStart', obj.cabac_handle);
		end
		function encodeBin(obj,binValue, ctxID)
			% encode bin
			if binValue == 1 || binValue == 0
				SimpleCABACMex('encodeBin', obj.cabac_handle, binValue, ctxID);
			else
				error('bin value to be encoded should either be 1 or 0');
			end
		end
		function encodeFinish(obj)
			% encode finish
			SimpleCABACMex('encodeFinish', obj.cabac_handle);
		end
		function decodeStart(obj)
			% decode start
			SimpleCABACMex('decodeStart', obj.cabac_handle);
		end
		function decodedBin = decodeBin(obj, ctxID)
			% decode bin
			decodedBin = SimpleCABACMex('decodeBin', obj.cabac_handle, ctxID);
		end
		function decodeFinish(obj)
			% decode finish
			SimpleCABACMex('decodeFinish', obj.cabac_handle);
    end
    function [trace, stats] = getEncoderStats(obj, ctxID)
      [trace, stats] = SimpleCABACMex('getEncoderStats',obj.cabac_handle,ctxID);
    end
    function [trace, stats] = getDecoderStats(obj, ctxID)
      [trace, stats] = SimpleCABACMex('getDecoderStats',obj.cabac_handle,ctxID);
    end
    function [bits] = getNumBits(obj)
      [bits] = SimpleCABACMex('getNumBits',obj.cabac_handle);
    end
	end
end