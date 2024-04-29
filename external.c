struct sembuf {
	short sem_num;
	short sem_op;
	short sem_flg;
};
int semop(int, struct sembuf*, int);

int P(int sem_id, int n) {
	struct sembuf op;
	op.sem_num = n;
	op.sem_op = -1;
	op.sem_flg = 0;
	return semop(sem_id, &op, 1);
}

int V(int sem_id, int n) {
	struct sembuf op;
	op.sem_num = n;
	op.sem_op = 1;
	op.sem_flg = 0;
	return semop(sem_id, &op, 1);
}

int wait_for_zero(int sem_id, int n) {
	struct sembuf op;
	op.sem_num = n;
	op.sem_op = 0;
	op.sem_flg = 0;
	return semop(sem_id, &op, 1);
}