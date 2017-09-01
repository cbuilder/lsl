#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

static int is_hidden(char *name)
{
	return name[0] == '.' ? 1 : 0;
}

static void p_nosuchfile(char *name)
{
	printf("ls: cannot access %s: No such file or directory\n", name);
}

static void p_rights(struct stat *s)
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
	printf("%s ", modestr);
}

static void p_links(struct stat *s)
{
	printf("%lu ", s->st_nlink);
}

static void p_usrown(struct stat *s)
{
	printf("%d ", s->st_uid);
}

static void p_grpown(struct stat *s)
{
	printf("%d ", s->st_gid);
}

static void p_size(struct stat *s)
{
	printf("%lu ", s->st_size);
}

static void p_timestamp(struct stat *s)
{
	struct tm *lt = localtime(&s->st_mtime);
	printf(" %s", asctime(lt));
}

static void print_about_file(const char* name, struct stat *s)
{
	p_rights(s);
	p_links(s);
	p_usrown(s);
	p_grpown(s);
	p_size(s);
	//p_timestamp(s);
	printf(" %s\n", name);
}

static void print_about_dir(struct stat *s)
{
	printf("total %lld\n", (long long)s->st_blocks);
}

static struct dirent ** pdirent_array_alloc(struct dirent **ents, DIR *dirp)
{
	unsigned long filcnt = 0;
	struct dirent *entry;
	while ((entry = readdir(dirp)) != NULL)
		if (!is_hidden(entry->d_name))
         		filcnt++;
	rewinddir(dirp);
	ents = calloc(sizeof(entry), filcnt);
	printf("%lu files\n", filcnt);
	return ents;
}

static unsigned add_list_direntry(struct dirent **entries, struct dirent *entry)
{	
	static unsigned entnum;
	entries = realloc(entries, sizeof(struct dirent *));
	entries[entnum++] = entry;
	return entnum;
}

static int ls2(const char *path)
{
	struct stat filestat;
	DIR *dir;
	struct dirent *entry;
	unsigned int entnum = 0, i = 0;
	struct dirent **entries = NULL;
	unsigned long blocks = 0;

	if (lstat(path, &filestat) < 0)
		return -1;

	if (S_ISDIR(filestat.st_mode)) {
		print_about_dir(&filestat);
		dir = opendir(path);
		entries = pdirent_array_alloc(entries, dir);
		while ((entry = readdir(dir)) != NULL) {
			if (!is_hidden(entry->d_name)) {
				entnum = add_list_direntry(entries, entry);
				lstat(entry->d_name, &filestat);
				blocks += (filestat.st_blocks);
			}
		}
		printf("total %lu\n", blocks*512/1024);
		closedir(dir);
		//qsort
		for (i = 0; i < entnum; i++) {
			lstat(entries[i]->d_name, &filestat);
			print_about_file(entries[i]->d_name, &filestat);
		}
	} else {
		print_about_file(path, &filestat);
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
				printf("%s: cannot access %s:"
				"No such file or directory\n", \
				argv[0], argv[i]);
			}
		}
	}
	return 0;
}
