function [ str ] = serialize2( values,inter,sep,doGroup,ignoreKeys,appendIfValEmpty,doKeySort )
  if nargin < 2, inter = '=';  end
  if nargin < 3, sep = ',';  end
  if nargin < 4, doGroup = 1; end
  if nargin < 5, ignoreKeys = {}; end
  if nargin < 6, appendIfValEmpty = 1; end
  if nargin < 7, doKeySort = 0; end
  
  sep0 = sep;
  doGroup0 = doGroup;

  % Convert values to cell
  keys = repmat({''}, numel(values), 1);
  switch class(values)
    case 'struct'
      values = orderfields(values);
      keys = fieldnames(values);
      values = struct2cell(values);
    case 'char'
      values = {values};
    case 'cell'
      % do nothing
    otherwise
      values = num2cell(values);
  end
  
  % Disable grouping if singular non structured value
  if length(values) == 1 && isempty(keys{1})
    doGroup0 = 0;
  end
  
  if doKeySort
    [a,b] = sort(keys);
    keys = a;
    values = values(b);
  end
  
  str = '';
  
  % Append grouping string
  if doGroup0, str = [str, '(']; end
  
  % Iterate over values cell
  for iter=1:numel(values)
    val = values{iter};
    key = keys{iter};
    
    if ~ismember(key,ignoreKeys)
      % Set inter element to empty string if no key available
      inter0 = inter;
      if isempty(key), inter0 = ''; end

      % Expand if necessary
      if ~ischar(val) && (length(val) > 1 || isstruct(val) || iscell(val))
        valStr = serialize2(val,inter,sep,doGroup,ignoreKeys);
      else
        % Convert value to string
        switch class(val)
          case 'double'
            if round(val) == val % integer
              valStr = sprintf('%d',val);
            else
              valStr = sprintf('%2.2f',val);
            end
          case 'logical'
            valStr = sprintf('%d',val);
          case 'char'
            valStr = sprintf('%s',val);
          case 'cell'
            if ~isempty(val),
              valStr = sprintf('%s',val{1});
            else
              valStr = '';
            end
          otherwise
            error('Unknown class')
        end
      end

      % Append string
      if appendIfValEmpty || ~isempty(val)
        str = [str,key,inter0,valStr,sep0]; %#ok<AGROW>
      end
    end
  end
  
  % Remove last separation string
  ind = length(str)-(length(sep0)-1):length(str);
  if length(str) >=length(sep0) && strcmp(str(ind),sep0)
    str(ind) = []; 
  end
  
  % Append grouping string
  if doGroup0, str = [str, ')']; end
end

