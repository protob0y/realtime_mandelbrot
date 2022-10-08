#include <iostream>
#include <math.h>


using namespace std;


void HSVtoRGB(float h, float s, float v, int* r, int* g, int* b);


int main(){
	int r,g,b;

	HSVtoRGB(200.0, 1.0, 1.0, &r, &g, &b);

	cout << "The result is: R=" << r << " G=" << g << " B=" << b << endl;

	return 0;
}


void HSVtoRGB(float h, float s, float v, int* r, int* g, int* b){
	// h can go from 0 to 360 (degrees)
	// s can go from 0.0 to 1.0
	// v can go from 0.0 to 1.0

	float c = v * s;
	float x = c * (1.0 - abs(fmod(h/60.0, 2) - 1.0));
	float m = v - c;

	float ri, gi, bi;

	if(h >= 0.0 && h < 60.0){
		ri = c;
		gi = x;
		bi = 0.0;
	}
	else if(h >= 60 && h < 120){
		ri = x;
		gi = c;
		bi = 0.0;
	}
	else if(h >= 120 && h < 180){
		ri = 0.0;
		gi = c;
		bi = x;
	}
	else if(h >= 180 && h < 240){
		ri = 0.0;
		gi = x;
		bi = c;
	}
	else if(h >= 240 && h < 300){
		ri = x;
		gi = 0.0;
		bi = c;
	}
	else{
		ri = c;
		gi = 0.0;
		bi = x;
	}

	*r = int((ri + m) * 255.0);
	*g = int((gi + m) * 255.0);
	*b = int((bi + m) * 255.0);
}