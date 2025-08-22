

gcc -Wall -O2 -I . -I ../libmodbus -c ../libmodbus/modbus-data.c -o obj/modbus-data.o
gcc -Wall -O2 -I . -I ../libmodbus -c ../libmodbus/modbus-rtu.c -o obj/modbus-rtu.o
gcc -Wall -O2 -I . -I ../libmodbus -c ../libmodbus/modbus.c -o obj/modbus.o
gcc -Wall -O2 -I . -I ../libmodbus -c mb_io.c -o obj/mb_io.o

g++  -o mb_io obj/modbus-data.o obj/modbus-rtu.o obj/modbus.o obj/mb_io.o -s  

