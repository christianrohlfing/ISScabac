function [xq, misc] = quantizeWrapper(x,qParam)
%-------------------------------------------------------------------------%
% Quantization wrapper
%
%   Christian Rohlfing
%   (C) 2017 Institut für Nachrichtentechnik, RWTH Aachen University

  if nargin < 2, qParam = struct(); end
  
  qParam.quantileprob = parseinput(qParam,'quantileprob', [0 1]);
  qParam.eps = parseinput(qParam,'eps', 1e-5);
  qParam.deadzoneQuant = parseinput(qParam,'deadzoneQuant', []);
  
  % Generate column vector and save size for later
  shape = size(x);
  x = x(:);
  
  if isfield(qParam,'fixedCentroids') && ~isempty(qParam.fixedCentroids), fixedCentroids = qParam.fixedCentroids; else,fixedCentroids = []; end
  if isfield(qParam,'initCentroids') && ~isempty(qParam.initCentroids), centroidsInit = qParam.initCentroids; else, centroidsInit = []; end
  
  % Check for deadzone quantization (even if fixed centroids are given!)
  if ~isempty(qParam.deadzoneQuant)  
    deadzoneThresh = quantile(x,qParam.deadzoneQuant); % Calculate threshold dependent on data
    maskdz = x<deadzoneThresh;

    if ~sum(maskdz) == 0
      xcolsmall = x(maskdz);
      x(maskdz) = []; % remove deadzone data from input
      qParam.N = qParam.N-1; % decrease number of centroids
      if ~isempty(centroidsInit)
        centroidsInit(1) = [];
      end

      if ~isempty(fixedCentroids), fixedCentroids(1) = []; end
    end
  end

  % Quantize (without deadzone area)
  if ~isempty(fixedCentroids) % Quantize with given centroids
    [xqcol, misccol] = quantize(x, 'centroids', fixedCentroids');
  else % how to find centroids?
    if qParam.GMM % Estimate centroids with Lloyd's algorithm
      [xqcol,misccol] = quantizeLloyd(x, 'N', qParam.N, 'initCentroids', centroidsInit');

    else % Quantize with uniformly spaced centroids
      [xqcol, misccol] = quantize(x, 'control_method','RATE','control_param',qParam.N,...
        'border_param', qParam.quantileprob);
    end
  end

  misccol.edges = [min(min(x),min(misccol.centroids)), (misccol.centroids(2:end) + misccol.centroids(1:end-1))/2, max(max(x),max(misccol.centroids))];
    
  % Undo deadzone stuff
  if ~isempty(qParam.deadzoneQuant) && ~sum(maskdz) == 0

    % Add mean of deadzone area to centroids
    misccol.centroids = [mean(xcolsmall) misccol.centroids];

    % Adjust groups
    group = zeros(size(maskdz));
    group(~maskdz) = misccol.group+1;
    group(maskdz) = 1;
    misccol.group = group;
    misccol.delta = diff(misccol.centroids);

    % Adjust xq
    xqcol0 = xqcol;
    xqcol = zeros(size(maskdz));
    xqcol(~maskdz) = xqcol0;
    xqcol(maskdz) = misccol.centroids(1);
  end

  xq = xqcol;
  fixedCentroids = misccol.centroids';
  group = misccol.group;
  delta = misccol.delta;

  
  if isvector(xq)
    xq = reshape(xq,shape);
    group = reshape(group,shape);
  end
  
  misc = struct();
  misc.centroids = fixedCentroids;
  misc.group = group;
  misc.delta = delta;
end


function [xbar, misc] = quantizeLloyd(x,varargin)
%-------------------------------------------------------------------------%
% Lloyd's quantization
%
%   Christian Rohlfing
%   (C) 2017 Institut für Nachrichtentechnik, RWTH Aachen University

if nargin < 1, demo(); return; end
p = inputParser;
p.addParameter('initCentroids', [], @(x)isnumeric(x)); % Initial centroids
p.addParameter('N', 8, @(x)isnumeric(x)); % Number of centroids
p.addParameter('tol', eps, @(x)isnumeric(x)); % Stop criterion 
p.addParameter('nIter', 100, @(x)(isnumeric(x)&&(x==round(x)))); % maximal number of iterations
p.addParameter('deadzoneValue',[], @(x)isnumeric(x)); % Fix centroids
p.addParameter('deadzoneQuant',[], @(x)isnumeric(x)); % Fix centroids
p.addParameter('display', 0, @(x)isnumeric(x));


p.KeepUnmatched = true;
p.parse(varargin{:})
p = p.Results;
if ~isempty(p.deadzoneValue) &&~isempty(p.deadzoneQuant), error('This does not make sense!'); end

if ~isempty(p.deadzoneQuant)
  deadzoneThresh = quantile(x,p.deadzoneQuant); % Calculate threshold dependent on data
  maskdz = x<deadzoneThresh;
  xsmall = x(maskdz);
  x(maskdz) = [];
  p.N = p.N-1;
  if ~isempty(p.initCentroids), p.initCentroids(1) = []; end
end

% Initialization
if isempty(p.initCentroids)
  % Initial guess for centroids
  [xbar,misc] = quantize(x,'border_param',[0 1],'control_param',p.N);
else
  % Get initial group
  [xbar,misc] = quantize(x,'centroids', p.initCentroids);
end

% Centroids
centroids = misc.centroids;
if ~isempty(p.deadzoneValue), centroids(1) = p.deadzoneValue; end

% Quantization edges
edges = [min(min(x),min(centroids)), (centroids(2:end) + centroids(1:end-1))/2, max(max(x),max(centroids))];
group = misc.group;

% Quantization error (for now: Euclidean)
eq = zeros(1,p.nIter+1);
eq(1) = mean((x-xbar).^2); 
centroidsOld = centroids;
for it=1:p.nIter
  % Update centroids
  centroids = (edges(1:end-1)+edges(2:end))/2;
  
  % Non empty groups
  tmp1 = unique(group(:));
  for itt=1:length(tmp1)
    itg = tmp1(itt);
    mask = group==itg;
    centroids(itg) = mean(x(mask));
  end
  
  if ~isempty(p.deadzoneValue), centroids(1) = p.deadzoneValue; end
  
  % Update edges
  edges = [min(min(x),min(centroids)), (centroids(2:end) + centroids(1:end-1))/2, max(max(x),max(centroids))];
  
  % Update group
  [xbar, misc] = quantize(x,'centroids',centroids);
  centroids = misc.centroids;
  group = misc.group;
  
  % Quantization error
  eq(it+1) = mean((x-xbar).^2);
  
  % Stopping criterion
  if mean((centroids-centroidsOld).^2) < p.tol %%abs(eq(it+1)-eq(it)) < eps
    break;
  end 
  centroidsOld = centroids;
end
eq = eq(1:(it+1));
misc.eq = eq;


if ~isempty(p.deadzoneQuant)
  xbar0 = xbar;
  xbar = zeros(size(maskdz));
  x0 = x;
  x = zeros(size(maskdz));
  x(~maskdz) = x0;
  x(maskdz) = xsmall;
  misc.centroids = [mean(xsmall) misc.centroids];
  xbar(~maskdz) = xbar0;
  xbar(maskdz) = misc.centroids(1);
  group = zeros(size(maskdz));
  group(~maskdz) = misc.group+1;
  group(maskdz) = 1;
  misc.group = group;
end

end