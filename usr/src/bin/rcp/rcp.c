#ifndef lint
static char sccsid[] = "@(#)rcp.c	4.15 85/04/11";
#endif

/*
 * rcp
 */
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <netinet/in.h>

#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <ctype.h>
#include <netdb.h>
#include <errno.h>

int	rem;
char	*colon(), *index(), *rindex(), *malloc(), *strcpy(), *sprintf();
int	errs;
int	lostconn();
int	iamremote;

int	errno;
char	*sys_errlist[];
int	iamremote, targetshouldbedirectory;
int	iamrecursive;
struct	passwd *pwd;
struct	passwd *getpwuid();
int	userid;
int	port;

/*VARARGS*/
int	error();

#define	ga()	 	(void) write(rem, "", 1)

main(argc, argv)
	int argc;
	char **argv;
{
	char *targ, *host, *src;
	char *suser, *tuser, *thost;
	int i;
	char buf[BUFSIZ], cmd[16];
	struct servent *sp;

	sp = getservbyname("shell", "tcp");
	if (sp == NULL) {
		fprintf(stderr, "rcp: shell/tcp: unknown service\n");
		exit(1);
	}
	port = sp->s_port;
	pwd = getpwuid(userid = getuid());
	if (pwd == 0) {
		fprintf(stderr, "who are you?\n");
		exit(1);
	}
	argc--, argv++;
	if (argc > 0 && !strcmp(*argv, "-r")) {
		iamrecursive++;
		argc--, argv++;
	}
	if (argc > 0 && !strcmp(*argv, "-d")) {
		targetshouldbedirectory = 1;
		argc--, argv++;
	}
	if (argc > 0 && !strcmp(*argv, "-f")) {
		argc--, argv++; iamremote = 1;
		(void) response();
		(void) setuid(userid);
		source(argc, argv);
		exit(errs);
	}
	if (argc > 0 && !strcmp(*argv, "-t")) {
		argc--, argv++; iamremote = 1;
		(void) setuid(userid);
		sink(argc, argv);
		exit(errs);
	}
	rem = -1;
	if (argc > 2)
		targetshouldbedirectory = 1;
	(void) sprintf(cmd, "rcp%s%s",
	    iamrecursive ? " -r" : "", targetshouldbedirectory ? " -d" : "");
	signal(SIGPIPE, lostconn);
	targ = colon(argv[argc - 1]);
	if (targ) {
		*targ++ = 0;
		if (*targ == 0)
			targ = ".";
		thost = index(argv[argc - 1], '@');
		if (thost) {
			*thost++ = 0;
			tuser = argv[argc - 1];
			if (*tuser == '\0')
				tuser = pwd->pw_name;
			else if (!okname(tuser))
				exit(1);
		} else {
			thost = argv[argc - 1];
			tuser = pwd->pw_name;
		}
		for (i = 0; i < argc - 1; i++) {
			src = colon(argv[i]);
			if (src) {
				*src++ = 0;
				if (*src == 0)
					src = ".";
				host = index(argv[i], '@');
				if (host) {
					*host++ = 0;
					suser = argv[i];
					if (*suser == '\0')
						suser = pwd->pw_name;
					else if (!okname(suser))
						continue;
		(void) sprintf(buf, "rsh %s -l %s -n %s %s '%s@%s:%s'",
					    host, suser, cmd, src,
					    tuser, thost, targ);
				} else
		(void) sprintf(buf, "rsh %s -n %s %s '%s@%s:%s'",
					    argv[i], cmd, src,
					    tuser, thost, targ);
				(void) susystem(buf);
			} else {
				if (rem == -1) {
					(void) sprintf(buf, "%s -t %s",
					    cmd, targ);
					host = thost;
					rem = rcmd(&host, port, pwd->pw_name, tuser,
					    buf, 0);
					if (rem < 0)
						exit(1);
					if (response() < 0)
						exit(1);
					(void) setuid(userid);
				}
				source(1, argv+i);
			}
		}
	} else {
		if (targetshouldbedirectory)
			verifydir(argv[argc - 1]);
		for (i = 0; i < argc - 1; i++) {
			src = colon(argv[i]);
			if (src == 0) {
				(void) sprintf(buf, "/bin/cp%s %s %s",
				    iamrecursive ? " -r" : "",
				    argv[i], argv[argc - 1]);
				(void) susystem(buf);
			} else {
				*src++ = 0;
				if (*src == 0)
					src = ".";
				host = index(argv[i], '@');
				if (host) {
					*host++ = 0;
					suser = argv[i];
					if (*suser == '\0')
						suser = pwd->pw_name;
					else if (!okname(suser))
						continue;
				} else {
					host = argv[i];
					suser = pwd->pw_name;
				}
				(void) sprintf(buf, "%s -f %s", cmd, src);
				rem = rcmd(&host, port, pwd->pw_name, suser,
				    buf, 0);
				if (rem < 0)
					continue;
				(void) setreuid(0, userid);
				sink(1, argv+argc-1);
				(void) setreuid(userid, 0);
				(void) close(rem);
				rem = -1;
			}
		}
	}
	exit(errs);
}

