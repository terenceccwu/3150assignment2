int parse_filename(unsigned char output[], unsigned char raw[]);
int get_dirEntry(struct DirEntry* temp_dirent, DiskInfo diskinfo, unsigned int* cluster_num, unsigned int* rec);
int print_entry(DiskInfo diskinfo, unsigned int cluster_num);
int find_directory(unsigned char target[], struct DiskInfo diskinfo, unsigned int cluster_num);
int list_directory(DiskInfo diskinfo, unsigned char* target);