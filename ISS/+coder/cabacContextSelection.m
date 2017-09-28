function ctxID = cabacContextSelection( n, g_prev, g_up1, g_up2, g_lft, types, Nlbp  )
%-------------------------------------------------------------------------%
% Select appropriate context for bin current bin at position n
%
%   Context ID mapping
%
%   Contexts for prefix:
%   context || ctx_n | ctx_n,up0 | ctx_n,up1 | ctx_n,le1 | ctx_rst
%   ID      || 1...N | N+1...2N  | 2N+1...3N | 3N+1...4N | 7N+1
%
%   Contexts for suffix:
%   context || ctx_n     | ctx_n,up0 | ctx_n,up1 | ctx_rst
%   ID      || 4N+1...5N | 5N+1...6N | 6N+1...7N | 7N+2
%
%   Note that we pass ID-1 to the CABAC engine (MATLAB likes to start
%   counting at 1, C/C++ at 0 ...).
%
%   Max Bläser, Christian Rohlfing
%   (C) 2017 Institut für Nachrichtentechnik, RWTH Aachen University

  if nargin<1, ISS(); return; end
  
  
  np_prev = find(~g_prev,1,'first');
  isPrefix_prev = true;
  if ~isempty(np_prev) && n > np_prev, isPrefix_prev = false; end
  
  np_up1  = find(~g_up1,1,'first');
  isPrefix_up1 = true;
  if ~isempty(np_up1) && n > np_up1, isPrefix_up1 = false; end
  
  
  if isPrefix_prev % we are in the prefix
    
    if n <= Nlbp
      ctxID = n; % default: no condition

      if length(g_up1) >= n && isPrefix_up1 % consider only up1 with prefix
        if g_up1(n) == 0 && any(ismember(types, 'cond0'))
          ctxID = Nlbp + n;
        elseif g_up1(n) == 1 && any(ismember(types, 'cond1'))
          ctxID = 2*Nlbp + n;
        end
      elseif n>1 && g_prev(n-1) == 1 && any(ismember(types, 'condbinlft')) 
        ctxID = 3*Nlbp + n-1;
      end

    else % select rest bin
      ctxID = 7*Nlbp + 1;
    end
    
  else % now we are in the suffix
    if n-np_prev <= Nlbp
      ctxID = 4*Nlbp + n - np_prev; % no condition
      
      if length(g_up1) >= n && ~isPrefix_up1
        if g_up1(n) == 0 && any(ismember(types, 'conds0'))
          ctxID = 5*Nlbp + n - np_prev;
        elseif g_up1(n) == 1 && any(ismember(types, 'conds1'))
          ctxID = 6*Nlbp + n - np_prev;
        end
      end
      
    else % select rest bin
      ctxID = 7*Nlbp + 2;
    end
  end
  
end

