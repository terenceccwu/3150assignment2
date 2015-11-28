#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "list_directory.h"

int print_usage(char* argv0)
{
	printf("Usage: %s -d [device filename] [other arguments]\n",argv0);
	printf("-l target\t\tList the target directory\n");
	printf("-r target -o dest\tRecover the target pathname\n");
	return 0;
}

int main(int argc, char* argv[])
{
	char dev_name[255];
	char target[255];
	char dest[255];

	//check argument format
	if(argc <= 1 || strcmp(argv[1],"-d") != 0) //first option must be -d
	{
		print_usage(argv[0]);
		return 0;
	}

	char opt;
	opterr = 0; //disable default error message
	int flag = 0;
	while((opt = getopt(argc,argv,"d:l:r:o:")) != -1)
	{
		switch(opt)
		{
			case 'd':
				strcpy(dev_name, optarg);
				break;
			case 'l':
				strcpy(target, optarg);
				flag += 4;
				break;
			case 'r':
				strcpy(target, optarg);
				flag += 2;
				break;
			case 'o':
				strcpy(dest, optarg);
				flag += 1;
				break;
			default: // opt == '?' i.e. missing option argument or option not found
				print_usage(argv[0]);
				return 0;
		}
	}

	if(flag == 3 || flag == 4)
	{
		list_directory(dev_name,target);
	}
	else
	{
		print_usage(argv[0]);
		return 0;
	}

	return 0;
}