function cabacVisualize(encoder,ctxInit,ctxHist,H,titleStrings,Ncols)
%-------------------------------------------------------------------------%
% Visualize CABAC state machine for used context.
%
%   Internally, CABAC uses 64 states and the value of the most probable
%   symbol (MPS). However, we visualize the state index histogram for MPS=0
%   and MPS=1 in the same plot. 
%   To evaluate the context design, two simple rules can help understand
%   the 
%   1) The value of MPS should not change too often. That means, that the
%      state distribution should remain either on the left side (MPS=0) or
%      on the right side (MPS=1).
%   2) A context is very efficient if the MPS probability is very high.
%      This means that the state at the edges (left-most edge for MPS=0 or
%      right-most edge for MPS=1, state ID in both cases equal to 63) 
%      should be selected quite often.
%
%   Max Bläser, Christian Rohlfing
%   (C) 2017 Institut für Nachrichtentechnik, RWTH Aachen University

  if nargin < 5, titleStrings = num2cell(0:(length(ctxInit)-1)); titleStrings = cellfun(@num2str,titleStrings,'unif',0); end
  if nargin < 6, Ncols = 4; end
  
  %% Map initial context probability (of '0' occurence) to state indices
  % Calculate MPS and initial state
  p0 = ctxInit;

  pLPS = zeros(size(p0));    
  mask = p0 >= 0.5; % 0 is MPS
  pLPS(mask) = 1-p0(mask);
  mask = ~mask; % 1 is MPS
  pLPS(mask) = p0(mask);

  % turn the LPS probability into the closest matching state
  stateInit = round(62 * log10(2.0 * pLPS) ./ log10(0.01875 / 0.5));
  stateInit(stateInit < 0) = 0; stateInit(stateInit > 62) = 62; % clipping

  % map initial state with respect to mps
  mask = p0 >= 0.5; % MPS = 0
  stateInit(mask) = -stateInit(mask)-1;
  mask = p0 < 0.5; % MPS = 1
  stateInit(mask) = stateInit(mask)+1;

  %% Plot state index histograms for each context
  fig=figure;fig.Color = [1 1 1];
  fs=10;fs2=9;textArgs = {'Interpreter','Latex','FontSize',fs};
  Ncols = min(Ncols,length(ctxInit));
  Nrows = ceil(length(ctxInit)/Ncols);
  
  it=0;
  for itr=1:Nrows
    for itc=1:Ncols
      it = it+1;
      if it > length(ctxInit), break; end
      ax=subplot(Nrows,Ncols,it);
      
      % Get debug trace from encoder
      trace = encoder.getEncoderStats(it-1); trace = trace'; 
      
      % Shift states
      state0 = double(trace(:,4));
      state = zeros(size(state0));
      mask = state0<=63; state(mask) = state0(mask)-64;
      mask = state0>=64; state(mask) = state0(mask)-63;
      
      % Histogram of state indices
      histogram(state,-63.5:1:63.5);
      ax.XLim = [-64 64]; grid(ax, 'on'); hold(ax, 'all');
      % Initial state
      line(ax,[stateInit(it) stateInit(it)], [0 ax.YLim(2)], 'Color', [1 0 0], 'LineWidth',2);
      % Zero line
      line(ax,[0 0], [0 ax.YLim(2)], 'Color', 0.25*[1 1 1], 'LineWidth',1);
      
      title(ax,sprintf('%s, total count: $%d$',titleStrings{it},length(state)),textArgs{:});
      
      % Beautify the plot
      ax.XTick = -60:20:60; ax.XTickLabel(1:3) = cellfun(@num2str,num2cell(60:-20:20),'unif',0); ax.TickLabelInterpreter = 'latex';ax.FontSize=fs2;
      t=text(ax,-35,0.85*ax.YLim(2),'MPS=$0$ $\longleftarrow$','BackgroundColor',0.75*[1 1 1],'HorizontalAlignment','right','VerticalAlignment','top',textArgs{:}); t.FontSize=fs2;
      t=text(ax,35,0.85*ax.YLim(2),'$\longrightarrow$ MPS=$1$','BackgroundColor',0.75*[1 1 1],'HorizontalAlignment','left','VerticalAlignment','top',textArgs{:}); t.FontSize=fs2;
    end
  end
  xlabel('State indices',textArgs{:}); ylabel('Count',textArgs{:}); %legend({'state id counts' 'initial state'},textArgs{:},'Location','Southeast');
  
  %% Plot heatmap
  figure;
  subplot(2,1,1)
  if isvector(H)
    plot(H); ylabel('Bits written per symbol')
  else
    imagesc(H); colorbar; 
  end
  title(sprintf('total bits=%d',sum(H(:))));
  
  %% Plot histogram for selected contexts
  subplot(2,1,2)
  bar(0:(length(ctxHist)-1),ctxHist);xlabel('Context ID');ylabel('Count');
end
