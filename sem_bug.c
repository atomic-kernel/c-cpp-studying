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

int main()
{
	const int semid = semget(30, 1, IPC_EXCL | IPC_CREAT | 0666);
	assert(semid >= 0);
	union semun arg;

	arg.val = 0;
	assert(semctl(semid, 0, SETVAL, arg) == 0);

	while (1) {
		pid_t pid = fork();
		if (pid == 0) {
			struct sembuf sops;
			sops.sem_num = 0;
			sops.sem_op = 1;
			sops.sem_flg = 0;
			if (semop(semid, &sops, 1)) {
				perror("child semop failed");
				exit(-1);
			}
			while (1) {}
		}
		struct sembuf sops;
		sops.sem_num = 0;
		sops.sem_op = -1;
		sops.sem_flg = SEM_UNDO;
		if (semop(semid, &sops, 1)) {
			perror("semop failed");
			exit(-1);
		}
		assert(kill(pid, SIGKILL) == 0);
		assert(waitpid(pid, NULL, 0) == pid);
	}
	return 0;
}
