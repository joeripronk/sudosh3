struct s_option {
	char logdir[BUFSIZ];
	char defshell[BUFSIZ];
	char fdl;
	int priority;
	int facility;
	int clearenvironment;
	char **argallow;
};

typedef struct s_option option;
#define FILENAME_FORMAT "%.2048s/%.128s%c%.128s%c%.32s%c%ld%c%.16s"
