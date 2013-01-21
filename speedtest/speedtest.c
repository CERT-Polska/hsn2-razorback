#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include "speedtest.h"

int sock;

double testRuntime() {
	int i;
	struct timeval startTime, endTime;
	char command[100];

	sprintf(command, "../pdfFox/pdfFox %s", FILE_PATH);

	gettimeofday(&startTime, NULL);
	for (i = 0; i < TEST_COUNT; ++i) {
		system(command);
	}
	gettimeofday(&endTime, NULL);

	double tS = startTime.tv_sec * 1000000 + (startTime.tv_usec);
	double tE = endTime.tv_sec * 1000000  + (endTime.tv_usec);
	return (tE - tS) / 1000000;
}

double testOneConnection() {
	int i;
	struct timeval startTime, endTime;
	char command[TEST_COUNT * (strlen(FILE_PATH) + 2) + 50];

	strcpy(command, "../pdfFox/pdfFox ");
	for (i = 0; i < TEST_COUNT; ++i) {
		strcat(command, FILE_PATH);
		strcat(command, " ");
	}

	gettimeofday(&startTime, NULL);
	system(command);
	gettimeofday(&endTime, NULL);

	double tS = startTime.tv_sec * 1000000 + (startTime.tv_usec);
	double tE = endTime.tv_sec * 1000000  + (endTime.tv_usec);
	return (tE - tS) / 1000000;
}

int main(int argc, char ** argv) {
	double result1, result2, sum1, sum2;
	int i;

	sum1 = sum2 = 0.0;

	for (i = 0; i < TRIALS_COUNT; ++i) {
		sum1 += testRuntime();
		sum2 += testOneConnection();
	}

	result1 = sum1 / TRIALS_COUNT;
	result2 = sum2 / TRIALS_COUNT;


	printf("\ndone %d tests\n", TEST_COUNT);
	printf("done %d repeats\n", TRIALS_COUNT);
	printf ("runtime test - %.5lf seconds\n", result1);
	printf ("one connection test - %.5lf seconds\n", result2);
	printf("result1 / result2 = %.5lf\n", result1 / result2);


}


