%% FFT implementation
%% Generacion de la senal
% Parametros de la senal
Fs = 44100;           % Frecuencia de muestreo                    
T = 1/Fs;             % Tiempo de muestreo     
L = 8;             % Longitud de la senal
t = (0:L-1)*T;        % Vector de tiempo

% Definicion de la senal
S = [5 5 5 5 5 5 5 5];

%% Implementacion de la decimacion en tiempo de la FFT
% Implementacion de la FFT
Y = FFTDIT(S);
% Implementacion de MATLAB
Y1 = fft(S);

P2 = abs(Y/L);
P1 = P2(1:L/2+1);
P2a = abs(Y1/L);
P1a = P2a(1:L/2+1);
P1(2:end-1) = P1(2:end-1);
P1a(2:end-1) = P1a(2:end-1);
f = Fs*(0:(L/2))/L;

% Se grafica la senal en tiempo y frecuencia
figure
subplot(3,1,1)
plot(1000*t,S)
title('Source signal')
xlabel('t (milliseconds)')
ylabel('S(t)')
xlim([0 10])
grid on
subplot(3,1,2)
plot(f,P1)
title('Single-Sided Amplitude Spectrum of X(t) | Implemented')
xlabel('f (Hz)')
ylabel('|P1(f)|')
grid on
hold on
xlim([0 2000])
subplot(3,1,3)
plot(f,P1a)
title('Single-Sided Amplitude Spectrum of X(t) | MATLAB')
xlabel('f (Hz)')
ylabel('|P1(f)|')
P2 = abs(Y/L);
P1 = P2(1:L/2+1);
P1(2:end-1) = 2*P1(2:end-1);
grid on
xlim([0 2000])

function y = FFTDIT(x)
    N = length(x); % Se calcula el tamano del arreglo
    S = log2(N); % Se calcula la cantidad de etapas                                                       
    Half = 1; % Se pone el valor que indicara sobre la mitad que se esta trabajando
    x = bitrevorder(x); % Se codifica a traves de bit-reverse
    for Stage = 1:S % Se recorre por etapa
        for Butter = 0:(2^Stage):(N-1) % Se define la cantidad de mariposas por etapa
            for n = 0:(Half-1) % Maxima extension de la posicion relativa
                pos = n+Butter+1; % Se calcula la posicion relativa
                r = (2^(S-Stage))*n; % Text book definition
                Wn = exp((-1i)*(2*pi)*r/N); % Se calcula el factor complejo
                p = x(pos+Half).*Wn;
                a = x(pos)+p; % Se calcula Xm[p]
                b = x(pos)-p; % Se calcula Xm[q]
                x(pos) = a; % Se guarda en la posicion relativa
                x(pos+Half) = b; % Se guarda en la otra mitad respecto a la posicion relativa
            end
        end
    Half = 2*Half; % Se calcula la division
    end
    y = x; % Se retorna el resultado
end
