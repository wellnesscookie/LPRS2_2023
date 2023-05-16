
///////////
// Config.

#include "config.h"
#define N_SAMPLES (1 << LOG2_SAMPLES)

///////////

#include "UART.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <bitset>
using namespace std;


#define DEBUG(var) do { cout << #var << " = " << var << endl; } while(false)

UART u(
	DEV,
	BAUD_RATE
);

typedef uint16_t sample_t;
sample_t sample_buffer[N_SAMPLES];

int main(int argc, char** argv) {
	//DEBUG(sizeof(sample_buffer));
	
	//u.write(uint8_t(1)); // Command to start sending.
	
	u.read(sample_buffer);
	
	ofstream of("log.tsv", ios::out);
	of << setprecision(10);
	of << "t" << '\t' << "adc" << endl;
	
	for(int i = 0; i < N_SAMPLES; i++){
		const double T = 1.0/F_SMPL;
		double t = i*T;
		uint16_t sample = sample_buffer[i];
		of << t << '\t' << sample << endl;
	}
	
	return 0;
}
