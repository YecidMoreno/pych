clc
% close all
clearvars 
% fileName = "../release/x86_64-alpine-linux-musl/SimplePD.log"
fileName = "../remote/logs/SimplePD_0.log";
% fileName = "/home/inception/git/pych/release/x86_64-linux-gnu/logs/SimplePD_1D_0.log";
D = readtable(fileName);
D.time = D.time/1000;
%%
fnc = {@mean, @std, @min, @max}; cellfun(@(ff)ff(diff(D.time(2:end-1))),fnc)

f = figure(100);
clf(f), hold on
subplot(311); hold on; legend; grid on; ylabel("Position [deg]")
plot(D.time,D.pos0,"DisplayName","\theta_m")
plot(D.time,D.pos1*10,"DisplayName","\theta_l")
subplot(312); hold on; legend(Interpreter="latex");grid on; ylabel("Velocity[deg/s]")
plot(D.time,D.d_pos0,"DisplayName","$\dot \theta_m$")
plot(D.time,D.d_pos1,"DisplayName","$\dot \theta_l$")
subplot(313); hold on; legend; grid on; ylabel("units [a]")
plot(D.time,D.error,"DisplayName","error")
plot(D.time,D.vel,"DisplayName","\mu")
xlabel("Time [s]")
drawnow
