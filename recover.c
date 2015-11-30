#include "data_struct.h"
#include "list_directory.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int search_file(unsigned char target[], DiskInfo diskinfo, unsigned int cluster_num, unsigned int* size)
{
	//traverse link list
	struct DirEntry dirent;
	unsigned int rec = 0;
	while(get_dirEntry(&dirent, diskinfo, &cluster_num, &rec))
	{
		//skip non-deleted files, folders, LFN and empty entries
		if(((dirent.DIR_Attr & 0x10) == 0x10) || (dirent.DIR_Attr & 0x0F == 0x0F ) || (dirent.DIR_Name[0] == 0))
			continue;

		//get filename
		unsigned char filename[13];
		parse_filename(filename, dirent.DIR_Name);

		printf("'%s'+'%s'\n", filename+1, target+1);

		//if can pass the for loop == found!
		if(strcmp(filename+1, target+1) == 0)
		{
			unsigned int starting_cluster_num = (unsigned int)dirent.DIR_FstClusLO + ((unsigned int)dirent.DIR_FstClusHI)* 0x10000;
			*size = dirent.DIR_FileSize;

			if(diskinfo.fat[starting_cluster_num] != 0) //cluster is occupied by a newer file
			{
				printf("error - fail to recover\n");
				return 0;
			}
			return starting_cluster_num;
		}
	}
	return 0;
}

int recover_main(DiskInfo diskinfo, unsigned char target[], unsigned char dest[])
{
	printf("recover main!\n");

	int i;
	int num_of_slash = 0;
	for(i=0;target[i] != '\0';i++)
		if(target[i] == '/')
			num_of_slash++;

	//cluster to be print
	unsigned int cluster_num = 2; //defalut is 2 (root)

	//go into sub-directory
	char* temp = strtok(target,"/");

	for(i=0;i < num_of_slash - 1;i++)
	{
		printf("%s\n", temp);
		//try to update cluster_num
		cluster_num = find_directory(temp, diskinfo, cluster_num);
		if(cluster_num == 0) //not found
		{
			printf("directory not found!\n");
			return 0;
		}
		temp = strtok(NULL,"/");
	}
	//after loop, temp stores the filename

	printf("%s\n", temp);

	unsigned int starting_cluster_num;
	unsigned int size = 0; //to be pass back by search_file()
	if(!(starting_cluster_num = search_file(temp, diskinfo, cluster_num, &size)))
		printf("%s: error - file not found\n",temp);
	else
	{
		//calculate address
		unsigned int address = diskinfo.start_of_Data + (starting_cluster_num - 2) * diskinfo.byte_per_cluster;

		//get dirent again (for file size)
		struct DirEntry dirent;
		pread(diskinfo.disk_fd, &dirent, sizeof(dirent), address);

		printf("%u\n", address);
		printf("%u\n", size);

		//write to file
		int outfile = open(dest, (O_WRONLY|O_CREAT|O_TRUNC), (S_IRWXU|S_IRWXG|S_IRWXO));
		if (outfile == -1){
			printf("%s: failed to open\n", dest);
		}
		else{
			char buf[1024]; // buf size = 1024B

			int k;
			for(k=0; k < size / 1024; k++) //loop for several-1024 times
			{
				pread(diskinfo.disk_fd, buf, 1024, address + k*1024);
				pwrite(outfile, buf, 1024, k*1024);
			}
			pread(diskinfo.disk_fd, buf, size % 1024, address + k*1024);
			pwrite(outfile, buf, size % 1024, k*1024);
			printf("%s: recovered\n",temp);
		}
		close(outfile);

	}

	return 0;
}
