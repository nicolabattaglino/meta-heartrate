#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>

#define N 2048

typedef float real;
typedef struct {real Re; real Im;} complex;

#ifndef PI
#define PI	3.14159265358979323846264338327950288
#endif

static int pipef[2];
char *dev_name = "/dev/hbmod_dev";
static int idx = 0;
static complex v[N];
static int fd = -1;
pthread_t tid;

void fft( complex *v, int n, complex *tmp )
{
  if(n>1) {			/* otherwise, do nothing and return */
    int k,m;    complex z, w, *vo, *ve;
    ve = tmp; vo = tmp+n/2;
    for(k=0; k<n/2; k++) {
      ve[k] = v[2*k];
      vo[k] = v[2*k+1];
    }
    fft( ve, n/2, v );		/* FFT on even-indexed elements of v[] */
    fft( vo, n/2, v );		/* FFT on odd-indexed elements of v[] */
    for(m=0; m<n/2; m++) {
      w.Re = cos(2*PI*m/(double)n);
      w.Im = -sin(2*PI*m/(double)n);
      z.Re = w.Re*vo[m].Re - w.Im*vo[m].Im;	/* Re(w*vo[m]) */
      z.Im = w.Re*vo[m].Im + w.Im*vo[m].Re;	/* Im(w*vo[m]) */
      v[  m  ].Re = ve[m].Re + z.Re;
      v[  m  ].Im = ve[m].Im + z.Im;
      v[m+n/2].Re = ve[m].Re - z.Re;
      v[m+n/2].Im = ve[m].Im - z.Im;
    }
  }
  return;
}


void *tF(){ // thread routine

	while(1){
		complex scratch[N];
  		float abs[N];
  		int k;
 		int m;
  		int i;
  		int minIdx, maxIdx;
		int val, bpm;
		// read value from the blocking pipe: the value is read only if the pipe is full, otherwise it will wait
		read(pipef[0], &val, sizeof(val));

 		v[idx].Re = val;
       		v[idx++].Im = 0;

       		if(idx == N){

 			fft( v, N, scratch );
  			// PSD computation
   			for(k=0; k<N; k++) {
  				abs[k] = (50.0/2048)*((v[k].Re*v[k].Re)+(v[k].Im*v[k].Im));
   			}

    			minIdx = (0.5*2048)/50;   // position in the PSD of the spectral line corresponding to 30 bpm
   			maxIdx = 3*2048/50;       // position in the PSD of the spectral line corresponding to 180 bpm

  			// Find the peak in the PSD from 30 bpm to 180 bpm
   			m = minIdx;
    			for(k=minIdx; k<(maxIdx); k++) {
      				if( abs[k] > abs[m] )
  				m = k;
   	 		}
		 	printf("BPM: %d\n", (m) * 60 * 50 / 2048);
		  	idx = 0;
		}
	}
	return NULL;
}

void readAndConvert(){
	char buffer[2];
	// read from the buffer
  	read(fd, &buffer, 2);

	// convert read value
	int val = 0;
	int conv_val, shift;
  	for (int i = 0; i < 2; i++) {
    		conv_val = (int)buffer[i];
		conv_val = conv_val & 0xFF;
    		shift = conv_val << (8 * i);
    		val = val + shift;
  	}

	// write value in the pipe
	write(pipef[1], &val, sizeof(val));
}

int main(void)
{

	 if(pipe(pipef) < 0){
	    fprintf(stderr, "Unable to open the pipe");
	    exit(-1);
	  }

	if(pthread_create(&tid, NULL, tF, NULL)){
		fprintf(stderr, "Unable to start the thread");
		exit(-1);
	}

	if((fd = open(dev_name, O_RDWR)) < 0){
		fprintf(stderr, "Unable to open %s\n", dev_name);
		pthread_cancel(tid);
    		exit(-1);
	}
	
	printf("Start reading...\n");
	while(1){
		readAndConvert();
		usleep(20000); //20 ms
	}
	return 0;  
}
