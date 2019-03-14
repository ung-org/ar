#define _XOPEN_SOURCE 700
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

enum {
	CLOBBER		= 1<<0,
	CREATE_SILENTLY	= 1<<1,
	SYMBOLS		= 1<<2,
	TRUNCATE	= 1<<3,
	UPDATE		= 1<<4,
	VERBOSE		= 1<<5,

	POSBEFORE	= 1<<6,
	POSAFTER	= 1<<7,
};

static int ar_delete(int archive, const char *file, const char *posname, int flags)
{
	(void)posname;

	if (flags & VERBOSE) {
		printf("d - %s\n", file);
	}

	return 0;
}

static int ar_move(int archive, const char *file, const char *posname, int flags)
{
	if (flags & VERBOSE) {
		printf("moving %s %s %s\n", file, flags & POSBEFORE ? "before" : "after", posname);
	}

	return 0;
}

static int ar_print(int archive, const char *file, const char *posname, int flags)
{
	(void)posname;

	if (flags & VERBOSE) {
		printf("\n<%s>\n\n", file);
	}

	return 0;
}

static int ar_quick(int archive, const char *file, const char *posname, int flags)
{
	(void)posname;

	if (flags & VERBOSE) {
		printf("quick adding %s\n", file);
	}

	return 0;
}

static int ar_replace(int archive, const char *file, const char *posname, int flags)
{
	if (flags & VERBOSE) {
		printf("%c - %s\n", /* file exists ? 'r' : */ 'a', file);
	}

	return 0;
}

static int ar_toc(int archive, const char *file, const char *posname, int flags)
{
	(void)posname;
	char *mode = "----------";
	unsigned int uid = 0;
	unsigned int gid = 0;
	unsigned int size = 0;
	char *month = "Jan";	/* %b */
	int day = 0;		/* %e */
	int hour = 0;		/* %H */
	int minute = 0;		/* %M */
	int year = 0;		/* %Y */

	if (flags & VERBOSE) {
		printf("%s %u/%u %u %s %d %d:%d %d %s\n", mode, uid, gid, size, month, day, hour, minute, year, file);
	} else {
		printf("%s\n", file);
	}

	return 0;
}

static int ar_extract(int archive, const char *file, const char *posname, int flags)
{
	(void)posname;

	if (flags & VERBOSE) {
		printf("x - %s\n", file);
	}

	return 0;
}

static int ar_usage(int archive, const char *file, const char *posname, int flags)
{
	(void)archive; (void)file; (void)posname; (void)flags;
	printf("usage:\n");
	printf("  ar -d [-v] archive file...\n");
	printf("  ar -m [-v] archive file...\n");
	printf("  ar -m (-a|-b|-i) [-v] posname archive file...\n");
	printf("  ar -p [-vs] archive [file...]\n");
	printf("  ar -q [-cv] archive file...\n");
	printf("  ar -r [-cuv] archive file...\n");
	printf("  ar -r (-a|-b|-i) [-cuv] posname archive file...\n");
	printf("  ar -t [-vs] archive [file...]\n");
	printf("  ar -x [-vsCT] archive [file...]\n");
	return 1;
}

int main(int argc, char *argv[])
{
	int (*fn)(int, const char *, const char*, int);
	char *posname = NULL;
	char *archive = NULL;
	int flags = CLOBBER;
	int open_flags = 0;
	int c;

	while ((c = getopt(argc, argv, "abcCdimpqrstTuvx")) != -1) {
		switch (c) {
		case 'a':
			flags |= POSAFTER;
			flags &= ~POSBEFORE;
			break;

		case 'b':
		case 'i':
			flags |= POSBEFORE;
			flags &= ~POSAFTER;
			break;

		case 'c':
			flags |= CREATE_SILENTLY;
			break;

		case 'C':
			flags &= ~CLOBBER;
			break;

		case 'd':
			fn = ar_delete;
			open_flags = O_RDWR;
			break;

		case 'm':
			fn = ar_move;
			open_flags = O_RDWR | O_CREAT;
			break;

		case 'p':
			fn = ar_print;
			open_flags = O_RDONLY;
			break;

		case 'q':
			fn = ar_quick;
			open_flags = O_RDWR | O_CREAT;
			break;

		case 'r':
			fn = ar_replace;
			open_flags = O_RDWR | O_CREAT;
			break;

		case 's':
			flags |= SYMBOLS;
			break;

		case 't':
			fn = ar_toc;
			open_flags = O_RDONLY;
			break;

		case 'T':
			flags |= TRUNCATE;
			break;

		case 'u':
			flags |= UPDATE;
			break;

		case 'v':
			flags |= VERBOSE;
			break;

		case 'x':
			fn = ar_extract;
			open_flags = O_RDONLY;
			break;

		default:
			return 1;
		}
	}

	if (flags & (POSBEFORE | POSAFTER)) {
		posname = argv[optind++];
	}

	archive = argv[optind++];

	int fd = open(archive, open_flags, 0644);
	if (fd == -1) {
		perror("ar");
		return 1;
	}

	if (open_flags & O_CREAT) {
		if (! (flags & CREATE_SILENTLY)) {
			fprintf(stderr, "ar: creating %s\n", archive);
		}
	}

	int ret = 0;
	do {
		ret |= fn(fd, argv[optind++], posname, flags);
	} while (argv[optind]);

	return 0;
}
