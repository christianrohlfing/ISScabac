function [ctxInit] = cabacInitContextModel(Gbin, types, Nlbp)
%-------------------------------------------------------------------------%
% Initialize CABAC context model
%
%   Determine the initial probability of occurence of '0' for each context.
%   For an overview over all contexts, refere to cabacContextSelection.m
%
%   Max Bläser, Christian Rohlfing
%   (C) 2017 Institut für Nachrichtentechnik, RWTH Aachen University

  if nargin<1, ISS(); return; end
  % Initialize context model

  % Shift B
  Gbin_up1 = [num2cell(nan*ones(1,size(Gbin,2))); Gbin(1:end-1,:)];

  % Calculate length of prefixes
  np = cellfun(@(x)find(~x,1,'first'),Gbin,'unif',0);
  mask = cellfun(@isempty,np);
  np(mask) = cellfun(@length,Gbin(mask),'unif',0); np = cell2mat(np);
  np_up1 = [zeros(1,size(np,2)); np(1:end-1,:)];
  L = cellfun(@length,Gbin);
  L_up1 = cellfun(@length,Gbin_up1);
  
  % Iterate over given number of bins
  p_pre = zeros(1,Nlbp); pc0 = p_pre; pc1 = p_pre; p_bin_lft = p_pre; p_suf = p_pre; pcs0 = p_pre; pcs1 = p_pre;
  for n=1:Nlbp
    %%% Prefix %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %% Occurences of zeros in prefix per bin
    % p( b(n)_{d,k}=0 | n <= np_{d_k} ) for prefix
    Gtmp = Gbin(n <= np);
    mask = cellfun(@(x) x(n) == 0, Gtmp);
    if ~isempty(mask),  p_pre(n) = sum(mask) / length(mask); else, p_pre(n) = 0; end
    
    %% Occurences of zeros in prefix per bin if condition of previous symbol is met
    % p( b_{d,k}(n)=0 | b_{d-1,k}(n)=0, n <= np_{d_k}, n <= np_{d-1,k} )
    if any(ismember(types, 'cond0'))
      maskSel = n <= np & n <= np_up1;
      Gtmp = Gbin(maskSel); Gtmp_up1 = Gbin_up1(maskSel);
      mask = cellfun(@(x,y) x(n) == 0 && y(n) == 0, Gtmp, Gtmp_up1);
      if ~isempty(mask), pc0(n) = sum(mask) / length(mask); else, pc0(n) = 0; end % this is the joint probability p( b(n)=0, b_{d-1,k}(n)=0 )
      
      mask = cellfun(@(x) x(n) == 0, Gtmp_up1);
      if ~isempty(mask) && sum(mask) ~= 0,  norm = sum(mask) / length(mask); else, norm = 1; end
      pc0(n) = pc0(n) / norm; % now it's the conditional one p( b(n)=0 | b_{d-1,k}(n)=0 )  
    end
    
    % p( b_{d,k}(n)=0 | b_{d-1,k}(n)=1, n <= np_{d_k}, n <= np_{d-1,k}  )
    if any(ismember(types, 'cond1'))
      maskSel = n <= np & n <= np_up1;
      Gtmp = Gbin(maskSel); Gtmp_up1 = Gbin_up1(maskSel);
      mask = cellfun(@(x,y) x(n) == 0 && y(n) == 1, Gtmp, Gtmp_up1);
      if ~isempty(mask), pc1(n) = sum(mask) / length(mask); else, pc1(n) = 0; end  % this is the joint probability p( b(n)=0, b_{d-1,k}(n)=1 )

      mask = cellfun(@(x) x(n) == 1, Gtmp_up1);
      if ~isempty(mask) && sum(mask) ~= 0, norm = sum(mask) / length(mask); else, norm=1; end
      pc1(n) = pc1(n) / norm; % now it's the conditional one p( b(n)=0 | b_{d-1,k}(n)=1 )
    end
    
    %% Occurences of zeros in prefix per bin if condition of left bin is met
    % p( b(n)_{d,k}=0 | b(n-1)_{d,k} = 1, n>1, np_{d-1,k} < n)
    if any(ismember(types, 'condbinlft'))
      maskSel = n+1 <= np & np_up1 < n+1;
      Gtmp = Gbin(maskSel); 
      mask = cellfun(@(x) x(n+1) == 0 && x(n) == 1, Gtmp);
      if ~isempty(mask), p_bin_lft(n) = sum(mask)/length(mask); else, p_bin_lft(n) = 0; end % this is the joint probability % p( b(n)_{d,k}=0 , b(n-1)_{d,k}=1 | n>1, np_{d-1,k}<n)
      
      % p( b(n-1)_{d,k} = 1 | n>1, np_{d-1,k} < n)
      mask = cellfun(@(x) x(n) == 1, Gtmp);
      if ~isempty(mask) && sum(mask) ~= 0, norm = sum(mask) / length(mask); else, norm=1; end
      p_bin_lft(n) = p_bin_lft(n) / norm; % now it's the conditional one p( b(n)_{d,k}=0 | b(n-1)_{d,k}=1)
    end
    
    %%% Suffix %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %% Occurences of zeros in suffix per bin
    % p( b(n)_{d,k}=0 | n > np_{d_k}) for suffix
    maskSel = n+np <= L;
    Gtmp = Gbin( maskSel);
    mask = cellfun(@(x,a) x(n+a) == 0, Gtmp, num2cell(np(maskSel)));
    if ~isempty(mask), p_suf(n) = sum(mask) / length(mask); else, p_suf(n) = 0; end
    
    %% Occurences of zeros in suffix per bin if condition of previous symbol is met
    % p( b_{d,k}(n)=0 | b_{d-1,k}(n)=0, n > np_{d_k}, n > np_{d-1,k} )
    if any(ismember(types, 'conds0'))
      maskSel = n+np <= L & n+np_up1 <= L_up1;
      Gtmp = Gbin( maskSel ); Gtmp_up1 = Gbin_up1( maskSel );
      mask = cellfun(@(x,y,a,b) x(n+a) == 0 && y(n+b) == 0, Gtmp, Gtmp_up1,num2cell(np(maskSel)),num2cell(np_up1(maskSel)));
      if ~isempty(mask), pcs0(n) = sum(mask) / length(mask); else, pcs0(n) = 0; end  % this is the joint probability p( b(n)=0, b_{d-1,k}(n)=0 )
      
      mask = cellfun(@(x) x(n) == 0, Gtmp_up1);
      if ~isempty(mask) && sum(mask) ~= 0,  norm = sum(mask) / length(mask); else, norm = 1; end
      pcs0(n) = pcs0(n) / norm; % now it's the conditional one p( b(n)=0 | b_{d-1,k}(n)=0 )  
    end
    
    % p( b_{d,k}(n)=0 | b_{d-1,k}(n)=1, n > np_{d_k}, n > np_{d-1,k} )
    if any(ismember(types, 'conds1'))
      maskSel = n+np <= L & n+np_up1 <= L_up1;
      Gtmp = Gbin( maskSel ); Gtmp_up1 = Gbin_up1( maskSel );
      mask = cellfun(@(x,y,a,b) x(n+a) == 0 && y(n+b) == 1, Gtmp, Gtmp_up1,num2cell(np(maskSel)),num2cell(np_up1(maskSel)));
      if ~isempty(mask), pcs1(n) = sum(mask) / length(mask); else, pcs1(n) = 0; end  % this is the joint probability p( b(n)=0, b_{d-1,k}(n)=1 )
      
      mask = cellfun(@(x) x(n) == 1, Gtmp_up1);
      if ~isempty(mask) && sum(mask) ~= 0,  norm = sum(mask) / length(mask); else, norm = 1; end
      pcs1(n) = pcs1(n) / norm; % now it's the conditional one p( b(n)=0 | b_{d-1,k}(n)=1 )  
    end
    
  end
  
  % Now the rest: n > Ngp % prefix
  % p( b(n)_{d,k}=0 | n > Ngp, n <= np_{d,k} )
  n = Nlbp+1;
  maskSel = n <= np;
  Gtmp = Gbin( maskSel );
  nptmp = num2cell(np( maskSel ));
  Gtmp = cellfun(@(x,y)x(n:y), Gtmp, nptmp,'unif',false);
  Gtmp = [Gtmp{:}];
  mask = Gtmp==0;
  if ~isempty(mask), p_rest_pre = sum(mask) / length(mask); else, p_rest_pre = 0; end
  
  % n > Ngp % suffix
  % p( b(n)_{d,k}=0 | n > Ngp, n > np_{d,k} )
  Gtmp = Gbin( n > np & n <= L);
  Gtmp = cellfun(@(x)x(n:end), Gtmp,'unif',false);
  Gtmp = [Gtmp{:}];
  mask = Gtmp==0;
  if ~isempty(mask), p_rest_suf = sum(mask) / length(mask); else, p_rest_suf = 0; end

  % Concatenate
  ctxInit = [p_pre pc0 pc1 p_bin_lft p_suf pcs0 pcs1 p_rest_pre p_rest_suf];

end