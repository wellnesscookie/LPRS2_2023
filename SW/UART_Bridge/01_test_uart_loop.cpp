
///////////
// Config.

#include "config.h"
#define ITERS 10

///////////

#include "UART.hpp"
#include <iostream>
#include <bitset>
using namespace std;

#define DEBUG(var) do { cout << #var << " = " << var << endl; } while(false)

UART u(
	DEV,
	BAUD_RATE
);

template<typename T, int N>
struct Data {
	static const int n;
	T a[N];
};
template<typename T, int N>
const int Data<T, N>::n = N;
typedef Data<uint8_t, 256> CommonData;
CommonData exp_data;
CommonData obs_data;


int main(int argc, char** argv) {
	for(int i = 0; i < exp_data.n; i++){
		exp_data.a[i] = i+10;
	}
	DEBUG(sizeof(exp_data));
	
	int mismatches = 0;
	for(int iter = 0; iter < ITERS; iter++){
		DEBUG(iter);
		u.write(exp_data);
		u.read(obs_data);
		
		for(int i = 0; i < min(obs_data.n, exp_data.n); i++){
			auto e = exp_data.a[i];
			auto o = obs_data.a[i];
			if(o != e){
				cout << "Mismatch:"
					<< " exp_data[" << i << "] = " 
						<< (int)e 
						<< " 0b" << bitset<8>(e)
					<< " obs_data[" << i << "] = " 
						<< (int)o 
						<< " 0b" << bitset<8>(o)
					<< endl;
				mismatches++;
			}
		}
	}
	if(mismatches == 0){
		cout << "Test PASS!" << endl;
	}else{
		cout << "Test FAIL!" << endl;
	}

	return 0;
}
