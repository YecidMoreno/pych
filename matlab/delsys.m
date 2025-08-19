clc
% close all
clearvars 
fileName = "/home/inception/git/pych/release/x86_64-linux-gnu/logs/Delsys.log";
D = readtable(fileName);
D.time = D.time/1000;


%%
d = abs(D.emg2);
clf
hold on
plot(D.time,D.emg3)
plot(D.time,D.emg_f3)
grid on
grid minor
xlabel("Time [s]")
ylabel("Activation [mV]")
title("Lateral Gastronemius")
% plot(D.time,movmean(d,50))
size(D,1)/D.time(end)
D.time(end)
[D.time_core(1) D.time_core(end)]