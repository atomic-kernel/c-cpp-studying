#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
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

        if (semid < 0) {
                printf("failed to get semid\n");
                exit(-1);
        }
        union semun arg;

        arg.val = 0;
        if (semctl(semid, 0, SETVAL, arg)) {
                printf("semctl failed!\n");
                exit(1);
        }

        while (1) {
                pid_t pid = fork();
                if (pid == 0) {
                        struct sembuf sops;
                        sops.sem_num = 0;
                        sops.sem_op = 1;
                        sops.sem_flg = 0;
                        //sleep(1);
                        //printf("wake\n");
                        if (semop(semid, &sops, 1)) {
                                perror("child semop failed");
                                printf("child semop failed\n");
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
                        printf("semop failed\n");
                        exit(-1);
                }
                //printf("aaaaa\n");
                if (kill(pid, SIGKILL)) {
                        printf("kill failed!\n");
                        exit(-1);
                }
                if (waitpid(pid, NULL, 0) != pid) {
                        printf("waitpid failed!\n");
                        exit(-1);
                }
        }
        return 0;
}