verifydir(cp)
	char *cp;
{
	struct stat stb;

	if (stat(cp, &stb) >= 0) {
		if ((stb.st_mode & S_IFMT) == S_IFDIR)
			return;
		errno = ENOTDIR;
	}
	error("rcp: %s: %s.\n", cp, sys_errlist[errno]);
	exit(1);
}

char *
colon(cp)
	char *cp;
{

	while (*cp) {
		if (*cp == ':')
			return (cp);
		if (*cp == '/')
			return (0);
		cp++;
	}
	return (0);
}

okname(cp0)
	char *cp0;
{
	register char *cp = cp0;
	register int c;

	do {
		c = *cp;
		if (c & 0200)
			goto bad;
		if (!isalpha(c) && !isdigit(c) && c != '_' && c != '-')
			goto bad;
		cp++;
	} while (*cp);
	return (1);
bad:
	fprintf(stderr, "rcp: invalid user name %s\n", cp0);
	return (0);
}

susystem(s)
	char *s;
{
	int status, pid, w;
	register int (*istat)(), (*qstat)();

	if ((pid = vfork()) == 0) {
		setuid(userid);
		execl("/bin/sh", "sh", "-c", s, (char *)0);
		_exit(127);
	}
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	while ((w = wait(&status)) != pid && w != -1)
		;
	if (w == -1)
		status = -1;
	signal(SIGINT, istat);
	signal(SIGQUIT, qstat);
	return (status);
}

source(argc, argv)
	int argc;
	char **argv;
{
	char *last, *name;
	struct stat stb;
	char buf[BUFSIZ];
	int x, sizerr, f;
	off_t i;

	for (x = 0; x < argc; x++) {
		name = argv[x];
		if ((f = open(name, 0)) < 0) {
			error("rcp: %s: %s\n", name, sys_errlist[errno]);
			continue;
		}
		if (fstat(f, &stb) < 0)
			goto notreg;
		switch (stb.st_mode&S_IFMT) {

		case S_IFREG:
			break;

		case S_IFDIR:
			if (iamrecursive) {
				(void) close(f);
				rsource(name, (int)stb.st_mode);
				continue;
			}
			/* fall into ... */
		default:
notreg:
			(void) close(f);
			error("rcp: %s: not a plain file\n", name);
			continue;
		}
		last = rindex(name, '/');
		if (last == 0)
			last = name;
		else
			last++;
		(void) sprintf(buf, "C%04o %D %s\n",
		    stb.st_mode&07777, stb.st_size, last);
		(void) write(rem, buf, strlen(buf));
		if (response() < 0) {
			(void) close(f);
			continue;
		}
		sizerr = 0;
		for (i = 0; i < stb.st_size; i += BUFSIZ) {
			int amt = BUFSIZ;
			if (i + amt > stb.st_size)
				amt = stb.st_size - i;
			if (sizerr == 0 && read(f, buf, amt) != amt)
				sizerr = 1;
			(void) write(rem, buf, amt);
		}
		(void) close(f);
		if (sizerr == 0)
			ga();
		else
			error("rcp: %s: file changed size\n", name);
		(void) response();
	}
}

#include <sys/dir.h>

rsource(name, mode)
	char *name;
	int mode;
{
	DIR *d = opendir(name);
	char *last;
	struct direct *dp;
	char buf[BUFSIZ];
	char *bufv[1];

	if (d == 0) {
		error("rcp: %s: %s\n", name, sys_errlist[errno]);
		return;
	}
	last = rindex(name, '/');
	if (last == 0)
		last = name;
	else
		last++;
	(void) sprintf(buf, "D%04o %d %s\n", mode&07777, 0, last);
	(void) write(rem, buf, strlen(buf));
	if (response() < 0) {
		closedir(d);
		return;
	}
	while (dp = readdir(d)) {
		if (dp->d_ino == 0)
			continue;
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		if (strlen(name) + 1 + strlen(dp->d_name) >= BUFSIZ - 1) {
			error("%s/%s: Name too long.\n", name, dp->d_name);
			continue;
		}
		(void) sprintf(buf, "%s/%s", name, dp->d_name);
		bufv[0] = buf;
		source(1, bufv);
	}
	closedir(d);
	(void) write(rem, "E\n", 2);
	(void) response();
}

response()
{
	char resp, c, rbuf[BUFSIZ], *cp = rbuf;

	if (read(rem, &resp, 1) != 1)
		lostconn();
	switch (resp) {

	case 0:
		return (0);

	default:
		*cp++ = resp;
		/* fall into... */
	case 1:
	case 2:
		do {
			if (read(rem, &c, 1) != 1)
				lostconn();
			*cp++ = c;
		} while (cp < &rbuf[BUFSIZ] && c != '\n');
		if (iamremote == 0)
			(void) write(2, rbuf, cp - rbuf);
		errs++;
		if (resp == 1)
			return (-1);
		exit(1);
	}
	/*NOTREACHED*/
}

