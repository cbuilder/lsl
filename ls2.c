#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

static unsigned max_links;
static unsigned max_user;
static unsigned max_group;
static unsigned max_size;
const char month[12][11] = {"янв. ", "фев. ", "марта", "апр. ", "мая  ", "июня ", "июля ", "авг. ", "сент.", "окт. ", "нояб.", "дек. "};

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
		modestr[0] = 's';
		break;
	default:
		break;
	}
	printf("%s", modestr);
}

static void p_links(struct stat *s)
{
	char buf[256] = "";
	int l = sprintf(buf, "%lu", s->st_nlink);
	printf("%*c%s" , max_links - l + 1, ' ', buf);
}

static void p_usrown(struct stat *s)
{
	char buf[256] = "";
	struct passwd *pwd = getpwuid(s->st_uid);
	int l = sprintf(buf, "%s", pwd->pw_name);
	printf(" %s%*c" , buf, max_user - l + 1, ' ');
}

static void p_grpown(struct stat *s)
{
	char buf[256] = "";
	struct group *grp = getgrgid(s->st_gid);
	int l = sprintf(buf, "%s", grp->gr_name);
	printf("%s%*c" , buf, max_group - l + 1, ' ');
}

static void p_size(struct stat *s)
{
	char buf[256] = "";
	int l = sprintf(buf, "%lu", s->st_size);
	printf("%*c%s" , max_size - l + 1, ' ', buf);
}

static void p_ls_time(time_t *mtime)
{
	time_t tt;
	tt = time(&tt);
	struct tm *lt = localtime(mtime);
	struct tm now;
	localtime_r(&tt, &now);
	printf(" %s %2d ", month[lt->tm_mon], lt->tm_mday);
	if (now.tm_year == lt->tm_year)
		printf("%02d:%02d", lt->tm_hour, lt->tm_min);
	else
		printf(" %u", lt->tm_year + 1900);
}

static void print_about_file(int dfd, const char* name, struct stat *s)
{
	p_rights(s);
	p_links(s);
	p_usrown(s);
	p_grpown(s);
	p_size(s);
	p_ls_time(&s->st_mtime);
	printf(" %s", name);
	if (S_ISLNK(s->st_mode)) {
		char buf[256] = "";
		readlinkat(dfd, name, buf, 255);
		printf(" -> %s\n", buf);
	} else {
		printf("\n");
	}
}

void count_columns(struct dirent *entry, struct stat *fs)
{
	size_t l;
	char buf[256] = "";
	l = sprintf(buf, "%lu", fs->st_nlink);
	if (l > max_links)
		max_links = l;
	if (strlen(getpwuid(fs->st_uid)->pw_name) > max_user)
		max_user = strlen(getpwuid(fs->st_uid)->pw_name);
	if (strlen(getgrgid(fs->st_gid)->gr_name) > max_group)
		max_group = strlen(getgrgid(fs->st_gid)->gr_name);
	l = sprintf(buf, "%lu", fs->st_size);
	if (l > max_size)
		max_size = l;
}

static int ls2(const char *path)
{
	DIR *dir;
	struct dirent *entry;
	struct stat fs;
	unsigned long blocks = 0;
	int dfd;

	if (lstat(path, &fs) < 0)
		return -1;

	if (S_ISDIR(fs.st_mode)) {
		dir = opendir(path);
		dfd = dirfd(dir);
		while ((entry = readdir(dir)) != NULL) {
			if (!is_hidden(entry->d_name)) {
				fstatat(dfd, entry->d_name, &fs, AT_SYMLINK_NOFOLLOW);
				blocks += (fs.st_blocks);
				count_columns(entry, &fs);
			}
		}
		printf("total %lu\n", blocks*512/1024);
		rewinddir(dir);
		while ((entry = readdir(dir)) != NULL) {
			if (!is_hidden(entry->d_name)) {
				fstatat(dfd, entry->d_name, &fs, AT_SYMLINK_NOFOLLOW);
				print_about_file(dfd, entry->d_name, &fs);
			}
		}
		closedir(dir);
	} else {
		fstatat(dfd, entry->d_name, &fs, AT_SYMLINK_NOFOLLOW);
		count_columns(entry, &fs);
		print_about_file(dfd, path, &fs);
	}
	return 0;
}

int main(int argc, char **argv)
{
	int i;
	if (argc == 1) {
		ls2(".");
	} else {
		for (i = argc-1; i > 0; i--)
			if (-1 == ls2(argv[i]))
				printf("%s: cannot access %s:"
				"No such file or directory\n", \
				argv[0], argv[i]);
	}
	return 0;
}
