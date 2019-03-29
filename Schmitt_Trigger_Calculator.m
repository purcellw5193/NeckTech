% The Shmitt Trigger considered here is an Op Amp comparator circuit with a
% hysteresis region.  That is, the input voltage must pass through a region
% rather than a single threshold value to trigger the comparator to switch.
% Refer to: 
% http://hyperphysics.phy-astr.gsu.edu/hbase/Electronic/schmitt.html#c3

%% Schmitt Trigger Calculator - Check section
clear;

% R1 is connected from ground to + input
% R2 is connected from Vref to + input
% R3 is connected from output to + input
% Triggering input is connected to - input

Vcc = 5;
Vref = 5;

% Modify resistor values to check switching levels
R1 = 7500;
R2 = 1600;
R3 = 1800;

% compute the parallel resistance value
R123 = (R1*R2)/(R1+R2);
R123 = (R123*R3)/(R123+R3);

% compute intermediate switching values
USL = R123*Vref/R2 + R123*Vcc/R3
LSL = R123*Vref/R2 - R123*Vcc/R3

%% Find best section
clear;

% Standard values for 5% resistors
SigFigs = [1.0 1.1 1.2 1.3 1.5 1.6 1.8 2.0 2.2 2.4 2.7 ...
        3.0 3.3 3.6 3.9 4.3 4.7 5.1 5.6 6.2 6.8 7.5 8.2 9.1];
    
% Ignore very large resistors (100,000x and 1Mx)
Multiplier = [1e1 1e2 1e3 1e4];

% set number of possible resistors
size = length(SigFigs)*length(Multiplier);
% create space for all possible resistor values
Rvals = zeros(1,length(SigFigs)*length(Multiplier));
% count the resistors
z = 1;

% compute the resistor values
for x = 1:length(SigFigs)   
    for y = 1:length(Multiplier)       
            Rvals(z) = SigFigs(x)*Multiplier(y);
            z = z+1;        
    end
end

% Vref is power supply to voltage divider Vcc goes to OPAmp
Vref = 5;
Vcc = 5;

% set space for best combination of resistors
best = [inf inf inf inf];
best = repmat(best,10,1);

% define upper and lower switching levels
USL = 4.5;
LSL = 0.25;

% define number of combinations to record
n = 10;

% test all possible combinations, record the best ones
for x = 1:size
   % pick R1
   R1 = Rvals(x);
    for y = 1:size
        % pick R2
        R2 = Rvals(y);
        for z = 1:size
           % pick R3 
           R3 = Rvals(z);
           % compute the parallel resistance value
           R123 = (R1*R2)/(R1+R2);
           R123 = (R123*R3)/(R123+R3);
           
           % compute intermediate switching values
           USLt = R123*Vref/R2 + R123*Vcc/R3;
           LSLt = R123*Vref/R2 - R123*Vcc/R3;
           
           % find the average deviation between upper and lower levels
           avg = (abs(USLt-USL) + abs(LSLt-LSL))/2;
           
           % record the combination if it is one of the n best
           for w = 1:n
               if avg < best(w,4)
                  % record
                  best(w,:) = [R1 R2 R3 avg];
                  % reset average to avoid double counting
                  avg = inf;
               end
           end
        end       
    end 
end

% print the results
best