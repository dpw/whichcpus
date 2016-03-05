#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

static int cpu()
{
	int res = sched_getcpu();
	assert(res >= 0);
	return res;
}

static volatile int yet_to_run;

static void *thread(void *v_cpuset)
{
	cpu_set_t *cpuset = v_cpuset;

	CPU_ZERO(cpuset);
	CPU_SET(cpu(), cpuset);

	if (!__sync_sub_and_fetch(&yet_to_run, 1))
		return NULL;

	do
		CPU_SET(cpu(), cpuset);
	while (yet_to_run);

	return NULL;
}

static void print_cpu_set(cpu_set_t *set)
{
	const char *sep = "";
	int i;

	for (i = 0; i < CPU_SETSIZE; i++) {
		if (CPU_ISSET(i, set)) {
			printf("%s%d", sep, i);
			sep = ",";
		}
	}

	if (*sep) {
		printf("\n");
	}
}

void sample(cpu_set_t *set, int max)
{
	int i;
	pthread_t *threads;
	cpu_set_t *cpusets;

	threads = malloc(max * sizeof(*threads));
	cpusets = malloc(max * sizeof(*cpusets));

	yet_to_run = max;

	for (i = 0; i < max; i++) {
		CPU_ZERO(&cpusets[i]);
		if (pthread_create(&threads[i], NULL, thread, &cpusets[i])) {
			perror("pthread_create");
			exit(1);
		}
	}

	for (i = 0; i < max; i++) {
		if (pthread_join(threads[i], NULL)) {
			perror("pthread_join");
			exit(1);
		}

		CPU_OR(set, set, &cpusets[i]);
	}

	free(threads);
	free(cpusets);
}

int main(void)
{
	int max = sysconf(_SC_NPROCESSORS_CONF);
	int rep;
	cpu_set_t res;

	CPU_ZERO(&res);

	/* Take a few samples initially */
	for (rep = 0; rep < 5; rep++) {
		sample(&res, max);
		usleep(100000);
	}

	/* Then keep trying until we get a consistent result. */
	for (;;) {
		cpu_set_t set = res;
		sample(&res, max);

		if (CPU_EQUAL(&res, &set))
			break;

		CPU_OR(&res, &res, &set);
		usleep(100000);
	}

	print_cpu_set(&res);
	return 0;
}
