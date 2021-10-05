#define	POS_SCN 1
#define	POS_CTL 3

#include <time.h>
char cmd[1024];
int scn_cmd_setsinglerecord(int scnid,int ctlid,int recordid)
{

	memset(cmd,0,1024);
	/* screen id */
	unsigned high = ((scnid>> 8 )& 0xff); 
	unsigned low  = (scnid & 0xff); 
	cmd[POS_SCN]	= (unsigned char)high;
	cmd[POS_SCN+1]	= (unsigned char)low;
	/* control id */
	cmd[POS_CTL]	= (char)(ctlid/255);
	cmd[POS_CTL+1]	= (char)ctlid&0xFF;
}
int main()
{
	time_t	t;
	int i;
	//scn_cmd_setsinglerecord(4,4,1);

	//for(i = 0;i<10;i++)
	//	printf(" %x",cmd[i]);
	//time();
	//int		timetick	= time(&t);
	//printf("%d\n",timetick);
	unsigned char p0 = 0x00;
	unsigned char p1 = 0x01;
	unsigned char p2 = 0x02;
	unsigned char p3 = 0x03;
	printf("test:%d\n",p0+(~p0));
	printf("test:%d\n",p1+(~p1));
	printf("test:%d\n",p2+(~p2));
	printf("test:%d\n",p3+(~p3));

	return 0;
}
