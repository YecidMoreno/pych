clc
clearvars 
fileName = "../logs/fes_force_sensor_t3.log";
D = readtable(fileName);
%%
f = figure(100);
clf(f), hold on
subplot(211); hold on; legend; grid on; ylabel("Current [mA]")
plot(D.time,D.fes0_r,"DisplayName","mA")
subplot(212); hold on; legend(Interpreter="latex");grid on; ylabel("Force[deg/s]")
plot(D.time,D.esp32_fsr0,"DisplayName","$FSR_0$")
xlabel("Time [s]")
drawnow