lostconn()
{

	if (iamremote == 0)
		fprintf(stderr, "rcp: lost connection\n");
	exit(1);
}

sink(argc, argv)
	int argc;
	char **argv;
{
	char *targ;
	char cmdbuf[BUFSIZ], nambuf[BUFSIZ], buf[BUFSIZ], *cp;
	int of, mode, wrerr, exists, first;
	off_t i, size;
	char *whopp;
	struct stat stb; int targisdir = 0;
#define	SCREWUP(str)	{ whopp = str; goto screwup; }
	int mask = umask(0);
	char *myargv[1];

	umask(mask);
	if (argc != 1) {
		error("rcp: ambiguous target\n");
		exit(1);
	}
	targ = *argv;
	if (targetshouldbedirectory)
		verifydir(targ);
	ga();
	if (stat(targ, &stb) == 0 && (stb.st_mode & S_IFMT) == S_IFDIR)
		targisdir = 1;
	for (first = 1; ; first = 0) {
		cp = cmdbuf;
		if (read(rem, cp, 1) <= 0)
			return;
		if (*cp++ == '\n')
			SCREWUP("unexpected '\\n'");
		do {
			if (read(rem, cp, 1) != 1)
				SCREWUP("lost connection");
		} while (*cp++ != '\n');
		*cp = 0;
		if (cmdbuf[0] == '\01' || cmdbuf[0] == '\02') {
			if (iamremote == 0)
				(void) write(2, cmdbuf+1, strlen(cmdbuf+1));
			if (cmdbuf[0] == '\02')
				exit(1);
			errs++;
			continue;
		}
		*--cp = 0;
		cp = cmdbuf;
		if (*cp == 'E') {
			ga();
			return;
		}
		if (*cp != 'C' && *cp != 'D') {
			/*
			 * Check for the case "rcp remote:foo\* local:bar".
			 * In this case, the line "No match." can be returned
			 * by the shell before the rcp command on the remote is
			 * executed so the ^Aerror_message convention isn't
			 * followed.
			 */
			if (first) {
				error("%s\n", cp);
				exit(1);
			}
			SCREWUP("expected control record");
		}
		cp++;
		mode = 0;
		for (; cp < cmdbuf+5; cp++) {
			if (*cp < '0' || *cp > '7')
				SCREWUP("bad mode");
			mode = (mode << 3) | (*cp - '0');
		}
		if (*cp++ != ' ')
			SCREWUP("mode not delimited");
		size = 0;
		while (*cp >= '0' && *cp <= '9')
			size = size * 10 + (*cp++ - '0');
		if (*cp++ != ' ')
			SCREWUP("size not delimited");
		if (targisdir)
			(void) sprintf(nambuf, "%s%s%s", targ,
			    *targ ? "/" : "", cp);
		else
			(void) strcpy(nambuf, targ);
		exists = stat(nambuf, &stb) == 0;
		if (cmdbuf[0] == 'D') {
			if (exists) {
				if ((stb.st_mode&S_IFMT) != S_IFDIR) {
					errno = ENOTDIR;
					goto bad;
				}
			} else if (mkdir(nambuf, mode) < 0)
				goto bad;
			myargv[0] = nambuf;
			sink(1, myargv);
			continue;
		}
		if ((of = creat(nambuf, mode)) < 0) {
	bad:
			error("rcp: %s: %s\n", nambuf, sys_errlist[errno]);
			continue;
		}
		if (exists)
			(void) fchmod(of, mode &~ mask);
		ga();
		wrerr = 0;
		for (i = 0; i < size; i += BUFSIZ) {
			int amt = BUFSIZ;
			char *cp = buf;

			if (i + amt > size)
				amt = size - i;
			do {
				int j = read(rem, cp, amt);

				if (j <= 0)
					exit(1);
				amt -= j;
				cp += j;
			} while (amt > 0);
			amt = BUFSIZ;
			if (i + amt > size)
				amt = size - i;
			if (wrerr == 0 && write(of, buf, amt) != amt)
				wrerr++;
		}
		(void) close(of);
		(void) response();
		if (wrerr)
			error("rcp: %s: %s\n", cp, sys_errlist[errno]);
		else
			ga();
	}
screwup:
	error("rcp: protocol screwup: %s\n", whopp);
	exit(1);
}

/*VARARGS*/
error(fmt, a1, a2, a3, a4, a5)
	char *fmt;
	int a1, a2, a3, a4, a5;
{
	char buf[BUFSIZ], *cp = buf;

	errs++;
	*cp++ = 1;
	(void) sprintf(cp, fmt, a1, a2, a3, a4, a5);
	(void) write(rem, buf, strlen(buf));
	if (iamremote == 0)
		(void) write(2, buf+1, strlen(buf+1));
}
