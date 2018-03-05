/**
 * Running a program as slow to test the background process properly
 *
 * @author : T. Reichherzer
 * @Filename : slow.c
 * @course : COP5990
 * @Project # : 1
 */


#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char ** argv) {

	int i;
	int pid = getpid();
	for (i = 0; i < 10; i++) {
		sleep(1);
		printf("slow %6d : %d\n", pid, i); 
		fflush(NULL);
	}
	return 0; 
}