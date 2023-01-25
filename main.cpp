// created by Zaine Rehman, 2022

#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <chrono>
#include <string>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#include <thread>
#else
#include <unistd.h>
#endif

/**********************************************/
// STANDARD SCREEN W/H: 230x64

// debug mode
static constexpr bool     PM_DEBUG = true ;
// show angle/coordinate debug text
static constexpr bool     PM_DEBUG_TXT = false;


// screen width
static constexpr uint16_t PM_SWIDTH = 400;
// screen height
static constexpr uint16_t PM_SHEIGHT = 100;
// time in between frames, in ms
static constexpr uint16_t PM_FRAMEDELAY = 60;
// base background
static constexpr char     PM_BACKGROUND = ' ';
// friction applied to velocity each frame
static constexpr float    PM_FRICTION = 0;


// blit as actual pixels rather than chars
static constexpr bool PM_TRUEBLIT = true;
// char to blit replacements, only use if PM_TRUEBLIT is true
static const std::vector<std::pair<char,std::array<int,3>>> PM_BLITREPLACE = 
{
	{PM_BACKGROUND , {0  ,0  ,0  }},
	{'W'           , {255,255,255}},
	{'w'           , {150,150,150}},
	{'R'           , {255,0  ,0  }},
	{'r'           , {150,0  ,0  }},
	{'G'           , {0  ,255,0  }},
	{'g'           , {0  ,150,0  }},
	{'B'           , {0  ,0  ,255}},
	{'b'           , {0  ,0  ,150}},
	{'.'           , {75 ,75 ,75 }}
};


// add/subtract random angle on reverse update
static constexpr bool     PM_RANDANG_ADD = false;
// random angle offset ceiling
static constexpr uint16_t PM_ANGLE_OFF_TOP = 10;

/**********************************************/

static constexpr double pi  = 3.1415926535;
static const     double DGR = pi / 180;
// STORES INPUT!
// } is processed as nothing
static char U_INPUT = '}'; 

#ifdef _WIN32
// windows terminal screen
HDC SCREEN_HDC = GetDC(GetConsoleWindow());
#endif

// trail type
typedef struct {
	uint16_t X;
	uint16_t Y;
	// index for blit in trailFade
	uint8_t blit;
	uint16_t offset;
} trail_t;

namespace TermColors 
{
	std::string reset   = "\u001b[0m";

	// normal

	std::string black   = "\u001b[30m";
	std::string red     = "\u001b[31m";
	std::string green   = "\u001b[32m";
	std::string yellow  = "\u001b[33m";
	std::string blue    = "\u001b[34m";
	std::string magenta = "\u001b[35m";
	std::string cyan    = "\u001b[36m";
	std::string white   = "\u001b[37m";

	std::array<std::string,8> colors {black,red,green,yellow,blue,magenta,cyan,white};

	// bright
	// sometimes doesnt work

	std::string BRI_black   = "\u001b[30m;1m";
	std::string BRI_red     = "\u001b[31m;1m";
	std::string BRI_green   = "\u001b[32m;1m";
	std::string BRI_yellow  = "\u001b[33m;1m";
	std::string BRI_blue    = "\u001b[34m;1m";
	std::string BRI_magenta = "\u001b[35m;1m";
	std::string BRI_cyan    = "\u001b[36m;1m";
	std::string BRI_white   = "\u001b[37m;1m";

	// backgrounds

	std::string BACK_black   = "\u001b[40m";
	std::string BACK_red     = "\u001b[41m";
	std::string BACK_green   = "\u001b[42m";
	std::string BACK_yellow  = "\u001b[43m";
	std::string BACK_blue    = "\u001b[44m";
	std::string BACK_magenta = "\u001b[45m";
	std::string BACK_cyan    = "\u001b[46m";
	std::string BACK_white   = "\u001b[47m";

	// bright backgrounds
	// sometimes doesnt work

	std::string BRI_BACK_black   = "\u001b[40m;1m";
	std::string BRI_BACK_red     = "\u001b[41m;1m";
	std::string BRI_BACK_green   = "\u001b[42m;1m";
	std::string BRI_BACK_yellow  = "\u001b[43m;1m";
	std::string BRI_BACK_blue    = "\u001b[44m;1m";
	std::string BRI_BACK_magenta = "\u001b[45m;1m";
	std::string BRI_BACK_cyan    = "\u001b[46m;1m";
	std::string BRI_BACK_white   = "\u001b[47m;1m";

	// others

