/*
 * $Id$
 *
 * sudosh - sudo shell that supports input and output logging to syslog
 *
 * Copyright 2004 and $Date$
 *
 * Licensed under the Open Software License version 2.0
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the Open Software
 * License, version 2.0 by Lauwrence E. Rosen.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.	See the Open Software License for details.
 */

#include "super.h"
#include "struct.h"
#include "getopt.h"

#ifndef SIGCHLD
#define SIGCHLD	SIGCLD
#endif

#define WRITE(a, b, c) do_write(a, b, c, __FILE__, __LINE__)

typedef enum {false=0, true=1} bool;

static struct termios termorig;
static struct winsize winorig;

struct pst
{
	char *master;
	char *slave;
	int mfd;
	int sfd;
} pspair;

struct s_file
{
	int fd;
	int bytes;
	struct stat stat;
	struct stat cstat;
	struct stat tstat;
	char name[BUFSIZ];
	char str[BUFSIZ];
};

struct s_file script;
struct s_file input;
struct s_file timing;

struct s_env
{
	char str[BUFSIZ];
	char *ptr;
};

struct s_user
{
	char to[BUFSIZ];
	char from[BUFSIZ];
	char *vshell;
	struct s_env home;
	struct s_env to_home;
	struct s_env term;
	struct s_env path;
	struct s_env mail;
	struct s_env shell;
	struct s_env logname;
	struct passwd *pw;
};

struct s_user user;
struct s_option sudosh_option;

static char *progname;
char start_msg[BUFSIZ];

int loginshell = 0;

double oldtime;


static void bye (int);
static void newwinsize (int);
static void prepchild (struct pst *, char ttyFather[BUFSIZ]);
static void rawmode (int);
static int findms (struct pst *);
void mysyslog (int, const char *, ...);
char *rand2str (size_t len);
int do_write (int, void *, size_t, char *, unsigned int);

static int process(bool blockSelect);
static void processAndBye (int);

extern void parse (option *, const char *);

extern int unlockpt (int);
extern int grantpt (int);
extern char *ptsname (int);

extern char *optarg;
extern int optind;

