#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>

int tsh_cd(char **args);
int tsh_help(char **args);
int tsh_exit(char **args);
int tsh_ls(char **args);
int tsh_echo(char **args);
int tsh_cat(char **args);
int tsh_pwd(char **args);
int tsh_mkdir(char **args);
int tsh_touch(char **args);
int tsh_rm(char **args);
int tsh_whoami(char **args);
int tsh_rmdir(char **args);

char *builtin_str[] = {
	"cd",
	"help",
	"ls",
	"echo",
	"cat",
	"pwd",
	"mkdir",
	"touch",
	"rm",
	"whoami",
	"rmdir",
	"exit"
};

int (*builtin_func[]) (char **) = {
	&tsh_cd,
	&tsh_help,
	&tsh_ls,
	&tsh_echo,
	&tsh_cat,
	&tsh_pwd,
	&tsh_mkdir,
	&tsh_touch,
	&tsh_rm,
	&tsh_whoami,
	&tsh_rmdir,
	&tsh_exit
};

int tsh_num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}

char *getcwd(char *buf, size_t size);
char *getlogin();

int tsh_cd(char **args)
{
	if (args[1] == NULL) {
		fprintf(stderr, "tsh: expected argument to \"cd\"\n");
	} else {
		if (chdir(args[1]) != 0) {
			perror("tsh");
		}
	}
	return 1;
}

int tsh_help(char **args)
{
	int i;
	printf("Tyler's Shell\n");
	printf("Type program names and arguments, and hit enter.\n");
	printf("The following are in:\n");

	for (i = 0; i < tsh_num_builtins(); i++) {
		printf("  %s\n", builtin_str[i]);
	}

	printf("Use the man command for information on the other programs.\n");
	return 1;
}

int tsh_ls(char **args)
{
	DIR *mydir;
	struct dirent *myfile;

	if (args[1] != NULL) {
		mydir = opendir(args[1]);
	} else {
		mydir = opendir("./");
	}
	while((myfile = readdir(mydir)) != NULL)
	{
		printf("  %s\n", myfile->d_name);
	}
	closedir(mydir);
	return 1;
}

int tsh_echo(char **args)
{
	int i;
	int arraylen = (int) ( sizeof(args) / sizeof(char *) );

	printf("%d\n", arraylen);
//	printf("%d\n", arraylen);
//	printf("%d\n", (int) ( sizeof(args[1]) ));
	//for (i = 1; i < arraylen; i++) {
	//	printf("%s", args[i]);
	//}
//	printf("%s", args[2]);

//	printf("\n");

	printf("this command is currently broken\n");

	return 1;
}

int tsh_cat(char **args)
{
	FILE *fptr;

	char c;

	fptr = fopen(args[1], "r");
    	if (fptr == NULL)
    	{
        	printf("Cannot open file \n");
        	exit(0);
    	}

	c = fgetc(fptr);
   	while (c != EOF)
	{
		printf ("%c", c);
		c = fgetc(fptr);
	}

    	fclose(fptr);
    	return 1;
}

int tsh_pwd(char **args)
{
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		printf("%s\n", cwd);
	}else {
		printf("An error occured\n");
	}

	return 1;
}

int tsh_mkdir(char **args)
{
	struct stat st = {0};

	if (stat(args[1], &st) == -1) {
		mkdir(args[1], 0700);
	}

	return 1;
}

int tsh_rmdir(char **args)
{
	struct stat st = {0};

	if (stat(args[1], &st) == 0) {
		rmdir(args[1]);
	}

	return 1;
}

int tsh_touch(char **args)
{
	FILE *fp;
	fp=fopen(args[1],"w");
	fclose(fp);
	return 1;
}

int tsh_rm(char **args)
{
	int status;

	status = remove(args[1]);

	if (status == 0) {
		printf("%s deleted successfully.\n", args[1]);
	}else {
		printf("unable to delete file\n");
	}

	return 1;
}

int tsh_whoami(char **args) {
	char *username;
	username = (char *) malloc(10*sizeof(char));
	username = getlogin();
	printf("%s\n", username);
}

int tsh_exit(char **args)
{
	return 0;
}

int tsh_launch(char **args)
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0) {
		if (execvp(args[0], args) == -1) {
			perror("tsh");
		}
		exit(EXIT_FAILURE);
	} else if (pid < 0) {
		perror("tsh");
	} else {
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}

int tsh_execute(char **args)
{
	int i;

	if (args[0] == NULL) {
		return 1;
	}

	for (i = 0; i < tsh_num_builtins(); i++) {
		if (strcmp(args[0], builtin_str[i]) == 0) {
			return (*builtin_func[i])(args);
		}
	}

	return tsh_launch(args);
}

#define TSH_RL_BUFSIZE 1024

char *tsh_read_line(void)
{
	int bufsize = TSH_RL_BUFSIZE;
	int position = 0;
	char *buffer = malloc(sizeof(char) * bufsize);
	int c;

	if (!buffer) {
		fprintf(stderr, "tsh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		c = getchar();

		if (c == EOF) {
			exit(EXIT_SUCCESS);
		} else if (c == '\n') {
			buffer[position] = '\0';
			return buffer;
		} else {
			buffer[position] = c;
		}
		position++;

		if (position >= bufsize) {
			bufsize += TSH_RL_BUFSIZE;
			buffer = realloc(buffer, bufsize);
			if (!buffer) {
				fprintf(stderr, "tsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}


#define TSH_TOK_BUFSIZE 64
#define TSH_TOK_DELIM " \t\r\n\a"

char **tsh_split_line(char *line)
{
	int bufsize = TSH_TOK_BUFSIZE, position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token, **tokens_backup;

	if (!tokens) {
		fprintf(stderr, "tsh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, TSH_TOK_DELIM);
	while (token != NULL) {
		tokens[position] = token;
		position++;

		if (position >= bufsize) {
			bufsize += TSH_TOK_BUFSIZE;
			tokens_backup = tokens;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if (!tokens) {
				free(tokens_backup);
				fprintf(stderr, "tsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, TSH_TOK_DELIM);
	}

	tokens[position] = NULL;
	return tokens;
}

void tsh_loop(void)
{
	char *line;
	char **args;
	int status;

	char hostname[HOST_NAME_MAX];
	char *username;
	char cwd[PATH_MAX];
	int result;
	bool nofail = true;

	result = gethostname(hostname, HOST_NAME_MAX);
	if (result) {
		nofail = false;
	}

	username = (char *) malloc(10*sizeof(char));
	username = getlogin();

	do {
		bool original_bool = nofail;
		if (getcwd(cwd, sizeof(cwd)) == NULL) {
			nofail = false;
		}

		printf("%s@%s:%s$ ", username, hostname, cwd);

		line = tsh_read_line();
		args = tsh_split_line(line);
		status = tsh_execute(args);
		nofail = original_bool;

		free(line);
		free(args);
	} while (status);
}

int main(int argc, char **argv)
{
	tsh_loop();

	return EXIT_SUCCESS;
}
