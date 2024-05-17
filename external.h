struct SimStats{
	long liberated_energy;
	long consumed_energy;
	long absorbed_energy;
	int activation_count;
	int activation_interrupted;
	int split_count;
	int waste_count;
};

int P(int sem_id, int n);
int V(int sem_id, int n);
int wait_for_zero(int sem_id, int n);
void sim_end();