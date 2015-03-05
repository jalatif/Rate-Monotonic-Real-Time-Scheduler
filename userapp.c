#define _ISOC99_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#define RUNTIME 5
#define MAX_N 1000
#define MAX_CHARS_PER_LINE 100
#define MAX_LIFE 5

#define RUN_LOOP 10000000
#define MAX_FAC_NUM 27

int mp2_read_status(int id);
int mp2_register(int id, int period, int computation_time);
int mp2_yield(int id);
int mp2_deregister(int id);
double time_calculation(void);
int factorial(int N);
void doJob(int factor);

int main(int argc, char* argv[])
{
    int period = 40;
    unsigned int computation_time;
    unsigned int factor_computation_time;
    double avg_time;
    int ret = 0;
    struct timeval init, start, end;
    double t0, t1, t2;
    
    int id = getpid();
    printf("My pid is %d\n", id);

    int arr_length = sizeof(argv) / sizeof(char*);
    if (argc < 3) {
        printf("Argument not sufficient. Exiting...\n"); 
        return -5;
    }
    period = atoi(argv[1]);
    factor_computation_time = atoi(argv[2]);
    
    /// do some time calculation
    printf("Doing computation_time avg calculation....\n");
    avg_time = time_calculation();
    
    avg_time = avg_time / 1000000.0;
    
    avg_time = round(avg_time);

    printf("Long time is %lf\n", avg_time);

    computation_time = factor_computation_time * (unsigned int) avg_time;
    printf("Arguments are %d, %d\n", period, computation_time); 
    
    if (period <= computation_time) {
        printf("Period is too less for this computation. Exiting...\n");
        return -6;
    }

    ////////////////////////////////////

    // register pid with the kernel mp1 module
    ret = mp2_register(id, period, computation_time);
    if (ret != 0) {
        printf("Please check if /mp2/proc/status file is existing\n. Exiting....\n");
        return ret;
    }

    // read the user time values
    ret = mp2_read_status(id);
    if (ret != 0) {
        printf("Process you are trying to submit will voilate the rate monotonic scheduler admission control.\n");
        //printf("Please check if /mp2/proc/status file is existing\n");
        printf("ID was not registered. Exiting...\n");
        return ret;
    }

    // yield function call
    ret = mp2_yield(id);
    if (ret != 0) {
        printf("Please check if /mp2/proc/status file is existing\n. Exiting....\n");
        return ret;
    }    

    // run some periodic task
    gettimeofday(&init, NULL);    
    t0 = init.tv_sec + (init.tv_usec / 1000000.0);

    printf("[%d] - Time recorder started at %lf\n", id, t1 / 1000);

    printf("Now running some tasks\n");
    int i = 0;
    while (i < MAX_LIFE) {
        //factorial_calculations(computation_time * FACTOR);
        gettimeofday(&start, NULL);    
        t1 = start.tv_sec + (start.tv_usec/1000000.0);  
        printf("[%d] - %dth job started after %.6lf seconds elapsed from start\n", id, (i + 1), (t1 - t0));

        doJob(factor_computation_time);
        gettimeofday(&end, NULL);
        t2 = end.tv_sec+(end.tv_usec/1000000.0);  
        printf("[%d] - %dth job took %.6lf seconds elapsed\n", id, (i + 1), (t2 - t1));

        ret = mp2_yield(id);
        if (ret != 0) {
            printf("Please check if /mp2/proc/status file is existing\n. Exiting....\n");
            return ret;
        }
        i++;
    }

    // Deregister pid from the scheduler
    ret = mp2_deregister(id);
    if (ret != 0) {
        printf("Please check if /mp2/proc/status file is existing\n. Exiting....\n");
        return ret;
    }
    
    //printf("Ran successfully for %ld\n", ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));
    
	return 0;
}

int mp2_read_status(int id) {
    FILE* PRfptr;
    ssize_t len = 0;
    ssize_t read;
    char *line = NULL;
    int contains_pid = -1;
    char pid_line[16];
    
    PRfptr = fopen("/proc/mp2/status", "r");
    if (PRfptr == NULL) {
        printf("/proc/mp2/status file is not present\n");
        return -1;
    }
    
    sprintf(pid_line, "PID => %d", id);

    while ((read = getline(&line, &len, PRfptr)) != -1) {
        //printf("Length of Line is %zu : \n", read);
        if (strstr(line, pid_line) != NULL) {
            contains_pid = 0;
        }
    }
    if (line)
        free(line);
    fclose(PRfptr);

    if (contains_pid == -1) {
        return -1;
    }
    printf("PID Registered\n");
    
    return 0;
}

int mp2_register(int id, int period, int computation_time) {
    FILE* PWfptr = fopen("/proc/mp2/status", "w");
    if (PWfptr == NULL) {
        printf("/proc/mp2/status file is not present\n");
        return -1;
    }
    // Register R, pid, period, processing time
    fprintf(PWfptr, "R, %d, %d, %d", id, period, computation_time);
    fclose(PWfptr);
    
    return 0;
}

int mp2_yield(int id) {
    FILE* PWfptr = fopen("/proc/mp2/status", "w");
    if (PWfptr == NULL) {
        printf("/proc/mp2/status file is not present\n");
        return -1;
    }
    // Yield Y, pid
    fprintf(PWfptr, "Y, %d", id);
    fclose(PWfptr);
    return 0;
}

int mp2_deregister(int id) {
    FILE* PWfptr = fopen("/proc/mp2/status", "w");
    if (PWfptr == NULL) {
        printf("/proc/mp2/status file is not present\n");
        return -1;
    }
    // DeRegister D, pid
    fprintf(PWfptr, "D, %d", id);
    fclose(PWfptr);
    printf("PID DeRegistered\n");

    return 0;
}

int factorial(int N) {
    if (N == 1) return 1;
    else return N * factorial(N - 1);
}

void doJob(int factor) {
    int j = 0;
    for (; j < factor; j++) {
        unsigned int i = 0;
        while(i++ < RUN_LOOP) {
            factorial(MAX_FAC_NUM);
        }
    }
}
double time_calculation() {
    struct timeval start, end;
    unsigned long t1, t2;
    int i, avg_loop = 4;
    gettimeofday(&start, NULL);
    for (i = 0; i < avg_loop; i++)
        doJob(1);
    gettimeofday(&end, NULL);

    t1 = start.tv_sec * 1000000 + start.tv_usec;  
    t2 = end.tv_sec * 1000000 + end.tv_usec;  

    return (t2 - t1) / (1.0 * avg_loop);// / RUN_LOOP;
}