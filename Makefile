all: 
    gcc -o Pbl3 Pbl3.c -lmosquitto -lwiringPi -lwiringPiDev -I/usr/local/lib

run: ./Pbl3

clean: 
    rm -fr Pbl3