	std::string BOLD   = "\u001b[1m";
	std::string UNDERL = "\u001b[4m";
	std::string REVRSD = "\u001b[7m";
}

class BetterRand {
public:
	// adds this to random each time, optional
	uint32_t extraRand;
	BetterRand(const uint32_t& ExtraRand = 0) : extraRand(ExtraRand) {};
	uint32_t genRand(const uint8_t& extra = 4, bool resetExtraRand = true, uint8_t resetERextraIt = 2) 
	{
		if (resetExtraRand) extraRand = genRand(resetERextraIt,false);
		// set random to unix time
		auto cool = std::chrono::system_clock::now();
		auto very = 
			(unsigned int) 
			std::chrono::time_point_cast
			<std::chrono::milliseconds>
			(cool).time_since_epoch().count();
		// add random()
		if (extra >= 1) very -= rand();
		// add line number
		if (extra >= 2) very += __LINE__;
		// add an iteration (extra = 2)
		if (extra >= 3) very += genRand(2,false);
		// bitshift right or left based on another iteration
		if (extra >= 4) 
		(genRand(2,false)) % 2 ? very >>= 1 : very <<= 1;
		// subtract an iteration (extra = 4)
		return (very + extraRand);
	}
};

// get keypress input
short getInput() {
	while (true) {
		#ifdef _WIN32
			short keyIn = (short) _getch();
			if (keyIn == 3) {exit(0);}
			if (keyIn == 0) {
				std::cout << "ZERO: " << keyIn << '\n';
			}
			//return keyIn;
			return keyIn;
		#else
			system("stty raw");
	        short keyIn = (short) getchar(); 
	        system("stty cooked");
			if (keyIn == 3) {exit(0);}
			//return keyIn;
			return keyIn;
		#endif
	}
}

void inline Nsleep(float ms) { //milliseconds
	#ifdef _WIN32
	Sleep(ms);
	#else
	usleep(ms*1000);
	#endif
}

#ifdef _WIN32
// no work?
void ShowConsoleCursor(bool showFlag)
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;

    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = showFlag; // set the cursor visibility
    SetConsoleCursorInfo(out, &cursorInfo);
}
#endif

void inline clear(uint8_t type = 3) {
	#ifndef _WIN32
	if (type == 3) type = 2;
	#endif
	switch (type) {
		case 0:
			for (uint16_t i = 0; i <= 72; ++i) std::cout << '\n';
			break;
		case 1:
			system("cls"); // :(
			break;
		case 2:
			std::cout<<"\e[1;1H\e[2J";
			break;
		#ifdef _WIN32
		case 3:
			HANDLE hOut;
			COORD Position;
			hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			Position.X = 0;
			Position.Y = 0;
			SetConsoleCursorPosition(hOut, Position);
			break;
		#endif
	}

}

class PixelMap
{
public:
	uint16_t width;
	uint16_t height;
	char filler;
	std::vector<std::vector<char>> pixels;
	std::vector<std::vector<std::string>> backPixels;

	PixelMap(
		const uint16_t& Width,
		const uint16_t& Height,
		const char&		Filler
	) :
		width(Width),
		height(Height),
		filler(Filler)
	{
		for (uint16_t i = 1; i <= height; ++i) {
			std::vector<char> xbuff;
			for (uint16_t x = 1; x <= width; ++x) {
				xbuff.push_back(filler);
			}
			pixels.push_back(xbuff);
		}
		for (uint16_t i = 1; i <= height; ++i) {
			std::vector<std::string> xbuff;
			for (uint16_t x = 1; x <= width; ++x) {
				xbuff.push_back("");
			}
			backPixels.push_back(xbuff);
		}
	}

	void printPixels()
	{
		std::string toPrint = TermColors::reset;
		for (uint16_t l = 0; l <= PM_SWIDTH; ++l) toPrint += "_";
		toPrint += '\n';
		for (uint16_t i = 0; i < pixels.size(); ++i) {
			toPrint += "|";
			for (uint16_t x = 0; x < pixels[i].size(); ++x) {
				toPrint += backPixels[i][x];
				toPrint += pixels[i][x];
			}
			toPrint += (TermColors::reset + "|\n");
		}
		for (uint16_t k = 0; k <= PM_SWIDTH; ++k) {toPrint += "-";}
		std::cout << toPrint << '\n';
	}

#ifdef _WIN32
	// prints the chars to the terminal as an actual image sort of thing
	void win_blitPixels(
		// how to treat char to RGB conversions
		// { {'O',{255,0,0}} , ... }
		const std::vector< std::pair< char,std::array< int,3 > > >& conversions
	)
	{
		//SetPixel(SCREEN_HDC, X, Y, RGB(255, 255, 255));
		for (uint16_t i = 0; i < pixels.size(); ++i) 
		{
			for (uint16_t x = 0; x < pixels[i].size(); ++x) 
			{
				for (uint8_t k = 0; k < conversions.size(); ++k) 
				{
					if (pixels[i][x] == conversions[k].first) 
					{
						SetPixel(SCREEN_HDC, x, i, RGB(conversions[k].second[0],conversions[k].second[1],conversions[k].second[2]));
					}
				}
			}
		}
	}
#endif

