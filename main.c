#include "data_struct.h" // must declare this before list_directory.h
#include "list_directory.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int print_usage(char* argv0)
{
	printf("Usage: %s -d [device filename] [other arguments]\n",argv0);
	printf("-l target\t\tList the target directory\n");
	printf("-r target -o dest\tRecover the target pathname\n");
	return 0;
}

int main(int argc, char* argv[])
{
	unsigned char dev_name[1025];
	unsigned char target[1025];
	unsigned char dest[1025];

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

	//open & read boot entry
	struct BootEntry bootent;
	int disk = open(dev_name,O_RDONLY);
	pread(disk,&bootent,sizeof(bootent),0);

	//calculate start_of_FAT & start_of_Data
	unsigned int start_of_FAT = bootent.BPB_RsvdSecCnt * bootent.BPB_BytsPerSec;
	unsigned int start_of_Data = start_of_FAT + bootent.BPB_FATSz32 * bootent.BPB_NumFATs * bootent.BPB_BytsPerSec; 

	//read FAT array
	unsigned int fat_size = bootent.BPB_FATSz32 * bootent.BPB_BytsPerSec; //in bytes
	unsigned int fat[fat_size / 4];
	pread(disk, fat, fat_size, start_of_FAT);

	DiskInfo diskinfo;
	diskinfo.dev_name = dev_name;
	diskinfo.disk_fd = disk;
	diskinfo.fat = fat;
	diskinfo.start_of_Data = start_of_Data;
	diskinfo.byte_per_cluster = (bootent.BPB_SecPerClus * bootent.BPB_BytsPerSec);
	diskinfo.no_dirent_per_cluster = diskinfo.byte_per_cluster / 32;

	if(flag == 4)
	{
		list_directory(diskinfo,target);
	}
	else if(flag == 3)
	{
		if(recover_main(diskinfo,target,dest) == -1)
			print_usage(argv[0]);
	}
	else
	{
		print_usage(argv[0]);
	}

	close(disk);
	return 0;
}