/*
 * $Id$
 *
 * Replay session recorded by sudosh
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
#include <time.h>
#include "super.h"
#include "struct.h"

extern void write2output(const char *oBuffer, size_t oCharCount);

#define LL() fprintf(stderr, "[%s, line %i]: ", __FILE__, __LINE__)

struct s_file {
	FILE *f;
	int fd;
	int line;
	char *ptr;
	char str[BUFSIZ];
};

struct s_session {
	char to[BUFSIZ];
	char from[BUFSIZ];
	char type[BUFSIZ];
	char randstr[BUFSIZ];
	char date[BUFSIZ];
	char id[BUFSIZ];
	long secs;
	time_t e;
	struct s_session *next;
	struct s_session *prev;
	struct s_file input;
	struct s_file script;
	struct s_file time;
};

typedef struct s_session session;

extern void parse(option *, const char *);

int getopt(int, char *const[], const char *);
extern char *optarg;
extern int optind;

char match[BUFSIZ];

char *progname;
session *s_top;
option config_option;

void replay(const char *, const char *, double, double,int);
void link_session(session *);
void unlink_session(session *);
void count_dur(session *);
char *int2smalltime(int);

session *session_malloc(void);
session *find_session(const char *, const char *, time_t e, const char *);
session *sort_list(session *);
void show_sessions(void);

int main(int argc, char **argv, char **environ)
{
	int c;
	int xtermresize=0;
	char *p = (char *) 0;
	char sysconfdir[BUFSIZ];
	struct dirent *dirp;
	DIR *dp;
	session *scan;
	session *last;

	progname = argv[0];

	if ((p = (char *) strrchr(progname, '/')) != (char *) 0)
		progname = p + 1;

	if (strstr(getenv("TERM"),"xterm") != NULL ) {
		xtermresize=1;
	}

	snprintf(sysconfdir, BUFSIZ - 1, "%s/sudosh.conf", SYSCONFDIR);
	parse(&config_option, sysconfdir);

	snprintf(match, BUFSIZ - 1,
		 "%%32[^%c]%c%%32[^%c]%c%%32[^%c]%c%%ld%c%%64[A-Za-z0-9]",
		 config_option.fdl, config_option.fdl, config_option.fdl,
		 config_option.fdl, config_option.fdl, config_option.fdl,
		 config_option.fdl);

	while (1) {
		c = getopt(argc, argv, "d:hVvxX");

		if (c == EOF)
			break;

		switch (c) {
			case 'X':
				xtermresize=1;
				break;
			case 'x':
				xtermresize=0;
				break;
			case 'V':
			case 'v':
				fprintf(stdout, "%s version %s xtermresize: %d\n", progname, VERSION,xtermresize);
				exit(EXIT_SUCCESS);
				break;
			case 'h':
				fprintf(stdout, "Usage: %s ID [MULTIPLIER] [MAXWAIT]\n",
					progname);
				fprintf(stdout,
					"Replay %s sessions as they were originally recorded.\n\n",
					progname);
				fprintf(stdout,
					"-d	Specify alternate LOGDIR where sessions are stored.\n");
				fprintf(stdout, "-x	disable xterm resize support.\n");
				fprintf(stdout, "-X	enable xterm resize support.\n");
				fprintf(stdout, "-h	Show help synopsis.\n");
				fprintf(stdout, "-v,-V	Show version.\n\n");
				fprintf(stdout,
					"ID\n	The session id of which you wish to view.  The format is user-id\n");
				fprintf(stdout,
					"	You may obtain a list of ids by just running %s\n\n",
					progname);
				fprintf(stdout, "MULTIPLIER\n");
				fprintf(stdout,
					"	This is optional.  If you wish to \"speed up\" the playback of the\n");
				fprintf(stdout,
					"	session, specify a rate to increase it.  For example if you used\n");
				fprintf(stdout,
					"	a multiplier of \"2\" it would playback the session 2x the original\n");
				fprintf(stdout, "	recording speed.\n\n");
				fprintf(stdout, "MAXWAIT\n");
				fprintf(stdout,
					"	This is optional.  If you do not want to wait during any long pauses\n");
				fprintf(stdout,
					"	use this option to specify a maximum wait time.  For example a MAXWAIT\n");
				fprintf(stdout,
					"	of 2 would wait a maximum of 2 seconds then continue.\n\n");
				fprintf(stdout,
					"Example: how to get a list of sessions to replay.\n");
				fprintf(stdout, "# %s\n\n", progname);
				fprintf(stdout,
					"Example: how to replay a session as it was recorded.\n");
				fprintf(stdout, "# %s ID\n", progname);
				fprintf(stdout,
					"Example: how to replay a session, but just echo the contains to the screen.\n");
				fprintf(stdout, "# %s ID 0\n\n", progname);
				fprintf(stdout, "This is %s version %s\n", progname, VERSION);
				fprintf(stdout, "Report bugs to <%s>\n", PACKAGE_BUGREPORT);
				exit(EXIT_SUCCESS);
				break;
			case 'd':
				strncpy(config_option.logdir, optarg, BUFSIZ - 1);
				break;
			default:
				fprintf(stderr, "%s: unrecognized option `%c'\n", progname, c);
				fprintf(stderr, "Try `%s -h' for more information.\n",
					progname);
				exit(EXIT_FAILURE);
				break;
		}
	}


	argc -= optind;
	argv += optind;

	if (argc) {

		char script[BUFSIZ];
		char time[BUFSIZ];
		char from[BUFSIZ];
		char to[BUFSIZ];
		char randstr[BUFSIZ];
		time_t e;
		double div, maxwait;
//		session *s;

		snprintf(match, BUFSIZ - 1,
				 "%%32[^%c]%c%%32[^%c]%c%%ld%c%%64[A-Za-z0-9]",
				 config_option.fdl, config_option.fdl, config_option.fdl,
				 config_option.fdl, config_option.fdl);

		if (sscanf(argv[0], match, from, to, &e, randstr) != 4) {
			LL();
			fprintf(stderr, "%s: invalid format: %s\n", progname, argv[0]);
			fprintf(stderr, "%s: the appropiate format is %s\n", progname, match);
			exit(EXIT_FAILURE);
		}

		snprintf(script,BUFSIZ-1,"%s/%s%c%s%c%s%c%ld%c%s",config_option.logdir,from,config_option.fdl,to,config_option.fdl,"script",config_option.fdl,e,config_option.fdl,randstr);
		snprintf(time,BUFSIZ-1,"%s/%s%c%s%c%s%c%ld%c%s",config_option.logdir,from,config_option.fdl,to,config_option.fdl,"time",config_option.fdl,e,config_option.fdl,randstr);

		div = 1.0;
		maxwait = 0;

		if (argc > 1)
			sscanf(argv[1],"%lf",&div);

		if (argc > 2)
			sscanf(argv[2],"%lf",&maxwait);

		if (div == 0.0) {

			FILE *f_script = fopen(script, "r");
			char buffer[BUFSIZ];

			if (f_script == (FILE *) 0) {
				LL();
				fprintf(stderr, "%s: %s: %s (%i)\n", progname,
						script, strerror(errno), errno);
				exit(EXIT_FAILURE);
			}
			size_t count=0;
			while ((count=fread(&buffer, 1, BUFSIZ, f_script)))
				write2output(buffer,count);
			fclose(f_script);
			write2output("\r\n",2);

		}
		else
			replay(time, script, div, maxwait,xtermresize);

		exit(EXIT_SUCCESS);
	}
	if ((dp = opendir(config_option.logdir)) == (DIR *) 0) {
		LL();
		fprintf(stderr, "%s: can't opendir(%s): %s (%i)\n", progname,
				config_option.logdir, strerror(errno), errno);
		exit(EXIT_FAILURE);
	}

	while ((dirp = readdir(dp)) != (struct dirent *) 0) {
		session *s;
		char from[BUFSIZ];
		char to[BUFSIZ];
		char type[BUFSIZ];
		char randstr[BUFSIZ];
		time_t e;

		if (strcmp(dirp->d_name, ".") == 0 ||
			strcmp(dirp->d_name, "..") == 0)
			continue;


		if (sscanf(dirp->d_name, match, from, to, type, &e, randstr) != 5) {
			LL();
			fprintf(stderr, "[warning]: invalid file: %s\n", dirp->d_name);
			continue;
		}

		if (strcmp(type, "custom") == 0) {
			// If it is already present, then it must be ignored
			continue;
		}

		if (strcmp(type, "script") &&
			strcmp(type, "input") &&
			strcmp(type, "time") &&
			strcmp(type, "interactive")) {
			fprintf(stderr, "[warning]: invalid type (%s): %s\n", type,
					dirp->d_name);
			continue;
		}

		if(!strcmp(type, "interactive")) {
			char *ptr;
			FILE *f;
			s = session_malloc();

			s->secs = 0;
			s->e = e;
			snprintf(s->script.str, BUFSIZ - 1, "%s/%s", config_option.logdir, dirp->d_name);

			f = fopen(s->script.str, "r");

			if(f == (FILE *) 0) {
				fprintf(stderr, "%.100s: %.100s (%i)\n", s->script.str, strerror(errno), errno);
				exit(EXIT_FAILURE);
			}

			if (!fgets(s->id, BUFSIZ - 1, f)) {
				fprintf(stderr, "fgets failed, should not happen");
			}

			fclose(f);

			ptr = strchr(s->id, '\n');
			if(ptr != (char *) 0)
				*ptr++ = '\0';

			strncpy(s->from, from, BUFSIZ - 1);
			strncpy(s->to, to, BUFSIZ - 1);
			strncpy(s->type, type, BUFSIZ - 1);
			strncpy(s->randstr, randstr, BUFSIZ - 1);
			strftime(s->date, 20, "%m/%d/%Y %H:%M:%S", localtime(&s->e));
			link_session(s);
			continue;
		}

		s = find_session(from, to, e, randstr);

		if (s == (session *) 0) {

			s = session_malloc();
			s->secs = 0;

			if (!strcmp(type, "script"))
				snprintf(s->script.str, BUFSIZ - 1, "%s/%s", config_option.logdir, dirp->d_name);

			if (!strcmp(type, "input"))
				snprintf(s->input.str, BUFSIZ - 1, "%s/%s", config_option.logdir, dirp->d_name);

			if (!strcmp(type, "time")) {
				snprintf(s->time.str, BUFSIZ - 1, "%s/%s", config_option.logdir, dirp->d_name);
				count_dur(s);
			}

			s->e = e;
			strncpy(s->from, from, BUFSIZ - 1);
			strncpy(s->to, to, BUFSIZ - 1);
			strncpy(s->type, type, BUFSIZ - 1);
			strncpy(s->randstr, randstr, BUFSIZ - 1);
			strftime(s->date, 20, "%m/%d/%Y %H:%M:%S", localtime(&s->e));
			snprintf(s->id, BUFSIZ - 1, "%s%c%s%c%ld%c%s", s->from,
					 config_option.fdl, s->to, config_option.fdl, s->e,
					 config_option.fdl, s->randstr);

			link_session(s);
		}
		else {
			if (!strcmp(type, "script"))
				snprintf(s->script.str, BUFSIZ - 1, "%s/%s", config_option.logdir, dirp->d_name);

			if (!strcmp(type, "input"))
				snprintf(s->input.str, BUFSIZ - 1, "%s/%s", config_option.logdir, dirp->d_name);

			if (!strcmp(type, "time")) {
				snprintf(s->time.str, BUFSIZ - 1, "%s/%s", config_option.logdir, dirp->d_name);
				count_dur(s);
			}
		}
	}

	if (closedir(dp) < 0) {
		LL();
		fprintf(stderr, "%s: can't closedir(%s): %s (%i)\n", progname,
				config_option.logdir, strerror(errno), errno);
		exit(EXIT_FAILURE);
	}


	if (!s_top) {
		fprintf(stderr, "No sessions are logged in %s\n",
				config_option.logdir);
		exit(EXIT_SUCCESS);
	}

	fprintf(stderr, "%-19s %-8s %-12s %-12s %s\n", "Date", "Duration", "From", "To", "ID");
	fprintf(stderr, "%-19s %-8s %-12s %-12s %s\n", "====", "========", "====", "==", "==");

	s_top = sort_list(s_top);
	last = s_top;

	for (scan = s_top; scan; scan = scan->next) {

		if ((scan->input.fd = open(scan->input.str, O_RDONLY)) == -1) {

			if(strcmp(scan->type, "interactive")) {

				fprintf(stderr, "[warning]: session %s is missing input information.\n", scan->id);
			}
		}
		else {
			close(scan->input.fd);
		}

		if ((scan->script.fd = open(scan->script.str, O_RDONLY)) == -1) {
			LL();
			fprintf(stderr, "[error]: session %s is missing script information.\n", scan->id);
			continue;
		}
		else {
			close(scan->script.fd);
		}

		if(!strcmp(scan->type, "interactive"))
			fprintf(stdout, "%-19s %-8s %-12s %-12s %s\n", scan->date, "-c arg", scan->from, scan->to, scan->id);
		else
			fprintf(stdout, "%-19s %-8s %-12s %-12s %s\n", scan->date, int2smalltime(scan->secs), scan->from, scan->to, scan->id);

		last = scan;
	}

	if (s_top && last)
		fprintf(stderr, "\nUsage: %s ID [MULTIPLIER] [MAXWAIT]\n" "See '%s -h' for more help.\n" "Example: %s %s 1 2\n", progname, progname, progname, last->id);
	else
		fprintf(stderr, "\nUsage: %s ID [MULTIPLIER] [MAXWAIT]\n" "See '%s -h' for more help.\n", progname, progname);

	exit(EXIT_SUCCESS);
}


void replay(const char *time, const char *script, double div, double maxwait,int xtermresize)
{
	char read_buffer[BUFSIZ];
	double ftime = 0;
	double total = 0;
	int b = 0;
	int bInput = 0;
	int r = 0;
	int row = 0;
	int col = 0;
	struct winsize win;
	struct winsize winorig;
	char buffer[BUFSIZ];
	struct s_file s_time;
	struct s_file s_script;
	struct timespec waitforit;

	if (ioctl (0, TIOCGWINSZ, &win) == -1) {
		LL();
		fprintf(stderr, "%s: %s: %s (%i)\n", progname, time, strerror(errno), errno);
		exit(EXIT_FAILURE);
	}
        /* save original winsize*/
        if (xtermresize) ioctl (0, TIOCGWINSZ, &winorig);

	if (div < 0)
		div = 1;

	s_time.f = fopen(time, "r");

	if (s_time.f == (FILE *) 0) {
		LL();
		fprintf(stderr, "%s: %s: %s (%i)\n", progname, time, strerror(errno), errno);
		exit(EXIT_FAILURE);
	}

	if ((s_script.fd = open(script, O_RDONLY)) == -1) {
		LL();
		fprintf(stderr, "%s: %s: %s (%i)\n", progname, script, strerror(errno), errno);
		exit(EXIT_FAILURE);
	}
	clock_gettime(CLOCK_MONOTONIC, &waitforit);

	for (s_time.line = 1; fgets(buffer, BUFSIZ - 1, s_time.f); s_time.line++) {

		/* find the five numbers */
		if (sscanf(buffer, "%lf %i %i %i %i\n", &ftime, &b, &bInput,&col,&row) != 5) {
		/* find the three numbers */
			if (sscanf(buffer, "%lf %i %i\n", &ftime, &b, &bInput) != 3) {

				/* find the two numbers (old format) */
				if (sscanf(buffer, "%lf %i\n",&ftime , &b) != 2) {

					LL();
					fprintf(stderr, "[error]: line %i: invalid timing information\n", s_time.line);
					fprintf(stderr, "[error]: expected one of the following formats:\n");
					fprintf(stderr, "[error]: %%lf %%i %%i %%i %%i\\n)\n");
					fprintf(stderr, "[error]: %%lf %%i %%i\\n)\n");
					fprintf(stderr, "[error]: %%lf %%i\\n)\n");
					exit(EXIT_FAILURE);
				}
			}
		} else {
			/* we have the new format with col & row info */
			if (xtermresize && (row!=win.ws_row || col!=win.ws_col)) {
				win.ws_row=row;
				win.ws_col=col;
				/* update xterm geometry */
				printf("\033[%d;%d;%dt", 8, row, col);
			}
		}

		if (maxwait>0.0 && ftime > maxwait) {
			ftime = maxwait;
		} 
		total+=ftime;
		ftime = ftime / div;

		if (b > 1024 * 1024 * 8) {	/* 8MB */
			LL();
			fprintf(stderr, "[error]: line %i: wanted to read %i bytes, but the limit is 8MB.\n", s_time.line, b);
			exit(EXIT_FAILURE);
		}

		memset(read_buffer, '\0', BUFSIZ);
		r = read(s_script.fd, read_buffer, (size_t) b);

		if (r != b) {
			LL();
			fprintf(stderr, "[failure]: read %i bytes out of %i.\n", r, b);
			exit(EXIT_FAILURE);
		}

		
		/* Add the time you want to sleep */
		waitforit.tv_nsec += (long)((ftime-(long)ftime)*1000000000);
		waitforit.tv_sec += (long)(ftime);

		/* Adjust nsec to sec */
		while(waitforit.tv_nsec >= 1000000000) {
			waitforit.tv_nsec -= 1000000000;
			waitforit.tv_sec++;
		}
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &waitforit, NULL);
		fputs(read_buffer, stdout);
		fflush(stdout);
		memset(read_buffer, '\0', BUFSIZ);
	}
	/* reset xterm geometry */
        if (xtermresize) printf("\033[%d;%d;%dt", 8, winorig.ws_row, winorig.ws_col);
	fprintf(stderr, "[info]: EOF\n");
	fflush(stderr);
}


