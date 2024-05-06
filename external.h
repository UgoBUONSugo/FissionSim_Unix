struct SimStats{
	long tot_energy;
	long consumed_energy;
	long absorbed_energy;
	int activation_count;
	int split_count;
	int waste_count;
};

int P(int sem_id, int n);
int V(int sem_id, int n);
int wait_for_zero(int sem_id, int n);