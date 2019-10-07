%% FFT implementation
%% Source signal generation
% Parameters of the signal
Fs = 44100;            % Sampling frequency                    
T = 1/Fs;             % Sampling period       
L = 1024;             % Length of signal
t = (0:L-1)*T;        % Time vector

% Signal definition
S = sin(2*pi*1000*t);

% Plot of the signal
figure
plot(1000*t,S)
title('Source signal')
xlabel('t (milliseconds)')
ylabel('S(t)')
grid on

%% FFT time decimation implementation
% FFT implementation
Y = FFTDIT(S);
% Matlab's implementation
Y1 = fft(S);

P2 = abs(Y/L);
P1 = P2(1:L/2+1);
P2a = abs(Y1/L);
P1a = P2a(1:L/2+1);
P1(2:end-1) = 4.8*P1(2:end-1);
P1a(2:end-1) = 2*P1a(2:end-1);
f = Fs*(0:(L/2))/L;


figure
plot(f,P1)
hold on
plot(f,P1a)
title('Single-Sided Amplitude Spectrum of X(t)')
xlabel('f (Hz)')
ylabel('|P1(f)|')
P2 = abs(Y/L);
P1 = P2(1:L/2+1);
P1(2:end-1) = 2*P1(2:end-1);
legend('Implemented','MATLAB')
grid on
xlim([0 2500])

function A = FFTDIT(a)
    n = length(a);
    A = zeros(1,n);
    y = 1:n;
    SignalRecod = bitrevorder(y);
    for i = 1:n
        SignalTemp = SignalRecod(i);
        A(SignalTemp) = a(i);
    end
    
    for S = 1:log2(n)
        m = 2^(S);
        j = sqrt(-1);
        Wm = exp(-j*2*pi/m);
        for k = 1:m:n-1 % For c implementation, start in 0
            W = 1;
            for i = 1:((m/2)-1) % For c implementation, start in 0
                t = W*A(k+i+(m/2));
                u = A(k+i);
                A(k+i) = u + t;
                A(k+i+(m/2)) = u - t;
                W = W * Wm;
            end
        end
    end
end
