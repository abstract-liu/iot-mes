#define TYPE_CMD_READ			0x32		/* read image*/
#define TYPE_CMD_READLEN		0x34		/* read image length */

#include "camera.h"
#include "base64.h"
/*camera ok*/
//#define UART_CAMERA_DIR	"/dev/ttySP0"
#define JPG_DIR			"/opt/sample.jpg"
#define JPGBASE64_DIR	"/opt/sample.base64.jpg"
#define SIZE_PIC		2*1024*1024	/*1M Bytes*/
#define SIZE_CMD		1*1024		/*1K Bytes*/
#define	FRM_SHEAD		0x56
#define	FRM_RHEAD		0x76
#define	IMAGE_ID		4
#define	IMAGE_QUALITY	8
#define	IMAGE_ADDR_S	6
#define	IMAGE_ADDR_E	10
#define	IMAGE_SIZE		8

int				JPG_ID;
int				JPGBASE64_ID;
extern unsigned char	pic[SIZE_PIC];
extern unsigned char	base64[SIZE_PIC];
extern int				UART_CAMERA_ID;
//unsigned char	temp[SIZE_CMD];


//int main()
int get_Image()
{
	int length = 0;
	int ret,i=0,reallen;

	//UART_CAMERA_ID	= uartInit( UART_CAMERA_DIR );
	JPG_ID			= open( JPG_DIR,O_CREAT|O_RDWR );

	if(JPG_ID < 0)
	{
		printf("Open /opt/sample.jgp failed\n");
		return -1;
	}
	
	printf("uart fd:%d\n",UART_CAMERA_ID);

	//JPGBASE64_ID			= open( JPGBASE64_DIR,O_CREAT|O_RDWR );
	/*if(JPGBASE64_ID < 0)
	{
		printf("Open /opt/sample.base64.jgp failed\n");
		return -1;
	}
	*/
	
	//printf("Send reset cmd\n");
	//camera_reset();
	//printf("Reset command return value\n");
	//camera_read_all();
	/* wait for restart */
	//sleep(5);
	//printf("init command return value\n");
	//camera_reset_read();

	//printf("send empty flash cmd\n");
	//camera_empty_flash();
	//camera_read_all();
	/*printf("send close motiondetection cmd\n");
	camera_close_motiondetection();
	camera_read_all();

	printf("send flash empty cmd\n");
	camera_empty_flash();
	camera_read_all();

	printf("send set quality cmd\n");
	camera_set_quality(0x36);
	camera_read_all();
	*/

	printf("send set size cmd\n");
	camera_set_size(0x11);
	camera_read_all();

	/*take picture command */
	printf("send take picture cmd\n");
	camera_take_picture();
	ret = camera_image_status();

	if(ret < 0 )
	{
		printf("Get picture status error:%d\n",ret);
		return 0;
	}


	/*get image length*/
	printf("send read length cmd\n");
	camera_read_length(0);
	length	= camera_image_length();
	//camera_image_length(&length);


	if(length <= 0 )
	{
		printf("Get picture length error:%d\n",length);
		return 0;
	}


	/*get image content*/
	printf("send read image cmd with length:%d\n",length);
	camera_read_image(0,0,length);
	ret		= camera_image_recv();

	if(ret < 0 )
	{
		printf("Get image content error:%d\n",ret);
		return 0;
	}

	/*raw jpg*/
	fchmod(JPG_ID,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	write(JPG_ID,pic+5,length);

	close(JPG_ID);

	/*base64 encode*/
	//reallen = base64_encode(pic+5, length, base64);

	//fchmod(JPGBASE64_ID,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	//write( JPGBASE64_ID,base64,reallen);
	//close(JPGBASE64_ID);
	

	//return reallen;
	return length;
}
int camera_init()
{
	return 0;
}
int camera_reset()
{
	char cmd[1024]={0x56,0x00,0x26,0x00};
	int	 len = 4;

	write( UART_CAMERA_ID, cmd, len );

	return 0;
}
int camera_empty_flash()
{
	char cmd[1024]={0x56,0x00,0x36,0x01,0x00};
	int	 len = 5;

	write( UART_CAMERA_ID, cmd, len );

	return 0;
}
int camera_set_size(int quality)
{
	char cmd[1024]={0x56,0x00,0x31,0x05,0x04,0x01,0x00,0x19,0x11};
	int	 len = 9;

	cmd[IMAGE_SIZE]	= (char)(quality&0x00ff);

	write( UART_CAMERA_ID, cmd, len );

	return 0;
}
int camera_set_quality(int quality)
{
	char cmd[1024]={0x56,0x00,0x31,0x05,0x01,0x01,0x12,0x04,0x36,0x01};
	int	 len = 9;

	cmd[IMAGE_QUALITY]	= quality;

	write( UART_CAMERA_ID, cmd, len );

	return 0;
}
int camera_getversion()
{
	char cmd[1024]={0x56,0x00,0x11,0x00};
	int	 len = 4;

	write( UART_CAMERA_ID, cmd, len );

	return 0;
}
int camera_take_picture()
{
	char cmd[1024]={0x56,0x00,0x36,0x01,0x00};
	int	 len = 5;

	write(UART_CAMERA_ID	,cmd,len);

	return 0;
}
/*
 * read camera image length
 * */
int camera_read_length(int imageid)
{
	char cmd[1024]={0x56,0x00,0x34,0x01,0x00};
	int	 len = 5;

	cmd[IMAGE_ID]	= imageid;

	write(UART_CAMERA_ID	,cmd,len);

	return 0;
}

int camera_read_image(int imageid,int imageaddr1,int imageaddr2)
{
	char	cmd[1024]={0x56,0x00,0x32,0x0C,0x00,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF};
	int		len = 16;
	int		i;
	unsigned char str[5];
	//char *itoa(int value, char *string, int radix);

	//printf("Start address:%d\tEnd address:%d\n",imageaddr1,imageaddr2);
	cmd[ IMAGE_ID ]		= imageid;
	memcpy(cmd+IMAGE_ADDR_S,&imageaddr1,4);

	str[0] = (unsigned char)( (imageaddr2&0xff000000) >>24);
	str[1] = (unsigned char)( (imageaddr2&0x00ff0000) >>16);
	str[2] = (unsigned char)( (imageaddr2&0x0000ff00) >>8);
	str[3] = (unsigned char)( (imageaddr2&0x000000ff) );

	memcpy(cmd+IMAGE_ADDR_E,str,4);

	//cmd[IMAGE_ADDR_S]	= imageaddr1;
	//cmd[IMAGE_ADDR_E]	= imageaddr2;
	write(UART_CAMERA_ID	,cmd,len);

	return 0;
}
int camera_set_motiondetection()
{
	char cmd[1024]={0x56,0x00,0x88,0x04,0x05,0x0A,0x00,0x64,0x05,0x05};
	int	 len = 8;

	//cmd[IMAGE_ID]	= imageid;

	write(UART_CAMERA_ID	,cmd,len);

	return 0;
}
int camera_set_motiondetection_arg()
{
	char cmd[1024]={0x56,0x00,0x31,0x05,0x01,0x01,0x1A,0x6E,0x03,0x05};
	int	 len = 9;

	//cmd[IMAGE_ID]	= imageid;

	write(UART_CAMERA_ID	,cmd,len);

	return 0;
}
int camera_open_motiondetection()
{
	char cmd[1024]={0x56,0x00,0x37,0x01,0x01,0x01,0x1A,0x6E,0x03,0x05};
	int	 len = 5;

	write(UART_CAMERA_ID	,cmd,len);

	return 0;
}
int camera_close_motiondetection()
{
	char cmd[1024]={0x56,0x00,0x37,0x01,0x00,0x01,0x1A,0x6E,0x03,0x05};
	int	 len = 5;

	write(UART_CAMERA_ID	,cmd,len);

	return 0;
}

/*int camera_read_image(int imageid)
{
	char cmd[1024]={0x56,0x00,0x37,0x01,0x00,0x01,0x1A,0x6E,0x03,0x05};
	int	 len = 5;

	write(UART_CAMERA_ID	,cmd,len);

	return 0;
}
*/

/*
 * carame image recv 串口接收线程
 * Send Command: FRM_HEADER + SEQ + COMMAND + LENGTH + Data
 * Recv Command: FRM_HEADER + SEQ + COMMAND + LENGTH + Data
 * */

int camera_image_recv()
{
	struct	timeval timeout = {5,0};		//select等待3秒，3秒轮询 sec usec
	fd_set	fs_read;
	int		i = 0,j=0,m=0,num = 0,totnum=0,n;
	unsigned char *temp = pic;
	int		curlen = 0,flag = 0;


	memset(pic,0,SIZE_PIC);
	if( 1 )
	{
		FD_ZERO(&fs_read);
		FD_SET(UART_CAMERA_ID	,&fs_read);
		timeout.tv_sec	= 5;
		timeout.tv_usec = 0;

		if((n = select(UART_CAMERA_ID + 1,&fs_read,NULL,NULL,&timeout)) < 0)
		{
			printf("error in UartRXThr, error code = %d\n", n);
			perror("select");
			return -10;
		}
		else
		{
			if( FD_ISSET( UART_CAMERA_ID	,&fs_read) )
			{
				while( (num = read(UART_CAMERA_ID ,temp+curlen,SIZE_PIC)) > 0 )
				{
					if( strncmp(temp,"error",strlen("error")) == 0 )
					{/*command exception*/
						return -20;
					}
					/*for(m=curlen;m<curlen+num;m++)
						printf("%02X ",temp[m]);
					printf("\n");
					*/

					for( i = curlen; i < curlen + num; i++)
					{
						if( (temp[i] == FRM_RHEAD) && (temp[i+2] == 0x32 ) && (flag == 0) )
						{/*check header*/ 
							flag = 1;

							printf("Recv image content curlen %d and flag:%d\n",i,flag);
							/*check image header FF D8 */
							if( temp[i+5] != 0xFF && temp[i + 6] != 0xD8 )
								return -5;
						}
						/* 0x76 0x00 0x32 0x00 0x00 */
						else if( (temp[i] == FRM_RHEAD)  && (temp[i+1] == 0x00)  &&
						      	 (temp[i+2] == 0x32 )    && (temp[i+3] == 0x00)  &&(flag == 1) )
						{/*check tailer*/
							/*check image tail FF D9 */
							if( temp[i-2] != 0xFF && temp[i-1] != 0xD9 )
								return -6;

							printf("Recv image content curlen %d and flag:%d\n",i,flag);
							return num -10;
						}
					}

					totnum += num;
					curlen += num;

					usleep(50*1000);
				}//end while
			}
		}
	}//end

	return -1;
}

int camera_image_length()
{
	struct	timeval timeout = {10,0};		//select等待3秒，3秒轮询 sec usec
	fd_set	fs_read;
	int		i = 0,num = 0,n;
	int		length = 0;
	unsigned char *temp = pic;
	unsigned char tint[5]={0,0,0,0,0};


	if( 1 )
	{
		FD_ZERO(&fs_read);
		FD_SET(UART_CAMERA_ID	,&fs_read);
		timeout.tv_sec	= 10;
		timeout.tv_usec = 0;

		if((n = select(UART_CAMERA_ID + 1,&fs_read,NULL,NULL,&timeout)) < 0)
		{
			printf("error in UartRXThr, error code = %d\n", n);
			perror("select");
			return -10;
		}
		else
		{
			if( FD_ISSET( UART_CAMERA_ID	,&fs_read) )
			{
				while( (num = read(UART_CAMERA_ID ,temp,SIZE_PIC)) > 0 )
				{
					if( strncmp(temp,"error",strlen("error")) == 0 )
					{/*command exception*/
						return -20;
					}

					if( temp[i] == FRM_RHEAD )
					{
						/*check cmd length*/
						if(num != 9)
							return -2;
						/*check cmd head*/
						if(temp[i] != FRM_RHEAD && temp[i + 2] != 0x34)
							return -3;

						//memcpy( length, temp+5, 4 );
						memcpy( tint, temp + 5, 4 );
						int len0 = (int)tint[0];
						int len1 = (int)tint[1];
						int len2 = (int)tint[2];
						int len3 = (int)tint[3];

						length	= (len0 << 24) + (len1 << 16) + (len2 << 8) + len3; 

						//printf("\nlen0 %d len1 %d len2 %d len3 %d temp:\t",len0,len1,len2,len3);

						for(i=0;i<num;i++)
							printf("%02X ",temp[i]);
						printf("\n");

						return length;
					}
				}//end while
			}
		}
	}//end

	return -1;
}


int camera_image_status()
{
	struct	timeval timeout = {10,0};		//select等待3秒，3秒轮询 sec usec
	fd_set	fs_read;
	int		i = 0,num = 0,n;
	int		length = 0;
	unsigned char *temp = pic;


	if( 1 )
	{
		FD_ZERO(&fs_read);
		FD_SET(UART_CAMERA_ID	,&fs_read);
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;

		if((n = select(UART_CAMERA_ID + 1,&fs_read,NULL,NULL,&timeout)) < 0)
		{
			printf("error in UartRXThr, error code = %d\n", n);
			perror("select");
			return -10;
		}
		else
		{
			if( FD_ISSET( UART_CAMERA_ID	,&fs_read) )
			{
				while( (num = read(UART_CAMERA_ID ,temp,SIZE_PIC)) > 0 )
				{
					if( strncmp(temp,"error",strlen("error")) == 0 )
					{/*command exception*/
						return -20;
					}

					if( temp[i] == FRM_RHEAD )
					{
						/*check cmd length*/
						if(num != 5)
							return -2;
						/*check cmd head*/
						if(temp[i] != FRM_RHEAD && temp[i + 2] != 0x36)
							return -3;

						for(i=0;i<num;i++)
							printf("%02X ",pic[i]);
						printf("\n");

						return 0;
					}
				}//end while
			}
		}
	}//end

	return -1;
}

int camera_reset_read()
{
	struct	timeval timeout = {10,0};		//select等待3秒，3秒轮询 sec usec
	fd_set	fs_read;
	int		i = 0,num = 0,n;
	int		length = 0;
	unsigned char *temp = pic;


	if( 1 )
	{
		FD_ZERO(&fs_read);
		FD_SET(UART_CAMERA_ID	,&fs_read);
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;

		if((n = select(UART_CAMERA_ID + 1,&fs_read,NULL,NULL,&timeout)) < 0)
		{
			printf("error in UartRXThr, error code = %d\n", n);
			perror("select");
			return -10;
		}
		else
		{
			if( FD_ISSET( UART_CAMERA_ID	,&fs_read) )
			{
				while( (num = read(UART_CAMERA_ID ,temp,SIZE_PIC)) > 0 )
				{
					printf("Return value:\t");
					for(i=0;i<num;i++)
						printf("%c ",temp[i]);
					printf("\n");
				}//end while
			}
		}
	}//end

	return -1;
}

int camera_read_all()
{
	struct	timeval timeout = {10,0};		//select等待3秒，3秒轮询 sec usec
	fd_set	fs_read;
	int		i = 0,num = 0,n;
	int		length = 0;
	unsigned char *temp = pic;

	if( 1 )
	{
		FD_ZERO(&fs_read);
		FD_SET(UART_CAMERA_ID	,&fs_read);
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;

		if((n = select(UART_CAMERA_ID + 1,&fs_read,NULL,NULL,&timeout)) < 0)
		{
			printf("error in UartRXThr, error code = %d\n", n);
			perror("select");
			return -10;
		}
		else
		{
			if( FD_ISSET( UART_CAMERA_ID	,&fs_read) )
			{
				while( (num = read(UART_CAMERA_ID ,temp,SIZE_PIC)) > 0 )
				{
					printf("Return value:\t");
					for(i=0;i<num;i++)
						printf("%02X ",temp[i]);
					printf("\n");
				}//end while
			}
		}
	}//end
	return -1;
}


