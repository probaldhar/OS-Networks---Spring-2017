/**
 * Implements the functions for reading numbers from a source file.
 *
 * @author T. Reichherzer
 */

#include <stdio.h>
#include <time.h>

struct timespec starttime, endtime;

void startTimer()
{
	// measure time
	clock_gettime(CLOCK_REALTIME, &starttime);
}

float stopTimer()
{
	float elapsed;
	
	// measure time again
	clock_gettime(CLOCK_REALTIME, &endtime);

    int timeDiffSec = endtime.tv_sec - starttime.tv_sec;
    long timeDiffNSec = endtime.tv_nsec - starttime.tv_nsec;
	
	// compute difference
	if (timeDiffNSec < 0) { 
     // borrow a second
        elapsed = timeDiffSec-1 + (1000000000 + timeDiffNSec)/1000000000.0;
	}
	else {
        elapsed = timeDiffSec + timeDiffNSec/1000000000.0;
	}
	
	return elapsed;
}
