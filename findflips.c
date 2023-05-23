/* findflips (c) 2023 Jacek Lipkowski SQ5BPF <sq5bpf@lipkowski.org>
 *
 * This is a simple utility to compare two files of the same size, 
 * show which bytes have changed, and which bits have changed.
 *
 * The output is supposed to be easy to read.
 *
 * I wrote it to ease reverse engineering of the configuration memory
 * of various radios and similar devices. 
 * 
 * Licensed under GPLv3. For more info please read the file LICENSE, 
 * which accompanies this program's sources.
 */
#include<stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void hdump_banner(int addr)
{
	printf("\n   0x%6.6x |0 |1 |2 |3 |4 |5 |6 |7 |8 |9 |a |b |c |d |e |f |\n",addr);
	printf("   ---------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+------------\n");

}

void hdump(unsigned char *buf,int len,int addr,char *s)
{
	int tmp1;
	char adump[80];
	int tmp2=0;
	int tmp3=0;
	unsigned char sss;
	char hexz[]="0123456789abcdef";

	int lasttmp;


	memset(&adump,' ',78);
	adump[78]=0;

	for (tmp1=0; tmp1<len; tmp1++)
	{
		tmp2=tmp1%16;
		if (tmp2==0) {
			if (tmp1!=0)  { printf("%2.2s 0x%6.6x: %.69s\n",s,addr+tmp3,adump); lasttmp=tmp1; }
			memset(&adump,' ',78);
			adump[78]=0;
			tmp3=tmp1;
		}
		sss=buf[tmp1];
		adump[tmp2*3]=hexz[sss/16];
		adump[tmp2*3+1]=hexz[sss%16];

		if (isprint(sss)) { adump[tmp2+50]=sss; } else adump[tmp2+50]='.';
	}
	if (lasttmp!=tmp1) printf("%2.2s 0x%6.6x: %.69s\n",s, addr+tmp3,adump);
}

void diffbits(unsigned char a,unsigned char b) {
	int i;
	/* from https://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format */
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
	((byte) & 0x80 ? '1' : '0'), \
	((byte) & 0x40 ? '1' : '0'), \
	((byte) & 0x20 ? '1' : '0'), \
	((byte) & 0x10 ? '1' : '0'), \
	((byte) & 0x08 ? '1' : '0'), \
	((byte) & 0x04 ? '1' : '0'), \
	((byte) & 0x02 ? '1' : '0'), \
	((byte) & 0x01 ? '1' : '0') 

	printf("0x%2.2x "BYTE_TO_BINARY_PATTERN"\n", a,BYTE_TO_BINARY(a));
	printf("0x%2.2x "BYTE_TO_BINARY_PATTERN"\n", b,BYTE_TO_BINARY(b));
	char buf[32];
	char buf2[2];
	buf[0]=0;
	buf2[0]=0;

	for (i=7;i!=-1;i--) {
		if ((a&(1<<i))!=(b&(1<<i))) { buf2[0]='0'+i; } else { buf2[0]=' '; }
		strcat(buf,buf2);  
	}
	printf("     %s\n",buf);


}


int main(int argc,char **argv) {
	unsigned char *bin1,*bin2;
	int file1,file2;
	struct stat stat1,stat2;
	int i,j,a,size,todump;

	if (argc<3) {
		printf("findflips - shows differences in single bits between two files\nUsage: %s file1 file2\nfile1 and file2 have to be the same size\n",argv[0]);
		exit(1);
	}


	file1=open(argv[1],O_RDONLY);
	if (file1<0) { fprintf(stderr,"open %s failed\n",argv[1]); exit(1); }

	file2=open(argv[2],O_RDONLY);
	if (file2<0) { fprintf(stderr,"open %s failed\n",argv[2]);  exit(1); }


	if (fstat(file1,&stat1)==-1) { fprintf(stderr,"stat %s failed\n",argv[1]); exit(1); }
	if (fstat(file2,&stat2)==-1) { fprintf(stderr,"stat %s failed\n",argv[2]); exit(1); }

	if (stat1.st_size!=stat2.st_size) { fprintf(stderr,"file sizes differ\n"); exit(1); }

	bin1=malloc(stat1.st_size);
	bin2=malloc(stat2.st_size);
	if ((!bin1)||(!bin2))  { fprintf(stderr,"malloc failed\n"); exit(1); }

	if (read(file1,bin1,stat1.st_size)!=stat1.st_size) { fprintf(stderr,"read %s failed\n",argv[1]); exit(1); }
	if (read(file2,bin2,stat2.st_size)!=stat1.st_size) { fprintf(stderr,"read %s failed\n",argv[2]); exit(1); }

	close(file1);
	close(file2);

	size=stat1.st_size;
	printf("d1: %s   d2: %s\n",argv[1],argv[2]);
	j=-1;
	for (i=0;i<size;i++) {
		if (bin1[i]!=bin2[i]) {

			if (j!=(i&0xfffffff0)) {
				j=i&0xfffffff0;

				todump=size-i; if (todump>0x10) todump=0x10;

				hdump_banner(j);
				hdump((unsigned char*)bin1+j,todump,j,"d1");
				hdump((unsigned char*)bin2+j,todump,j,"d2");

				char buf[128]=">            ";
				for(a=j;a<(j+todump);a++) {
					if (bin1[a]!=bin2[a]) { strcat(buf,"^^ "); } else { strcat(buf,"   "); }
				}
				printf("%s\n",buf);
			}

			printf("0x%4.4x: d1: 0x%2.2x (%3.3i)   d2: 0x%2.2x (%3.3i)\n",i,bin1[i],bin1[i],bin2[i],bin2[i]);
			diffbits(bin1[i],bin2[i]);
			printf("\n\n");
		}


	}

}