	bool setPixel
	(
		const uint16_t& x,
		const uint16_t& y,
		const char&		blit
	)
	{
		// OUT OF BOUNDS
		if  (  (x > width-1) 
			|| (y > height-1) 
			|| (x < 0) 
			|| (y < 0)) return false;

		pixels[y][x] = blit;
		return true;
	}

	char getPixel
	(
		const uint16_t& x,
		const uint16_t& y
	) 
	{
		// OUT OF BOUNDS
		if  (  (x > width-1) 
			|| (y > height-1) 
			|| (x < 0) 
			|| (y < 0)) return '\n';

		return pixels[y][x];
	}
	
	void resetScreen ()
	{
		for (uint16_t p = 0; p < pixels.size(); ++p) {
			for (uint16_t m = 0; m < pixels[p].size(); ++m) {
				pixels[p][m] = filler;
			}
		}
	}
};

class Ent 
{
public:
	float X, Y;
	float velocity;
	int16_t angle;

	// what each trail will fade into
	std::vector<char> trailFade;
	// holds all the trails
	std::vector<trail_t> trails;
	// standard trail offset
	uint16_t trailOffset;

	std::string color = TermColors::white;

	char blit;
	std::vector<std::vector<char>> sprite = {{}};
	// https://www.alanzucconi.com/2016/02/03/2d-rotations/

	Ent (
		const float& x,
		const float& y,
		const char& Blit,
		
		float 	  V = 0,
		float Angle = 0,

		std::vector
		<char>   TrailFade = {' '},
		uint64_t trailOff  = 0

	) :
		X(x),
		Y(y),
		blit(Blit),
		velocity(V),
		angle(Angle),
		trailFade(TrailFade),
		trailOffset(trailOff)
	{}
	
	// set the sprite image
	void setSprite (std::vector<std::vector<char>> Sprite) {
		sprite = Sprite;
	}

	// update coordinates based on angle and velocity
	void update ()
	{
		//uint16_t oX = X;
		//uint16_t oY = Y;
		X += (velocity * cos(angle*DGR));
		Y += (velocity * sin(angle*DGR));
	}

	// return coordinates for the next update
	std::pair<float,float> nextUpdate 
	(
		const uint16_t& standard,
		const uint16_t& sWidth  = 10,
		const uint16_t& sHeight = 10
	)
	{
		float dX = X, dY = Y;
		if (!standard) {
			dX += (velocity * cos(angle*DGR))/sWidth;
			dY += (velocity * sin(angle*DGR))/sHeight;
			return {dX,dY};
		}
		dX += (velocity * cos(angle*DGR))/standard;
		dY += (velocity * sin(angle*DGR))/standard;
		return {dX,dY};
	}

	void reverseAngle (bool side = false, bool noOffset = false)
	{
		if (side) {
			angle = 180 - angle;
			normalizeAngle();
		}
		else {
			angle = - angle;
			normalizeAngle();
		}
		if (PM_RANDANG_ADD) {
			if (rand() % 2)
				angle += rand() % PM_ANGLE_OFF_TOP;
			else 
				angle -= rand() % PM_ANGLE_OFF_TOP;
		}
	}

	void normalizeAngle ()
	{
		while (angle < 0)    angle += 360;
		while (angle >= 360) angle -= 360;
	}
	
	char getTrailBlit (const trail_t& trail)
	{
		if (trail.blit >= trailFade.size()) return ' ';
		return trailFade[trail.blit];
	}
};

