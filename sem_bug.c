#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <signal.h>
#include <sys/sem.h>
#include <sys/wait.h>

union semun {
	int              val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO
				    (Linux-specific) */
};

int semid;

static void child(void)
{
	// Notify the parent process that initialization has completed
	struct sembuf sops;
	sops.sem_num = 0;
	sops.sem_op = 1;
	sops.sem_flg = 0;
	if (semop(semid, &sops, 1)) {
		perror("child semop failed");
		exit(-1);
	}
	
	// wait for a kill
	while (1) {}
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("usage: ./test sem_key\n");
		return 1;
	}

	const key_t key = atoi(argv[1]);
	assert(key != 0);
	semid = semget(key, 1, IPC_EXCL | IPC_CREAT | 0666);
	assert(semid >= 0);

	union semun arg;
	arg.val = 0;
	assert(semctl(semid, 0, SETVAL, arg) == 0);

	while (1) {
		pid_t pid = fork();

		if (pid == 0)
			child();

		// Wait for the child process to complete initialization
		struct sembuf sops;
		sops.sem_num = 0;
		sops.sem_op = -1;
		sops.sem_flg = SEM_UNDO;
		if (semop(semid, &sops, 1)) {
			perror("semop failed");
			exit(-1);
		}

		// kill child
		assert(kill(pid, SIGKILL) == 0);
		assert(waitpid(pid, NULL, 0) == pid);
	}
	return 0;
}
