#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>


int8_t write_buf[1024];
int8_t read_buf[1024];



int main()
{
	int fd;
	char option;

	printf("welcome to the demo of charcter device driver\n");

//	fd = open("/dev/my_device",O_RDWR);
	fd = open("/proc/chr_proc",O_RDWR);
	if(fd <0 ){
		printf("cannot open device file ..\n");
		return 0;
	}

	do{
		printf("please enter your options\n");
		printf("1.write \n");
		printf("2. read\n");
		printf("3.exit\n");
		scanf(" %c",&option);
		printf("your option = %c\n",option);

		switch(option){
			case '1':
				printf("enter the string to write into driver\n");
				scanf(" %[^\t\n]s",write_buf);
				printf("data written  ");
				write(fd,write_buf,strlen(write_buf)+1);
				printf("done\n");
				break;
			case '2':
				printf("data is reading\n");
				read(fd,read_buf,1024);
				printf("done \n");
				printf("date = %s  \n\n",read_buf);

				break;
			case '3':

				close(fd);
				break;
			default: 
				printf("enter valid option");
				break;
		}
	}while(option != '3');
	close(fd);
}


