
cansend can0 000#0100

# Fault reset
cansend can0 605#2240600000000000
cansend can0 605#2240600080000000

# Velocity mode
cansend can0 605#2F606000FE
# cansend can0 605#0F00FE

# Enable motor
cansend can0 605#2240600006000000  
cansend can0 605#2240600007000000  
cansend can0 605#224060000F000000  

sleep .5

cansend can0 305#C8000000
cansend can0 080#00

sleep 1

cansend can0 305#00000000
cansend can0 080#00

sleep 0.1
cansend can0 605#2240600000000000
cansend can0 605#2240600080000000