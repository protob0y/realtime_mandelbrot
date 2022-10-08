#include <SFML/Graphics.hpp>
#include <iostream>
#include <complex>

#include <vector>

#define SINGLESHOT 0 // run only for one frame?

#define COLORSAT 1.0 // saturation and value for the diverging numbers/blocks
#define COLORVAL 1.0

#define BLOCKSIZE 5
#define X_BLOCKS 200 // how many pixels in x and y, as visible to the user
#define Y_BLOCKS 200
#define MANDEL_ITERATIONS 20 // how many iterations for mandelbrot?

using namespace std;

// converts HSV to RGB color model.
void HSVtoRGB(float h, float s, float v, int* r, int* g, int* b);
// a function that maps a range to a range (0 to +500, value x to i.e. -1 to +1, value y)
void inline mapping(int xbegin, int xend, int xvalue, double ybegin, double yend, double* yvalue);


int main(){
	// per default: 250px in each direction, coordinate origin in the middle (250,250)

	const int sfmlResX = BLOCKSIZE * X_BLOCKS;
	const int sfmlResY = BLOCKSIZE * Y_BLOCKS;

	sf::RenderWindow window(sf::VideoMode(sfmlResX,sfmlResY), "RTmandel");

	cout << "Initing blocks... ";
	// Init all the rectangles (blocks) that make up the final image
	// (!) FIXME (!) // done, using vector. Should be working fine?
	//sf::RectangleShape block[X_BLOCKS][Y_BLOCKS];
	vector<vector<sf::RectangleShape>> block(Y_BLOCKS, vector<sf::RectangleShape>(X_BLOCKS)); // init vector of vectors of type RectShape

	for(int y = 0; y < Y_BLOCKS; y++){ // assign values
		for(int x = 0; x < X_BLOCKS; x++){
			block[x][y].setSize(sf::Vector2f(BLOCKSIZE,BLOCKSIZE));
			block[x][y].setPosition(x*5, y*5);
			block[x][y].setFillColor(sf::Color::Blue);
		}
	}
	cout << "[OK] (" << (sizeof(sf::RectangleShape) * X_BLOCKS * Y_BLOCKS) / 1024 << " kB)" << endl;

	
	// Init a complex num var for each pixel there exists
	//complex<double> com[X_BLOCKS][Y_BLOCKS]; // vector is better?
	cout << "Initing complex vars... ";
	vector<vector<complex<double>>> com(Y_BLOCKS, vector<complex<double>>(X_BLOCKS));
	cout << "[OK] (" << (sizeof(complex<double>) * X_BLOCKS * Y_BLOCKS) / 1024 << " kB)" << endl;

	double comReal;
	double comImag; // these are temporarily used for mapping
	complex<double> z;
	double a; // z and a(bsolute) are part of later calculations, only inited here! (performance)
	bool diverges;
	sf::Color blockColor;
	sf::Color darkColor;
	darkColor.r = 43;
	darkColor.g = 44;
	darkColor.b = 90;


	// Create a lookup table for colors with 360 entries.
	cout << "Creating lookup table... ";
	int lookupcolor[360][3];
	for(int i = 0; i < 360; i++){
		int r;
		int g;
		int b;
		HSVtoRGB(i, COLORSAT, COLORVAL, &r, &g, &b);
		lookupcolor[i][0] = r;
		lookupcolor[i][1] = g;
		lookupcolor[i][2] = b;
	}
	cout << "[OK] (" << (sizeof(int) * 360 * 3) / 1024 << " kB)" << endl;
	
	
	double canvas_xbound[2] = {-1.0, +1.0}; // how big is the coordinate system? // note: [1] has to be always > [0] (!)
	double canvas_ybound[2] = {-1.0, +1.0};
	double canvas_size[2] = {canvas_xbound[1] - canvas_xbound[0], canvas_ybound[1] - canvas_ybound[0]};
	double zoom_pivot[2] = {0,0}; // pivot point, where to orient zoom

	while(window.isOpen()){

		/////////////////
		// GAME LOOP
		/////////////////

		// Event handling
		sf::Event event;
		while(window.pollEvent(event)){
			if(event.type == sf::Event::Closed){
				window.close();
			}
			else if(event.type == sf::Event::MouseWheelScrolled){
				cout << "Mouswheel mov at (" << event.mouseWheelScroll.x << "|" << event.mouseWheelScroll.y << "), ";
				// zooming action happens now
				mapping(0, sfmlResX, event.mouseWheelScroll.x, canvas_xbound[0], canvas_xbound[1], &(zoom_pivot[0]));
				mapping(0, sfmlResY, event.mouseWheelScroll.y, canvas_ybound[0], canvas_ybound[1], &(zoom_pivot[1]));

				cout << "Pivot: " << zoom_pivot[0] << "," << zoom_pivot[1] << ", ";

				if(event.mouseWheelScroll.delta > 0){
					// zoom in
					cout << "zoom IN" << endl;
					canvas_size[0] = 0.95 * canvas_size[0];
					canvas_size[1] = 0.95 * canvas_size[1];
				}
				else if(event.mouseWheelScroll.delta < 0){
					// zoom out
					cout << "zoom OUT" << endl;
					canvas_size[0] = 1.05 * canvas_size[0];
					canvas_size[1] = 1.05 * canvas_size[1];
				}
				canvas_xbound[0] = zoom_pivot[0] - canvas_size[0] / double(2);
				canvas_xbound[1] = zoom_pivot[0] + canvas_size[0] / double(2);

				canvas_ybound[0] = zoom_pivot[1] - canvas_size[1] / double(2);
				canvas_ybound[1] = zoom_pivot[1] + canvas_size[1] / double(2);

				//zooming done
				sf::Mouse::setPosition(sf::Vector2i(sfmlResX / 2, sfmlResY / 2), window);
			}
		}

		window.clear();


		// calculate the mandelbrot
		for(int y = 0; y < Y_BLOCKS; y++){
			for(int x = 0; x < X_BLOCKS; x++){
				// for each pixel:

				// assign complex var at place [x][y]
				//			  +--- old range
				//			  |		 +--- old value
				//			  |		 |      +--- new range
				//			  |		 |      |		   +--- new value
				//      |-----+----| |	|---+---|	   |
				//mapping(0, X_BLOCKS, x, -1.0, 1.0, &comReal);
				//mapping(0, Y_BLOCKS, y, 1.0, -1.0, &comImag);

				mapping(0, X_BLOCKS, x, canvas_xbound[0], canvas_xbound[1], &comReal);
				mapping(0, Y_BLOCKS, y, canvas_ybound[0], canvas_ybound[1], &comImag);

				//cout << "New Frame!" << endl;

				com[x][y].real(comReal);
				com[x][y].imag(comImag);

				//cout << "Complex at " << x << "/" << y << " is " << comReal << " +j* " << comImag << endl;
				//cout << "Complex at " << x << "/" << y << " is " << com[x][y].real() << " +j* " << com[x][y].imag() << endl;

				// reset complex var z to zero
				z.real(0);
				z.imag(0);
				diverges = false;
				// do the recursive calculation of mandelbrot
				for(int iteration = 0; iteration < MANDEL_ITERATIONS; iteration++){
					z = z*z + com[x][y];
					a = abs(z);
					if(a > 1000){
						diverges = true;
						blockColor.r = lookupcolor[iteration * 10][0];
						blockColor.g = lookupcolor[iteration * 10][1];
						blockColor.b = lookupcolor[iteration * 10][2];
						block[x][y].setFillColor(blockColor);
						break;
					}
				}

				if(a <= 1000 && a > 2.0){ //diverges slowly (very rarely)
					diverges = true;
					blockColor.r = lookupcolor[200][0];
					blockColor.g = lookupcolor[200][1];
					blockColor.b = lookupcolor[200][2];
					block[x][y].setFillColor(blockColor);
				}

				switch(diverges){
					case false: // does not diverge
						block[x][y].setFillColor(darkColor); // if part of mandelbrot, paint dark
						break;		
				}
			}
		}

		
		// draw all the blocks
		for(int y = 0; y < Y_BLOCKS; y++){
			for(int x = 0; x < X_BLOCKS; x++){
				window.draw(block[x][y]);
			}
		}

		window.display();

		if(SINGLESHOT){
			cout << "Close?" << endl;
			getchar();
			window.close();
		}

		/////////////////
		// GAME LOOP END
		/////////////////
	}
	return 0;
}


void inline mapping(int xbegin, int xend, int xvalue, double ybegin, double yend, double* yvalue){
	// range x is input range
	// range y is destination range

	int xdelta  = xvalue - xbegin;
	int xrange = xend - xbegin;

	//  yv  = start  +  offsetfraction                   *  newrange
	*yvalue = ybegin + (double(xdelta) / double(xrange)) * (yend - ybegin);
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