int main() 
{
	std::ios_base::sync_with_stdio(false);
	std::cin.tie(NULL);

	BetterRand rnd {};

	PixelMap pxm {PM_SWIDTH,PM_SHEIGHT,PM_BACKGROUND};
	float midX = (PM_SWIDTH / 2);
	float midY = (PM_SHEIGHT / 2);

	Ent mob1 {midX, midY,'O', 1 , 38 , {'o','.','.'}, 3};
	Ent mob2 {midX, midY,'O', 1 , 204 , {'o','.','.'}, 3};
	Ent mob3 {midX, midY,'O', 1 , 333 , {'o','.','.'}, 3};

	Ent mobR {midX, midY, 'R', 1, 252, {'r','.'}, 2};
	Ent mobG {midX, midY, 'G', 1, 332.33, {'g','.'}, 2};
	Ent mobB {midX, midY, 'B', 1, 112, {'b','.'}, 2};
	Ent mobW {midX, midY, 'W', 1, 29, {'w','.'}, 2};

	std::vector<Ent> mobjs {mobR,mobG,mobB,mobW};

	for (uint8_t i=0; i < mobjs.size(); ++i) 
		mobjs[i].color = TermColors::colors[rnd.genRand() % 8];
	
	#ifdef _WIN32
	std::thread INPUT_THREAD(getInput);
	#endif
	
	while (1) {
		// reset screen to all filler
		pxm.resetScreen();

		//std::cout << U_INPUT << '\n';

		// process all mobile objects
		for (uint8_t i=0; i < mobjs.size(); ++i) {
			// FRICTION!
			if (mobjs[i].velocity > 0) {
				mobjs[i].velocity -= PM_FRICTION;
				if (mobjs[i].velocity < 0) mobjs[i].velocity = 0;
			}
			if (mobjs[i].velocity < 0) {
				mobjs[i].velocity += PM_FRICTION;
				if (mobjs[i].velocity > 0) mobjs[i].velocity = 0;
			}

			if (PM_DEBUG_TXT) 
				std::cout << mobjs[i].blit << " at " << mobjs[i].X 
				          << ',' << mobjs[i].Y << '\n';
			if (PM_DEBUG_TXT) std::cout << "angle: " << mobjs[i].angle << '\n';

			// shitty collision detection incoming
			
			auto next = mobjs[i].nextUpdate(1);
			if (!mobjs[i].sprite[0].size()) {
				if (PM_DEBUG_TXT) std::cout << "next: " << next.first << ',' << next.second << '\n';

				if (next.first > PM_SWIDTH)  // too far right
				{
					if (PM_DEBUG_TXT) std::cout << "right check\n";
					mobjs[i].reverseAngle(true);
				}
				if (next.first < 0)          // too far left
				{
					if (PM_DEBUG_TXT) std::cout << "left check\n";
					mobjs[i].reverseAngle(true);
				}
				if (next.second > PM_SHEIGHT) // too far down
				{
					if (PM_DEBUG_TXT) std::cout << "bottom check\n";
					mobjs[i].reverseAngle();
				}
				if (next.second < 0)          // too far up
				{
					if (PM_DEBUG_TXT) std::cout << "top check\n";
					mobjs[i].reverseAngle();
				}
			}
			
			// coordinates before udpate
			float preX = mobjs[i].X, preY = mobjs[i].Y;

			mobjs[i].update();

			// append trail if blit moved
			if (
				((int) preX != (int) mobjs[i].X) ||
				((int) preY != (int) mobjs[i].Y)
				)
			{
				trail_t Ntrail {
					(uint16_t) preX,(uint16_t) preY,
					0,mobjs[i].trailOffset
				};
				mobjs[i].trails.push_back(Ntrail);
			}
			
			for (uint16_t t = 0; t < mobjs[i].trails.size(); ++t) {
				if (mobjs[i].trails[t].offset) {
					pxm.setPixel(mobjs[i].trails[t].X,
					             mobjs[i].trails[t].Y,
						         mobjs[i].getTrailBlit(mobjs[i].trails[t]));
					mobjs[i].trails[t].offset--;
					mobjs[i].trails[t].blit++;
				}
			}
			// update mobjs
			pxm.setPixel(mobjs[i].X,mobjs[i].Y, mobjs[i].blit);
			if (PM_DEBUG_TXT) std::cout << "ANGLE: " << mobjs[i].angle << '\n';
		}

		#ifdef _WIN32
		if (!PM_TRUEBLIT) 
		#endif	
			pxm.printPixels();
		#ifdef _WIN32
		if (PM_TRUEBLIT) pxm.win_blitPixels(PM_BLITREPLACE);
		#endif
		
		// reset input
		U_INPUT = '}';
		Nsleep(PM_FRAMEDELAY);
		//clear(2);
	}

	return 0;
}