session *session_malloc(void)
{
	session *s = (session *) malloc(sizeof(session));

	if (!s) {
		LL();
		fprintf(stderr, "%s: session malloc() failed: %s (%i)\n", progname, strerror(errno), errno);
		exit(EXIT_FAILURE);
	}
	else
		memset(s, '\0', sizeof(session));

	return s;
}


session *find_session(const char *from, const char *to, time_t e, const char *randstr)
{
	session *scan;

	for (scan = s_top; scan; scan = scan->next)
		if (scan->e == e)
			if (!strcmp(from, scan->from))
				if (!strcmp(to, scan->to))
					if (!strcmp(randstr, scan->randstr))
						return scan;

	return (session *) 0;
}


session *find_max(time_t e)
{
	session *scan;

	for (scan = s_top; scan; scan = scan->next)
		if (scan->e == e)
			return scan;

	return (session *) 0;
}


void link_session(session * s)
{
	s->next = s_top;

	if (s_top)
		s_top->prev = s;

	s_top = s;
}


void unlink_session(session * s)
{
	if (s->next)
		s->next->prev = s->prev;

	if (s->prev)
		s->prev->next = s->next;
	else
		s_top = s->next;

	s->next = (session *) 0;
	s->prev = (session *) 0;

	return;
}


void show_sessions(void)
{
	session *scan;

	for (scan = s_top; scan; scan = scan->next) {
		fprintf(stderr, "%s\n", scan->from);
	}
}


