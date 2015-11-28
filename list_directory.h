int print_entry(int disk, unsigned int fat[], unsigned int cluster_num);
int find_directory(unsigned char target[], int disk, unsigned int fat[], unsigned int cluster_num);
int list_directory(char* dev_name, char* target);