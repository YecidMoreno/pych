clc
% close all
clearvars 
fileName = "/home/inception/git/pych/release/x86_64-linux-gnu/logs/SimplePD_1D_0.log";
D = readtable(fileName);
% D.time = D.time/1000;
%%
fnc = {@mean, @std, @min, @max}; cellfun(@(ff)ff(diff(D.time(2:end-1))),fnc)

f = figure(100);
clf(f), hold on
subplot(311); hold on; legend; grid on; ylabel("Position [deg]")
plot(D.time,D.pos,"DisplayName","\theta")
plot(D.time,D.pos_d,"DisplayName","\theta^d")
subplot(312); hold on; legend(Interpreter="latex");grid on; ylabel("Velocity[deg/s]")
plot(D.time,D.vel,"DisplayName","$\dot \theta_m$")
plot(D.time,D.vel_d,"DisplayName","$\dot \theta_d$")
subplot(313); hold on; legend; grid on; ylabel("units [a]")
plot(D.time,D.error,"DisplayName","error")
xlabel("Time [s]")
drawnow

figure(2)
plot(D.dt)
