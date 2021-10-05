/*
 * MyUart.c
 *
 *  Created on: 2015年5月23日
 *      Author: ys
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include "myUart.h"


/*
 * 串口初始化
 * */
int uartInit115200(char * name)
{
	int fd = 0;

	if((fd = uart_open(fd,name))<0)
	{
		perror("open port error");
				exit(EXIT_FAILURE);
	}
	if(uart_set(fd,115200,8,'N',1)<0)
	{
		perror("set port error");
		exit(EXIT_FAILURE);
	}
	return fd;
}

int uartInit2(char * name,int baud)
{
	int fd = 0;

	if((fd = uart_open(fd,name))<0)
	{
		perror("open port error");
				exit(EXIT_FAILURE);
	}
	if(uart_set(fd,baud,8,'N',1)<0)
	{
		perror("set port error");
		exit(EXIT_FAILURE);
	}
	return fd;
}

int uartInit9600(char * name)
{
	int fd = 0;

	if((fd = uart_open(fd,name))<0)
	{
		perror("open port error");
				exit(EXIT_FAILURE);
	}
	if(uart_set(fd,9600,8,'N',1)<0)
	{
		perror("set port error");
		exit(EXIT_FAILURE);
	}
	return fd;
}
int uart_set(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio,oldtio;
    if  ( tcgetattr( fd,&oldtio)  !=  0)
    {
        perror("setup serial error!");
        return -1;
    }
    bzero( &newtio, sizeof( newtio ) );
    newtio.c_cflag  |=  CLOCAL;
    newtio.c_cflag  |=  CREAD;
    newtio.c_cflag &= ~CSIZE;

    switch( nBits )
    {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    }

    switch( nEvent )
    {
    case 'O':                     //奇校验
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'E':                     //偶校验
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'N':                    //无校验
        newtio.c_cflag &= ~PARENB;
        newtio.c_iflag &= ~INPCK;
        break;
    }

switch( nSpeed )
    {
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    case 230400:
        cfsetispeed(&newtio, B230400);
        cfsetospeed(&newtio, B230400);
        break;
    default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }
    if( nStop == 1 )
    {
        newtio.c_cflag &=  ~CSTOPB;
    }
    else if ( nStop == 2 )
    {
        newtio.c_cflag |=  CSTOPB;
    }
    newtio.c_cc[VTIME]  = 0;
    newtio.c_cc[VMIN] = 0;
    tcflush(fd,TCIFLUSH);
    if((tcsetattr(fd,TCSANOW,&newtio))!=0)
    {
        perror("com set error");
        return -1;
    }
    printf("set done!\n");
    return 0;
}

int uart_open(int fd,char * dev)
{
//	   char *dev[]={"/dev/ttyS0","/dev/ttyS1","/dev/ttyS2"};//"/dev/ttyUSB0"

	fd = open( dev, O_RDWR|O_NOCTTY|O_NDELAY);
	if (-1 == fd)
	{
	            perror("Can't Open Serial Port");
	            return(-1);
	}
	else
	{
	            printf("open%s OK ..... \n",dev);
	}


	    if(fcntl(fd, F_SETFL, 0)<0)
	    {
	        printf("fcntl failed!\n");
	    }
	    else
	    {
//	        printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
	    }
	    if(isatty(STDIN_FILENO)==0)
	    {
	        printf("standard input is not a terminal device\n");
	    }
	    else
	    {
//	        printf("isatty success!\n");
	    }
//	    printf("fd-open=%d\n",fd);
	    return fd;
}
