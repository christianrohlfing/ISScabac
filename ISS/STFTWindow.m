function [ w1,w2 ] = STFTWindow( ws,hs )
%-------------------------------------------------------------------------%
% Generate window function
%
%   ws/hs = c, c > 2:
%   E = mean((hann(ws*8,'periodic')).^2) = 3/8.
%   sum_t w1(t-n*hs).*w2(t-n*hs) != 1, therefore normalization with
%   sqrt(1/(c*E)) with (for one small interval, only c hann windows overlap!)
%
%   Martin Spiertz
%   (C) 2012 Institut für Nachrichtentechnik, RWTH Aachen University

  if(abs(ws/hs-2)<eps) % sinus
    w1 = hann(ws,'periodic').^0.5;
    w2 = w1;
  elseif(abs(ws/hs-4)<eps)
    w1 = hann(ws,'periodic')*sqrt(2/3);
    w2 = w1;
  elseif(abs(ws/hs-8)<eps)
    w1 = hann(ws,'periodic')*sqrt(1/3);
    w2 = w1;
  elseif(abs(ws/hs-16)<eps)
    w1 = hann(ws,'periodic')*sqrt(1/6);
    w2 = w1;
  elseif(abs(ws/hs-32)<eps)
    w1 = hann(ws,'periodic')*sqrt(1/12);
    w2 = w1;
  elseif(abs(ws/hs-1)<eps)
    w1=ones(ws,1);
    w2=w1;
  elseif(abs(ws/hs-3)<eps)
    w1  = sqrt(blackman(ws,'periodic'));
    tmp = [w1.^2;zeros(2*hs,1)]+[zeros(hs,1);w1.^2;zeros(hs,1)]+[zeros(2*hs,1);w1.^2];
    tmp = max(tmp);
    w1  = w1/sqrt(tmp);
    w2  = w1;
  elseif(and((ws/hs)<2,(ws/hs)>1))
    flank_length = ws-hs;
    w1 = hann(2*flank_length,'periodic').^0.5;
    w1 = [w1(1:flank_length);ones(ws-2*flank_length,1);w1(flank_length+1:end)];
    w2 = w1;
  end

end

