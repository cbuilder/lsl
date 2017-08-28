#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

int opt_filter_pass(char *name)
{
	return name[0] == '.' ? 0 : 1;
}

void lsl(const char *path)
{
	DIR *dir;
	struct dirent *entry;
	printf("%s:\ntotal %d\n", path, 5);
	dir = opendir(path);
	while ((entry = readdir(dir)) != NULL) {
		if (opt_filter_pass(entry->d_name))
			printf("%d %s\n", entry->d_type, entry->d_name);
	}
	printf("\n");
}

int main(int argc, char **argv)
{
	int i;
	if (argc == 1) {
		lsl(".");
	} else {
		for (i = argc-1; i > 0; i--)
			lsl(argv[i]);
	}
	return 0;
}