session *sort_list(session * list)
{
	session *p, *q, *e, *tail;
	int insize, nmerges, psize, qsize, i;

	if (!list)
		return (session *) 0;

	insize = 1;

	while (1) {

		p = list;
		list = (session *) 0;
		tail = (session *) 0;
		nmerges = 0;

		while (p) {

			nmerges++;
			q = p;
			psize = 0;

			for (i = 0; i < insize; i++) {

				psize++;
				q = q->next;

				if (!q)
					break;
			}

			qsize = insize;

			while (psize > 0 || (qsize > 0 && q)) {

				if (psize == 0) {
					e = q;
					q = q->next;
					qsize--;
				}
				else if (qsize == 0 || !q) {
					e = p;
					p = p->next;
					psize--;
				}
				else if (p->e < q->e) {
					e = p;
					p = p->next;
					psize--;
				}
				else {
					e = q;
					q = q->next;
					qsize--;
				}

				if (tail)
					tail->next = e;
				else
					list = e;

				tail = e;
			}

			p = q;
		}

		tail->next = (session *) 0;

		if (nmerges <= 1)
			return list;

		insize *= 2;
	}
}


void count_dur(session * s)
{
	struct s_file s_time;
	char buffer[BUFSIZ];
	double t_time = 0;

	s_time.f = fopen(s->time.str, "r");

	if (s_time.f == (FILE *) 0) {
		LL();
		fprintf(stderr, "%s: %s (%i)\n", s->time.str, strerror(errno), errno);
		exit(EXIT_FAILURE);
	}

	for (s_time.line = 1; fgets(buffer, BUFSIZ - 1, s_time.f); s_time.line++) {

		char time[BUFSIZ];
		double ftime = 0;
		int b = 0;

		if (sscanf(buffer, "%s %i\n", (char *) &time, &b) != 2) {	/* find the two numbers */
			LL();
			fprintf(stderr, "[error]: line %i: invalid timing information (!= %%lf %%i\\n)\n", s_time.line);
			fclose(s_time.f);
			exit(EXIT_FAILURE);
		}

		if (sscanf(time, "%lf", &ftime) != 1) {
			LL();
			fprintf(stderr, "[error]: line %i: invalid double (!= %%lf)", s_time.line);
			fclose(s_time.f);
			exit(EXIT_FAILURE);
		}

		t_time += ftime;
	}
	fclose(s_time.f);

	s->secs = (long) t_time;
}
