#include "list_directory.h"
#include "data_struct.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int list_directory(char* dev_name, char* target)
{
	printf("hihi read_device\n");
	printf("%s-%s\n", dev_name, target);
	printf("%d\n", (int)sizeof(struct BootEntry));

	struct BootEntry bootent;
	int disk = open(dev_name,O_RDONLY);
	pread(disk,&bootent,sizeof(bootent),0);

	int start_of_FAT = bootent.BPB_RsvdSecCnt * bootent.BPB_BytsPerSec;
	int start_of_Data = start_of_FAT + bootent.BPB_FATSz32 * bootent.BPB_NumFATs * bootent.BPB_BytsPerSec; 
	printf("start of FAT: %d\n", start_of_FAT);
	printf("start of Data Area: %d\n",start_of_Data);

	int num_of_dirent = (bootent.BPB_SecPerClus * bootent.BPB_BytsPerSec) / 32;
	printf("num_of_dirent: %d\n", num_of_dirent);
	

	//read FAT array
	int fat[bootent.BPB_FATSz32 * bootent.BPB_BytsPerSec / 4];
	

	int cluster_address = start_of_Data;

	int count = 1;

	int rec;
	for(rec=0; rec < num_of_dirent; rec++)
	{
		struct DirEntry dirent;
		pread(disk, &dirent, sizeof(dirent), cluster_address + rec * 32);

		if((dirent.DIR_Attr & 0x0F == 0x0F ) || (dirent.DIR_Name[0] == 0)) //skip LFN and empty entries
		{
			continue;
		}

		char filename[13];
		int i;
		//get filename
		for(i=0;dirent.DIR_Name[i] != 0x20;i++)
			filename[i] = dirent.DIR_Name[i];
		//place a dot if contains extension
		if(dirent,dirent.DIR_Name[8] != 0x20)
		{
			filename[i++] = '.';
			//get extension
			int j;
			for(j=8;dirent.DIR_Name[j] != 0x20;j++,i++)
				filename[i] = dirent.DIR_Name[j];
		}
		//end with NULL char
		filename[i] = '\0';

		int starting_cluster_num = (int)dirent.DIR_FstClusLO + ((int)dirent.DIR_FstClusHI)*16*16;
		printf("%d, %s", count++, filename);

		if((dirent.DIR_Attr & 0x10) == 0x10) //if is Directory, skip the remaining
			printf("/\n");
		else
			printf(", %d, %d\n", dirent.DIR_FileSize, starting_cluster_num);
			
	}

	close(disk);

	return 0;
}