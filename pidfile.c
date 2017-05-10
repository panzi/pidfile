#define _DEFAULT_SOURCE

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

pid_t pid;
const char *pidfile;
FILE *stream;

void usage(int argc, char *argv[]) {
	printf(
		"usage: %s PIDFILE COMMAND [ARG]...\n",
		argc > 0 ? argv[0] : "pidfile");
}

void cleanup() {
	if (stream) {
		if (pidfile && unlink(pidfile) == -1) {
			perror(pidfile);
		}

		if (fclose(stream) == EOF) {
			perror(pidfile);
		}

		pidfile = NULL;
		stream = NULL;
	}
}

void handle_term_signal(int signum) {
	(void)signum;

	if (pid > 0) {
		kill(pid, SIGTERM);
	}
}

void handle_passthrough_signal(int signum) {
	if (pid > 0) {
		kill(pid, signum);
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		usage(argc, argv);
		return EXIT_FAILURE;
	}
	pidfile = argv[1];

	if (atexit(cleanup) == -1) {
		perror("register cleanup function");
		return EXIT_FAILURE;
	}

	if (
		signal(SIGHUP,  handle_passthrough_signal) == SIG_ERR ||
		signal(SIGTERM, handle_passthrough_signal) == SIG_ERR ||
		signal(SIGINT,  handle_passthrough_signal) == SIG_ERR ||
		signal(SIGUSR1, handle_passthrough_signal) == SIG_ERR ||
		signal(SIGUSR2, handle_passthrough_signal) == SIG_ERR ||
		signal(SIGQUIT, handle_term_signal) == SIG_ERR ||
		signal(SIGPIPE, handle_term_signal) == SIG_ERR ||
		signal(SIGILL,  handle_term_signal) == SIG_ERR ||
		signal(SIGSEGV, handle_term_signal) == SIG_ERR) {
		perror("register signal handler");
		return EXIT_FAILURE;
	}

	stream = fopen(pidfile, "wxe");
	if (!stream) {
		perror(pidfile);
		return EXIT_FAILURE;
	}

	const char *command = argv[2];
	pid = vfork();
	if (pid == -1) {
		// error
		perror("forking child process");
		return EXIT_FAILURE;
	}
	else if (pid == 0) {
		// child process
		execvp(command, argv + 2);
		perror(command);
		return EXIT_FAILURE;
	}

	fprintf(stream, "%d\n", pid);
	fflush(stream);

	// parent process
	int status = 0;
	if (waitpid(pid, &status, 0) == -1) {
		perror(command);
		pid = -1;
		return EXIT_FAILURE;
	}
	pid = -1;

	return EXIT_SUCCESS;
}
