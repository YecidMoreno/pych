clc
close all
clear all

if ~isempty(instrfind())
    fclose(instrfind);
    delete(instrfind);
    clear instrfind;
end

HOST_IP = '0.0.0.0';

server = tcpserver(HOST_IP, 1515);

v = single([0,0]);

while true
    if server.NumBytesAvailable > 0
        data = readline(server);  % o use read(server, server.NumBytesAvailable)
        v(1) = 0;
        fprintf("Mensaje recibido: %s\n", data);
    end
    if server.Connected
        bytes = typecast(v, 'uint8');
        write(server, [uint8('@'), bytes, uint8('#')], 'uint8');
        v(1) = v(1)+1;
    end
    pause(.5);  % pequeña espera para evitar sobrecarga
end