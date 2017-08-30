#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

static int is_hidden(char *name)
{
	return name[0] == '.' ? 0 : 1;
}
#if 0
void lsl(const char *path)
{
	DIR *dir;
	struct dirent *entry;
	printf("%s:\ntotal %d\n", path, 5);
	dir = opendir(path);
	while ((entry = readdir(dir)) != NULL) {
		if (is_hidden(entry->d_name)) {
			pperm(entry->d_type);
			printf("%d %s\n", entry->d_type, entry->d_name);
		}
	}
	closedir(dir);
	printf("\n");
}
#endif
static void p_nosuchfile(char *name)
{
	printf("ls: cannot access %s: No such file or directory\n", name);
}

static void p_file_stat(struct stat *s)
{
	char modestr[11] = "----------";
	mode_t m = s->st_mode;

	if (S_IRUSR & m) modestr[1] = 'r';
	if (S_IWUSR & m) modestr[2] = 'w';
	if (S_IXUSR & m) modestr[3] = 'x';

	if (S_IRGRP & m) modestr[4] = 'r';
	if (S_IWGRP & m) modestr[5] = 'w';
	if (S_IXGRP & m) modestr[6] = 'x';

	if (S_IROTH & m) modestr[7] = 'r';
	if (S_IWOTH & m) modestr[8] = 'w';
	if (S_IXOTH & m) modestr[9] = 'x';

	switch (S_IFMT & m) {
	case S_IFBLK:
		modestr[0] = 'b';
		break;
	case S_IFCHR:
		modestr[0] = 'c';
		break;
	case S_IFDIR:
		modestr[0] = 'd';
		break;
	case S_IFIFO:
		modestr[0] = 'p';
		break;
	case S_IFLNK:
		modestr[0] = 'l';
		break;
	case S_IFREG:
		if (S_ISVTX & m) modestr[9] = 's';
		if (S_ISUID & m) modestr[6] = 's';
		else if (S_ISGID & m) modestr[3] = 's';
		break;
	case S_IFSOCK:
		sprintf(modestr, "s");
		break;
	default:
		break;
	}
	printf("%s\n", modestr);
}

static int ls2(const char *path)
{
	struct stat filestat;
	DIR *dir;
	struct dirent *entry;

	if (lstat(path, &filestat) < 0)
		return -1;
	if (S_ISDIR(filestat.st_mode)) {
		printf("total %lld\n", (long long)filestat.st_blocks);
	} else {
		p_file_stat(&filestat);
	}
}

int main(int argc, char **argv)
{
	int i;
	if (argc == 1) {
		ls2(".");
	} else {
		for (i = argc-1; i > 0; i--) {
			if (-1 == ls2(argv[i])) {
				p_nosuchfile(argv[i]);
			}
		}
	}
	return 0;
}
