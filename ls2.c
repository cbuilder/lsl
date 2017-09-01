#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

static int is_hidden(char *name)
{
	return name[0] == '.' ? 1 : 0;
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
	struct passwd *pwd = getpwuid(s->st_uid);
	printf("%s ", pwd->pw_name);
}

static void p_grpown(struct stat *s)
{
	struct group *grp = getgrgid(s->st_gid);
	printf("%s ", grp->gr_name);
}

static void p_size(struct stat *s)
{
	printf("%lu ", s->st_size);
}

static void p_timestamp(struct stat *s)
{
	//struct tm *lt = localtime(&s->st_mtime);
	//printf(" %s", asctime(lt));
	printf(" %lu", s->st_mtime);
}

static void print_about_file(const char* name, struct stat *s)
{
	p_rights(s);
	p_links(s);
	p_usrown(s);
	p_grpown(s);
	p_size(s);
	p_timestamp(s);
	printf(" %s\n", name);
}

static unsigned max_links;
static unsigned max_user;
static unsigned max_group;
static unsigned max_size;
static unsigned max_date;

static int ls2(const char *path)
{
	DIR *dir;
	struct dirent *entry;
	struct stat fs;
	unsigned long blocks = 0;

	if (lstat(path, &fs) < 0)
		return -1;

	if (S_ISDIR(fs.st_mode)) {
		dir = opendir(path);
		while ((entry = readdir(dir)) != NULL) {
			if (!is_hidden(entry->d_name)) {
				lstat(entry->d_name, &fs);
				blocks += (fs.st_blocks);
				if (fs.st_nlink > max_links)
					max_links = fs.st_nlink;
				if (strlen(getpwuid(fs.st_uid)->pw_name) > max_user)
					max_user = strlen(getpwuid(fs.st_uid)->pw_name);
				if (strlen(getgrgid(fs.st_gid)->gr_name) > max_group)
					max_group = strlen(getgrgid(fs.st_gid)->gr_name);
			}
		}
		printf("total %lu\n", blocks*512/1024);
		printf("max_links = %u, max_user = %u, max_group = %u\n", max_links, max_user, max_group);
		rewinddir(dir);
		while ((entry = readdir(dir)) != NULL) {
			if (!is_hidden(entry->d_name)) {
				lstat(entry->d_name, &fs);
				print_about_file(entry->d_name, &fs);
			}
		}
		closedir(dir);
	} else {
		print_about_file(path, &fs);
	}
	return 0;
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
