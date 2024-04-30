#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

// Usage: your_docker.sh run <image> <command> <arg1> <arg2> ...
int main(int argc, char *argv[]) {
	// Disable output buffering
	setbuf(stdout, NULL);
	const size_t BUFSIZE = 1024;

	// You can use print statements as follows for debugging, they'll be visible when running tests.
//	printf("Logs from your program will appear here!\n");

	// Uncomment this block to pass the first stage
	//
	int pipe_stdout[2]; // Pipe for stdout
	int pipe_stderr[2]; // Pipe for stderr

	// Create pipes
	if (pipe(pipe_stdout) == -1 || pipe(pipe_stderr) == -1) {
		perror("Pipe creation failed");
		exit(EXIT_FAILURE);
	}
	 char *command = argv[3];
	 int child_pid = fork();
	 if (child_pid == -1) {
	     printf("Error forking!");
	     return 1;
	 }

	 if (child_pid == 0) {
	 	   // Replace current program with calling program
	     close(pipe_stdout[0]);
	     close(pipe_stderr[0]);
	     dup2(pipe_stdout[1], STDOUT_FILENO);
	     dup2(pipe_stderr[1], STDERR_FILENO);
	     execv(command, &argv[3]);
	 } else {
	 	   // We're in parent

		 char buf[1024] = {0};
		 int count = 0;
		 close(pipe_stdout[1]);
		 close(pipe_stderr[1]);
		 while ((count = read(pipe_stdout[0], buf, BUFSIZE)) > 0)
			 write(STDOUT_FILENO, buf, count);
		 while ((count = read(pipe_stderr[0], buf, BUFSIZE)) > 0)
			 write(STDERR_FILENO, buf, count);

		 int status;
		 int corpse = wait(&status);
		 if (corpse < 0)
			 printf("Failed to wait for process %d (errno = %d)\n", (int)child_pid, errno);
		 else if (corpse != child_pid)
			 printf("Got corpse of process %d (status 0x%.4X) when expecting PID %d\n",
					corpse, status, (int)child_pid);
		 else if (WIFEXITED(status)){
			return WEXITSTATUS(status);
		 }
		 else if (WIFSIGNALED(status))
			 printf("Process %d exited because of a signal 0x%.4X (signal %d = 0x%.2X)\n",
					corpse, status, WTERMSIG(status), WTERMSIG(status));
		 else
			 printf("Process %d exited with status 0x%.4X which is %s\n",
					corpse, status, "neither a normal exit nor the result of a signal");
//	 	   printf("Child terminated");
	 }

	return 0;
}
