OBJS =  base64.o faceSearch.o camera.o CRC.o cQueue.o myQueue.o myUart.o info.o mcuio.o global.o print.o myMQTT.o rfid.o serialscreen.o sscreenupdate.o hmiFSM.o main.o
#INCLUDES = -I ./include/ -I mosquitto-1.4.11/lib/  sscreenupdate.o
INCLUDES = -I ./include/ 
CC = arm-linux-gcc
CXX = arm-linux-g++
CFLAGS = -g -Wall -c $(INCLUDES) 
LIBS = -L ./lib -lssl -lcurl -lcrypto 

#arm-linux-gcc test.c -I./org.eclipse.mosquitto/lib/ -L./org.eclipse.mosquitto/lib -lmosquitto -lpthread -lrt
mes:clean $(OBJS) 
	$(CXX) -o mes $(OBJS) $(LIBS) -lpthread -lsqlite3 -lrt -lmosquitto -lcjson -lstdc++ -lcurl
main.o : main.c  
	$(CC) $(CFLAGS) main.c  
hmiFSM.o : hmiFSM.c
	$(CXX) $(CFLAGS) hmiFSM.c  
serialscreen.o : serialscreen.c
	$(CC) $(CFLAGS) serialscreen.c  
sscreenupdate.o : 
	$(CC) $(CFLAGS) sscreenupdate.c
rfid.o : rfid.c
	$(CC) $(CFLAGS) rfid.c  
myMQTT.o : myMQTT.c
	$(CC) $(CFLAGS) myMQTT.c  
myQueue.o : myQueue.c
	$(CC) $(CFLAGS) myQueue.c   
cQueue.o : cQueue.c
	$(CC) $(CFLAGS) cQueue.c   
myUart.o : myUart.c
	$(CC) $(CFLAGS) myUart.c
myUart485.o : myUart485.c
	$(CC) $(CFLAGS) myUart485.c
CRC.o : CRC.c
	$(CC) $(CFLAGS) CRC.c
info.o : info.c
	$(CC) $(CFLAGS) info.c
mcuio.o : mcuio.c
	$(CC) $(CFLAGS) mcuio.c
global.o : global.c
	$(CC) $(CFLAGS) global.c
print.o : print.c
	$(CC) $(CFLAGS) print.c
faceSearch.o : faceSearch.cpp
	$(CXX) $(CFLAGS) -c faceSearch.cpp -o faceSearch.o
base64.o : base64.c
	$(CC) $(CFLAGS) -c base64.c -o base64.o
monitor:
	$(CC) $(CFLAGS) smartlibdaemon.c -o monitor1

 
clean :
	rm $(OBJS)  mes monitor -rf
update:
	scp ./smartlib root@10.82.80.234:/var/www/smartlibrary-update/
