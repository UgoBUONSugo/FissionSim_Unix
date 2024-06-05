int P(int sem_id, int n);
int V(int sem_id, int n);
int wait_for_zero(int sem_id, int n);
void toggle_signals(int block, int sig);