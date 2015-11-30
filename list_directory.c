#include "data_struct.h"
#include "list_directory.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int parse_filename(unsigned char output[], unsigned char raw[])
{
	//get filename
	int i;
	for(i=0;(raw[i] != 0x20) && (i<8);i++)
		output[i] = raw[i];
	//place a dot if contains extension
	if(raw[8] != 0x20)
	{
		output[i++] = '.';
		//get extension
		int j;
		for(j=8;(raw[j] != 0x20) && (j<11);j++,i++)
			output[i] = raw[j];
	}
	//end with NULL char
	output[i] = '\0';

	if(output[0] == 0xe5) //replace 0xE5 in deleted files
		output[0] = '?';

	return 0;
}

int get_dirEntry(struct DirEntry* temp_dirent, DiskInfo diskinfo, unsigned int* cluster_num, unsigned int* rec)
{
	//traverse link list
	if(*cluster_num < 0x0ffffff8)
	{
		unsigned int cluster_address = diskinfo.start_of_Data + (*cluster_num - 2) * diskinfo.byte_per_cluster;

		if(*rec < diskinfo.no_dirent_per_cluster)
		{
			pread(diskinfo.disk_fd, temp_dirent, sizeof(*temp_dirent), cluster_address + *rec * 32);
			(*rec)++;

			if(*rec == 16) // if reaches last element of cluster, reset all pointers
			{
				*cluster_num = diskinfo.fat[(*cluster_num)];
				*rec = 0;
			}

		}
		return 1;
	}
	else
		return 0;
}


int print_entry(DiskInfo diskinfo, unsigned int cluster_num)
{
	unsigned int count = 1;

	//traverse link list
	struct DirEntry dirent;
	unsigned int rec = 0;
	while(get_dirEntry(&dirent, diskinfo, &cluster_num, &rec))
	{
		//skip LFN and empty entries
		if((dirent.DIR_Attr & 0x0F == 0x0F ) || (dirent.DIR_Name[0] == 0)) 
			continue;

		//get filename
		unsigned char filename[13];
		parse_filename(filename, dirent.DIR_Name);

		printf("%d, %s", count++, filename);

		//if is ./ or ../ Directory, skip the remaining
		if(((dirent.DIR_Attr & 0x10) == 0x10) && filename[0] == 0x2e)
		{
			printf("/\n");
			continue;
		}
		else if ((dirent.DIR_Attr & 0x10) == 0x10) //if it is a Directory
			printf("/");

		//print filesize and starting cluster num
		unsigned int starting_cluster_num = (unsigned int)dirent.DIR_FstClusLO + ((unsigned int)dirent.DIR_FstClusHI)* 0x10000;
		printf(", %u, %u\n", dirent.DIR_FileSize, starting_cluster_num);
	}

	return 0;
}

int find_directory(unsigned char target[], DiskInfo diskinfo, unsigned int cluster_num)
{
	//traverse link list
	struct DirEntry dirent;
	unsigned int rec = 0;
	while(get_dirEntry(&dirent, diskinfo, &cluster_num, &rec))
	{
		//skip normal files, LFN and empty entries
		if(((dirent.DIR_Attr & 0x10) != 0x10) || (dirent.DIR_Attr & 0x0F == 0x0F ) || (dirent.DIR_Name[0] == 0)) 
			continue;

		//get filename
		unsigned char filename[13];
		parse_filename(filename, dirent.DIR_Name);

		//compare
		if(strcmp(filename,target) == 0) //target found!
		{
			unsigned int starting_cluster_num = (unsigned int)dirent.DIR_FstClusLO + ((unsigned int)dirent.DIR_FstClusHI)* 0x10000;
			if(starting_cluster_num == 0) //re map cluster 0 to 2
				starting_cluster_num = 2;
			return starting_cluster_num;
		}
	}

	return 0;
}

int list_directory(DiskInfo diskinfo, unsigned char* target)
{
	//cluster to be print
	unsigned int cluster_num = 2; //defalut is 2 (root)

	//go into sub-directory
	char* temp = strtok(target,"/");

	while(temp)
	{
		//try to update cluster_num
		cluster_num = find_directory(temp, diskinfo, cluster_num);
		if(cluster_num == 0) //not found
		{
			printf("not found!\n");
			return 0;
		}
		temp = strtok(NULL,"/");
	}

	print_entry(diskinfo, cluster_num);

	return 0;
}