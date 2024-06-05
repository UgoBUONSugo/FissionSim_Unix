#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#define SIM_PRINT \
printf("|[\033[0;34m//\033[0;37m"); \
			for(int i = 0; i < (ratio * 20) && i < 5; i++){ \
				printf("\033[0;36m/"); \
			} \
			for(int i = 5; i < (ratio * 20) && i < 15; i++){ \
				printf("\033[0;32m/"); \
			} \
			for(int i = 15; i <= (ratio * 20) && i <= 20; i++){ \
				printf("\033[0;33m/"); \
			} \
			for(int i = 0; i < (20 - (ratio * 20)); i++){ \
				printf("\033[0;37m-"); \
			} \
			printf("\033[0;31m//\033[0m]                     |\n"); \
			printf(" ------------------------------------------------ \n"); \
			printf("STATUS INIBITORE: "); \
			inhib_status == 0 ? printf("\x1B[31mOFF \033[0m\n\n") : printf("\x1B[32mON \033[0m\n\n")

struct SimStats{
	long liberated_energy;
	long consumed_energy;
	long absorbed_energy;
	int activation_count;
	int activation_interrupted;
	int split_count;
	int waste_count;
};