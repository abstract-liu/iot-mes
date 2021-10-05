/*
 * myUDPClient.c
 *
 *  Created on: Aug 24, 2020
 *      Author: lj
 */


#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "mcuio.h"

#define	CMD_GETID		"#####100+GETID+0#####"
#define	BUF_SIZE		128
#define	MCUIO_IP		"127.0.0.1"
#define	MCUIO_PORT		"9200"

static void setnonblocking(int sockfd) 
{
	int flag = fcntl(sockfd, F_GETFL, 0);  //取标志

	if (flag < 0) 
	{
		perror("fcntl F_GETFL fail");
		return;
	}
	if (fcntl(sockfd, F_SETFL, flag | O_NONBLOCK) < 0) 
	{  //设置标志
		perror("fcntl F_SETFL fail");
	}
}

int getID()
{
	char	cmdid[128];
	int		sid,id;
	int		clifd=0;
	int		tolen=0,ret=0,on=1,buflen=0;
	char	buf[1024];

	clifd=socket(AF_INET,SOCK_DGRAM,0);//ipv4,SOCK_DGRAM=no connect
	if(clifd<0)
	{
		perror("socke failed");
		return -1;
	}

	memset(buf,0,BUF_SIZE);
	strcpy(buf,CMD_GETID);
	setsockopt(clifd,SOL_SOCKET,SO_REUSEADDR | SO_BROADCAST,&on,sizeof(on));

	setnonblocking(clifd);

	struct sockaddr_in seraddr	={0};
	seraddr.sin_family			=AF_INET;//ipv4
	seraddr.sin_addr.s_addr	=inet_addr(MCUIO_IP);
	seraddr.sin_port		=htons(MCUIO_PORT);
	tolen					=sizeof(seraddr);
	buflen					=strlen(buf);

	ret=sendto(clifd,buf,strlen(buf),0,(struct sockaddr *)&seraddr,sizeof(seraddr));
	if(ret<0)
	{
		perror("sendto failede");
		close(clifd);
		return -1;
	}
	sleep(1);

	ret=recvfrom(clifd,buf,sizeof(buf),0,NULL,NULL);
	if(ret<0)
	{
		perror("recvfrom failed");
		close(clifd);
		return -1;
	}

	printf("receive:%s\n",buf);
	sscanf(buf,"#####%d+%[a-zA-Z]+%d#####",&sid,cmdid,&id);

	close(clifd);

	return id;
}
int setLED(int i)
{
	//������socket������
	int clifd=0;
	clifd=socket(AF_INET,SOCK_DGRAM,0);//ipv4,SOCK_DGRAM=no connect
	if(clifd<0)
	{
		perror("socke failed");
		return -1;
	}
	printf("socket success\n");
	//������������������������
	if(1)
	{
		int tolen=0;
		int ret=0;
		int on=1;
		char buf[1024]={0};
		int buflen=0;
		//gets(buf);
		sprintf(buf,"#####100+SETLED+%d#####",i);
		printf("buf==%s\n",buf);
		setsockopt(clifd,SOL_SOCKET,SO_REUSEADDR | SO_BROADCAST,&on,sizeof(on));

	setnonblocking(clifd);

		struct sockaddr_in seraddr={0};
		seraddr.sin_family=AF_INET;//ipv4
		seraddr.sin_addr.s_addr=inet_addr("localhost");
		seraddr.sin_port=htons(9200);
		tolen=sizeof(seraddr);
		buflen=strlen(buf);
		printf("sendto:  %s\n lenth:%d\n",buf,buflen);
		ret=sendto(clifd,buf,strlen(buf),0,(struct sockaddr *)&seraddr,tolen);
		if(ret<0)
		{
			perror("sendto failede");
			close(clifd);
			return -1;
		}
		printf("sendto success\n");
		//���������������������������������
		ret=recvfrom(clifd,buf,sizeof(buf),0,NULL,NULL);
		if(ret<0)
		{
			perror("recvfrom failed");
			close(clifd);
			return -1;
		}
		printf("recvfrom success\n");
		printf("receive:%s\n",buf);
	}
	close(clifd);

	return 0;
}
int reSetLED(int i)
{
	//������socket������
	int clifd=0;
	clifd=socket(AF_INET,SOCK_DGRAM,0);//ipv4,SOCK_DGRAM=no connect
	if(clifd<0)
	{
		perror("socke failed");
		return -1;
	}
	printf("socket success\n");
	//������������������������
	if(1)
	{
		int tolen=0;
		int ret=0;
		int on=1;
		char buf[1024]={0};
		int buflen=0;
		//gets(buf);
		sprintf(buf,"#####100+RESETLED+%d#####",i);
			printf("buf==%s",buf);
		setsockopt(clifd,SOL_SOCKET,SO_REUSEADDR | SO_BROADCAST,&on,sizeof(on));

	setnonblocking(clifd);

		struct sockaddr_in seraddr={0};
		seraddr.sin_family=AF_INET;//ipv4
		seraddr.sin_addr.s_addr=inet_addr("localhost");
		seraddr.sin_port=htons(9200);
		tolen=sizeof(seraddr);
		buflen=strlen(buf);
		printf("sendto:  %s\n lenth:%d\n",buf,buflen);
		ret=sendto(clifd,buf,strlen(buf),0,(struct sockaddr *)&seraddr,tolen);
		if(ret<0)
		{
			perror("sendto failede");
			close(clifd);
			return -1;
		}
		printf("sendto success\n");
	//���������������������������������
		ret=recvfrom(clifd,buf,sizeof(buf),0,NULL,NULL);
		if(ret<0)
		{
			perror("recvfrom failed");
			close(clifd);
			return -1;
		}
		printf("recvfrom success\n");
		printf("receive:%s\n",buf);
	}
	close(clifd);

	return 0;
}

int getMac(char*pmac)
{
	struct ifreq ifreq;
	char mac[20]={0};
	int sockfd;

	//GatewayRegister
	if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0)
	{
		perror("socket");
		return 2;
	}

	strcpy(ifreq.ifr_name,"eth0");
	if(ioctl(sockfd,SIOCGIFHWADDR,&ifreq)<0)
	{
		perror("ioctl");
		return 3;
	}

	//sprintf(mac,"%02X-%02X-%02X-%02X-%02X-%02X",
	sprintf(mac,"%02X%02X%02X%02X%02X%02X",
			(unsigned char)ifreq.ifr_hwaddr.sa_data[0],
			(unsigned char)ifreq.ifr_hwaddr.sa_data[1],
			(unsigned char)ifreq.ifr_hwaddr.sa_data[2],
			(unsigned char)ifreq.ifr_hwaddr.sa_data[3],
			(unsigned char)ifreq.ifr_hwaddr.sa_data[4],
			(unsigned char)ifreq.ifr_hwaddr.sa_data[5]);

	memcpy(pmac,mac,20);

	return 0;
}