int main (int argc, char *argv[], char *environ[])
{
	int result = EXIT_SUCCESS;
//	int n = 1;
	int valid = -1;
	//int found = FALSE;
//	char iobuf[BUFSIZ];
	char sysconfdir[BUFSIZ];
	char c_str[BUFSIZ];
	char c_command[BUFSIZ];
	char *p = NULL;
	char *c_args = NULL;
	char *rand = rand2str (16);
	time_t now = time ((time_t *) NULL);
	struct stat s;
	struct sigaction saterm;
	struct sigaction sawinch;
	struct sigaction sachild;
/*
 *	struct timeval tv;
 *	double oldtime
 *	double newtime;
 */
	struct stat ttybuf;
	int c;

	user.vshell = NULL;
	user.shell.ptr = NULL;
	user.home.ptr = NULL;
	user.term.ptr = NULL;

	progname = argv[0];

	if ((p = (char *) strrchr (progname, '/')) != NULL)
		progname = p + 1;

	if (*progname == '-')
		loginshell = 1;

	/* Who are you? */
	user.pw = getpwuid ((uid_t) geteuid ());

	if (user.pw == NULL) {
		fprintf (stderr, "I do not know who you are.	Stopping.\n");
		perror ("getpwuid");
		exit (EXIT_FAILURE);
	}

	strncpy (user.to, user.pw->pw_name, BUFSIZ - 1);

	user.term.ptr = getenv ("TERM");

	if (user.term.ptr == NULL)
		user.term.ptr = "dumb";

	if (strlen (user.term.ptr) < 1)
		user.term.ptr = "dumb";

	snprintf(sysconfdir, BUFSIZ - 1, "%s/sudosh.conf", SYSCONFDIR);
	parse (&sudosh_option, sysconfdir);

	while ((c = getopt(argc, argv, "c:hivV")) != EOF) {

		switch (c) {
			case 'c':
			//	fprintf(stderr,"optarg is [%s]\n",optarg);
				strncpy (user.from, user.pw->pw_name, BUFSIZ - 1);
				strncpy (c_str, optarg, BUFSIZ - 1);
				strncpy (c_command, optarg, BUFSIZ -1);
				c_args = (char *) strchr (optarg, ' ');
				p=strchr(c_str, ' ');

				if (p) {
					p[0]=0;
				//	fprintf(stderr,"args=%s\n",c_args);
				}

				//if (c_str) {
				//	fprintf(stderr,"Testing c\n");
					// Make sure that c_str is in argallow
					char argtest[BUFSIZ];

					sprintf(argtest,"$%.100s$",c_str);
				//	fprintf(stderr,"Testing for %s\n",argtest);

					if (strstr(sudosh_option.argallow,argtest)!=NULL) {

						FILE *f;
						snprintf (script.name, (size_t) BUFSIZ - 1,
								"%s/%s%c%s%cinteractive%c%i%c%s",
								sudosh_option.logdir, user.from,
								sudosh_option.fdl, user.to, sudosh_option.fdl,
								sudosh_option.fdl, (int) now, sudosh_option.fdl,
								rand);

						f = fopen (script.name, "w");

						if (f == (FILE *) 0) {
							fprintf (stderr, "%.100s: %.100s (%i)\n", script.name,
									 strerror (errno), errno);
							exit (EXIT_FAILURE);
						}

						fprintf (f, "%.256s\n", c_str);
						fclose (f);

						execl ("/bin/sh", "sh", "-c", c_command,	(char *) 0);
						exit (EXIT_SUCCESS);
						break;
					}
					else {
						fprintf (stderr, "\"%s\" isn't allowed to be executed.\n",
								 c_str);
						exit (EXIT_FAILURE);
						break;
					}
				//}
				break;
			case 'h':
			case '?':
				fprintf (stdout,
					 "Usage: sudosh\n"
					 "sudo shell that supports input and output logging to syslog\n"
					 "\n"
					 "-h, --help	display this help and exit\n"
					 "-i, --init	initialize logdir (mkdir and chmod) (ignored for compatibility)\n"
					 "-v, --version	output version information and exit\n"
					 "\n" "Report bugs to <%s>\n", PACKAGE_BUGREPORT);
				exit (EXIT_SUCCESS);
				break;
			case 'i':
				fprintf(stdout,"Ignoring initialize option, this is done automatically\n");
				exit (EXIT_SUCCESS);
				break;
			case 'v':
			case 'V':
				fprintf (stdout, "%s version %s\n", PACKAGE_NAME, VERSION);
				exit (EXIT_SUCCESS);
				break;
			default:
				fputs ("Try `sudosh -h' for more information.\n", stderr);
				exit (EXIT_FAILURE);
				break;
		}
	}

	if (ttyname (0) != NULL) {

		if (stat (ttyname (0), &ttybuf) == 0) {

			if ((getpwuid (ttybuf.st_uid)->pw_name) == NULL) {
				fprintf (stderr, "I have no idea who you are.\n");
				exit (EXIT_FAILURE);
			}
			strncpy (user.from, getpwuid (ttybuf.st_uid)->pw_name, BUFSIZ - 1);
		}
		else {
			fprintf (stderr, "Couldn't stat %s\n", ttyname (0));
			exit (EXIT_FAILURE);
		}
	}
	else {
		fprintf(stderr, "%s: couldn't get your controlling terminal.\n", progname);
		exit(EXIT_FAILURE);
	}

	user.pw = getpwuid ((uid_t) geteuid ());

	snprintf (user.home.str, BUFSIZ - 1, "HOME=%s", user.pw->pw_dir);
	strncpy (user.to_home.str, user.pw->pw_dir, BUFSIZ - 1);
	snprintf (user.term.str, BUFSIZ - 1, "TERM=%s", user.term.ptr);


#ifdef HAVE_GETUSERSHELL
	if ((user.shell.ptr = getenv ("SHELL")) == NULL)
		user.shell.ptr = user.pw->pw_shell;

	/* check against /etc/shells to make sure it's a real shell */
	setusershell ();

	while ((user.vshell = (char *) getusershell ()) != (char *) 0) {
		if (strcmp (user.shell.ptr, user.vshell) == 0)
			valid = 1;
	}

	endusershell ();

	if (valid != 1) {

		if (user.shell.ptr == NULL) {

			fprintf (stderr, "Could not determine a valid shell.\n");

			if (sudosh_option.priority!=-1)
				mysyslog (sudosh_option.priority, "Could not determine a valid shell");

			exit (EXIT_FAILURE);
		}
		else {
			fprintf (stderr, "%s is not in /etc/shells\n", user.shell.ptr);
			mysyslog (sudosh_option.priority,
					"%s,%s: %s is not in /etc/shells", user.from,
					ttyname (0), user.shell.ptr);
			exit (EXIT_FAILURE);
		}
	}

	if (stat ((const char *) user.shell.ptr, &s) == -1) {

		fprintf (stderr, "Shell %s doesn't exist.\n", user.shell.ptr);

		if (sudosh_option.priority!=-1)
			mysyslog (sudosh_option.priority, "%s,%s: shell %s doesn't exist.",
					user.from, ttyname (0), user.shell.ptr);
		exit (EXIT_FAILURE);
	}
#else
	user.shell.ptr = user.pw->pw_shell;
#endif /* HAVE_GETUSERSHELL */

	if (loginshell)
		user.shell.ptr = sudosh_option.defshell;

	script.bytes = 0;
	timing.bytes = 0;
	input.bytes = 0;

	snprintf (script.name, (size_t) BUFSIZ - 1, "%s/%s%c%s%cscript%c%i%c%s",
			sudosh_option.logdir, user.from, sudosh_option.fdl, user.to,
			sudosh_option.fdl, sudosh_option.fdl, (int) now,
			sudosh_option.fdl, rand);
	snprintf (timing.name, (size_t) BUFSIZ - 1, "%s/%s%c%s%ctime%c%i%c%s",
			sudosh_option.logdir, user.from, sudosh_option.fdl, user.to,
			sudosh_option.fdl, sudosh_option.fdl, (int) now,
			sudosh_option.fdl, rand);
	snprintf (input.name, (size_t) BUFSIZ - 1, "%s/%s%c%s%cinput%c%i%c%s",
			sudosh_option.logdir, user.from, sudosh_option.fdl, user.to,
			sudosh_option.fdl, sudosh_option.fdl, (int) now,
			sudosh_option.fdl, rand);

	snprintf (start_msg, BUFSIZ - 1,
			"starting session for %s as %s, tty %s, shell %s", user.from, user.to,
			ttyname (0), user.shell.ptr);

	char ttyFather[BUFSIZ];
	snprintf (ttyFather, strlen(ttyname(0))+12,	"TTY_SUDOSH=%s", ttyname(0));

	if ((script.fd = open (script.name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) == -1) {
		perror (script.name);
		bye (EXIT_FAILURE);
	}

	if (fstat (script.fd, &script.stat) == -1) {
		perror ("fstat script.fd");
		exit (EXIT_FAILURE);
	}

	if ((timing.fd = open (timing.name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) == -1) {
		perror (timing.name);
		bye (EXIT_FAILURE);
	}

	if (fstat (timing.fd, &timing.stat) == -1) {
		perror ("fstat timing.fd");
		exit (EXIT_FAILURE);
	}

	if ((input.fd = open (input.name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) == -1) {
		perror (input.name);
		bye (EXIT_FAILURE);
	}

	if (fstat (input.fd, &input.stat) == -1) {
		perror ("fstat input.fd");
		exit (EXIT_FAILURE);
	}

	if (sudosh_option.priority!=-1)
		mysyslog (sudosh_option.priority, start_msg);

	rawmode (0);

	if (findms (&pspair) < 0) {
		perror ("open pty failed");
		bye (EXIT_FAILURE);
	}

	switch (fork ()) {
		case 0:
			close (pspair.mfd);
			prepchild (&pspair, ttyFather);
		case -1:
			perror ("fork failed");
			bye (EXIT_FAILURE);
		default:
			close (pspair.sfd);
	}

	setuid (getuid ());

	memset (&sawinch, 0, sizeof sawinch);
	sawinch.sa_handler = newwinsize;
	sawinch.sa_flags = SA_RESTART;
	sigaction (SIGWINCH, &sawinch, (struct sigaction *) 0);

	memset (&saterm, 0, sizeof saterm);
/*
 * If there are some info to write,
 * we have to write them all
 */
	saterm.sa_handler = processAndBye;
//  saterm.sa_handler = bye;
	sigaction (SIGTERM, &sawinch, (struct sigaction *) 0);

	memset (&sachild, 0, sizeof sachild);
/*
 * If there are some info to write,
 * we have to write them all
 */
	sachild.sa_handler = processAndBye;
//  sachild.sa_handler = bye;
	sigaction (SIGCHLD, &sachild, (struct sigaction *) 0);

	oldtime = time (NULL);

	result = process(true);
	bye (result);

	return (0);
}

static int findms (struct pst *p)
{
	char *sname;

	if ((p->mfd = open ("/dev/ptmx", O_RDWR)) == -1) {

		if ((p->mfd = open ("/dev/ptc", O_RDWR)) == -1) {
			perror ("Cannot open cloning master pty");
			return -1;
		}
	}

	(void) unlockpt (p->mfd);
	(void) grantpt (p->mfd);

	sname = (char *) ptsname (p->mfd);

	if ((p->sfd = open (sname, O_RDWR)) == -1) {
		perror ("open slave pty");
		close (p->mfd);
		return -1;
	}

	p->master = (char *) 0;

	p->slave = malloc (strlen (sname) + 1);
	strcpy (p->slave, sname);

#ifdef I_LIST
	if (ioctl (p->sfd, I_LIST, NULL) > 0) {
#ifdef I_FIND
		if (ioctl (p->sfd, I_FIND, "ldterm") != 1 && ioctl (p->sfd, I_FIND, "ldtty") != 1) {
#ifdef I_PUSH
			(void) ioctl (p->sfd, I_PUSH, "ptem");
			(void) ioctl (p->sfd, I_PUSH, "ldterm");
#endif
		}
#endif
	}
#endif

	return p->mfd;
}

static void prepchild (struct pst *pst, char ttyFather[BUFSIZ])
{
	int i;
	char *b = NULL;
	char newargv[BUFSIZ];
	char *env_list[] = { user.term.str
					   , user.home.str
					   , user.shell.str
					   , user.logname.str
					   , user.path.str
					   , ttyFather
					   , NULL
					   };

	close (0);
	close (1);
	close (2);

	setsid ();
#ifdef TIOCSCTTY
	(void) ioctl (pst->sfd, TIOCSCTTY, 0);
#endif

	if ((pst->sfd = open (pst->slave, O_RDWR)) < 0)
		exit (EXIT_FAILURE);

	i=dup (0);
	i=dup (0);

	for (i = 3; i < 100; ++i)
		close (i);

#ifdef TCSETS
	(void) ioctl (0, TCSETS, &termorig);
#endif
	(void) ioctl (0, TIOCSWINSZ, &winorig);

	setuid (getuid ());

	strncpy (newargv, user.shell.ptr, BUFSIZ - 1);

	if ((b = strrchr (newargv, '/')) == NULL)
		b = newargv;

	*b = '-';

	snprintf (user.shell.str, BUFSIZ - 1, "SHELL=%s", user.shell.ptr);
	snprintf (user.logname.str, BUFSIZ - 1, "LOGNAME=%s", user.to);

	if (!strcmp (user.to, "root"))
		snprintf (user.path.str, BUFSIZ - 1, "PATH=/sbin:/bin:/usr/sbin:/usr/bin:");
	else
		snprintf (user.path.str, BUFSIZ - 1, "PATH=/usr/bin:/bin:/usr/local/bin:");

#ifdef HAVE_SETPENV
	/* I love AIX - setpenv takes care of everything, including chdir() and the env */
	setpenv (user.to, PENV_INIT, (char **) 0, (char *) 0);
#endif
	if (chdir (user.to_home.str) == -1) {
		fprintf (stderr, "Unable to chdir to %s, using /tmp instead.\n",
				 user.pw->pw_dir);
		if (chdir ("/tmp") == -1)
			fprintf (stderr, "Unable to chdir to /tmp\n");
	}

	if (sudosh_option.clearenvironment==0)
		execl (user.shell.ptr, b, (char *) 0);
	else
		execle (user.shell.ptr, b, (char *) 0, env_list);

	abort ();
}

static int process(bool blockSelect)
{
	int n      = 0;
	int nInput = 0;
	int nInputTmp = 0;
	int i = 0;
	double newtime;
	struct timeval tv;
	struct timeval* timeOut = NULL;
	char ibuf[BUFSIZ];
	char iobuf[BUFSIZ];
	int result = EXIT_SUCCESS;

	if ( ! blockSelect) {
		timeOut = malloc(sizeof(struct timeval));
		timeOut->tv_sec  = 0;
		timeOut->tv_usec = 0;
	}

	do {
		fd_set readfds;

		FD_ZERO (&readfds);
		FD_SET (pspair.mfd, &readfds);
		FD_SET (0, &readfds);

		if (select (pspair.mfd + 1, &readfds, (fd_set*) 0, (fd_set*) 0, timeOut) < 0) {

			if (errno == EINTR) {
				continue;
			}

			perror ("select");
			result = EXIT_FAILURE;
			break;
		}

		if (FD_ISSET (pspair.mfd, &readfds))  {

			if ((n = read (pspair.mfd, iobuf, sizeof (iobuf))) > 0) {

				gettimeofday ((struct timeval *) &tv, NULL);

				newtime = tv.tv_sec + (double) tv.tv_usec / 1000000;

				WRITE (1, &iobuf, n);

				script.bytes += WRITE (script.fd, &iobuf, n);

				if (nInput > 0) {

					switch (iobuf[0]) {
						case '\r':
						case '\n':

							nInputTmp = 0;

							for (i = 0; i < nInput; i++) {

								if ( ! isprint(ibuf[i]) || isspace(ibuf[i])) {

									input.bytes += WRITE (input.fd, &ibuf[i], 1);
									nInputTmp++;
								}
							}

							nInput = nInputTmp;

							break;
						default:
							input.bytes += WRITE (input.fd, &ibuf, nInput);
							break;
					}
				}

				snprintf (timing.str, BUFSIZ - 1, "%f %i %i %i %i\n", newtime - oldtime, n, nInput,winorig.ws_col,winorig.ws_row);

				timing.bytes += WRITE (timing.fd, &timing.str, strlen (timing.str));

				nInput = 0;

				oldtime = newtime;
			}
		}

		if (FD_ISSET (0, &readfds)) {

			if ((n = read (0, ibuf, sizeof (ibuf))) > 0) {

				WRITE (pspair.mfd, &ibuf, n);

				nInput = n;
			}
		}
	}
	while (n > 0);

	if (timeOut != NULL) {
		free (timeOut);
		timeOut = NULL;
	}

	return result;
}

static void processAndBye (int signum)
{
	process(false);
	bye(signum);
}

static void rawmode (int ttyfd)
{
	static struct termios termnew;

#ifdef TCGETS
	if (ioctl (ttyfd, TCGETS, &termorig) == -1) {
		perror ("ioctl TCGETS failed");
		exit (EXIT_FAILURE);
	}
#endif

	if (ioctl (ttyfd, TIOCGWINSZ, &winorig) == -1) {
	//	perror ("ioctl TIOCGWINSZ failed");
	//	exit (EXIT_FAILURE);
		winorig.ws_row = 24;
		winorig.ws_col = 80;
	}

	termnew.c_cc[VEOF] = 1;
	termnew.c_iflag = BRKINT | ISTRIP | IXON | IXANY;
	termnew.c_oflag = 0;
	termnew.c_cflag = termorig.c_cflag;
	termnew.c_lflag &= ~ECHO;

#ifdef TCSETS
	(void) ioctl (ttyfd, TCSETS, &termnew);
#endif
}

static void bye (int signum)
{
#ifdef TCSETS
	(void) ioctl (0, TCSETS, &termorig);
#endif

	close (timing.fd);
	close (script.fd);
	close (input.fd);

	if (sudosh_option.priority!=-1)
		mysyslog (sudosh_option.priority, "stopping session for %s as %s, tty %s, shell %s",
			user.from, user.to, ttyname(0), user.shell.ptr);
	exit (signum);
}

static void newwinsize (int signum)
{
	int fd;

	if (ioctl (0, TIOCGWINSZ, &winorig) != -1) {

		if ((fd = open (pspair.slave, O_RDWR)) >= 0) {

			(void) ioctl (fd, TIOCSWINSZ, &winorig);
			close (fd);
		}
	}
}

void mysyslog (int pri, const char *fmt, ...)
{
	va_list ap;
	char buf[BUFSIZ + 1];

	va_start (ap, fmt);
	vsnprintf (buf, sizeof (buf), fmt, ap);

	openlog (progname, 0, sudosh_option.facility);
	syslog (pri, "%s", buf);
	closelog ();
}


int do_write (int fd, void *buf, size_t size, char *file, unsigned int line)
{
	char str[BUFSIZ];
	int s;

	if (fd < 0)
		return -1;

	if ((s = write (fd, buf, size)) < 0) {

		snprintf (str, BUFSIZ - 1, "%s [%s, line %i]: %s\n", progname, file, line, strerror (errno));
		perror (str);
		exit (EXIT_FAILURE);
	}

	return s;
}