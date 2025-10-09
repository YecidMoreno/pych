clc
% close all
clearvars 
fileName = "/home/inception/git/pych/release/x86_64-linux-gnu/logs/SimplePD_1D_0.log";
fileName = "../remote/logs/SimpleCurrentMode_0.log";
D = readtable(fileName);
fieldnames(D)
% D.time = D.time/1000;
%%

f = figure(100);clf
subplot(311);hold on;
plot(D.time,D.theta_in,"DisplayName","\theta_in")
plot(D.time,D.theta_out,"DisplayName","\theta_out")
legend

subplot(312);hold on;
plot(D.time,D.dtheta_in,"DisplayName","\theta_in")
plot(D.time,D.dtheta_out,"DisplayName","\theta_out")
legend

subplot(313);hold on;
plot(D.time,D.u,"DisplayName","u")
legend

% figure
% subplot(211)
% plot(D.theta_in)
% subplot(212)
% plot(D.theta_out)