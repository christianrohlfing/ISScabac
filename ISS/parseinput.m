function var = parseinput(opts, varname, default, varargin)

  if isfield(opts, varname)
    var = opts.(varname);
  else
    var = default;
  end
  
%   for narg = 1:2:nargin-4
%     cmd = varargin{narg};
%     arg = varargin{narg+1};
%     switch cmd
%       case 'instrset',
%         if ~any(strcmp(arg, var))
%           fprintf(['Wrong argument %s = ''%s'' - ', ...
%             'Using default : %s = ''%s''\n'], ...
%             varname, var, varname, default);
%           var = default;
%         end
%       otherwise,
%         error('Wrong option: %s.', cmd);
%     end
%   end

end

