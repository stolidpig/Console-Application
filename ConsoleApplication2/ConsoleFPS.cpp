#include <iostream>
#include <chrono>
#include <Windows.h>
#include <vector>
#include <algorithm>

using namespace std;

int nScreenWidth = 120;
int nScreenHeight = 40;

int nMapWidth = 16;
int nMapHeight = 16;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 1.0f;
int nPlayerPos() {
	int PlayerPos = (int)fPlayerY * nMapWidth + (int)fPlayerX;
	return PlayerPos;
}

float fFOV = 3.14159 / 4.0;
float fDepth = 12.0f;

int main()
{
    // Create Screen Buffer
	wchar_t* screen = new wchar_t[nScreenWidth*nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;
	
	// Create map
	wstring map;

	map += L"################";
	map += L"#..............#";
	map += L"#..#...#.......#";
	map += L"#..#...#.......#";
	map += L"#..#...#..##...#";
	map += L"#..#...#..##...#";
	map += L"#..#...#..######";
	map += L"#..#...#.......#";
	map += L"#..#...#.......#";
	map += L"#..##########..#";
	map += L"#..#...........#";
	map += L"#..#...........#";
	map += L"#..#...##......#";
	map += L"#..#...####....#";
	map += L"#..#...######..#";
	map += L"################";

	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	// Game Loop
	while (true)
	{
		// Calculate time between frames
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapedTime = elapsedTime.count();

		// Controls

		// Handle CCW Rotation
		if (GetAsyncKeyState((unsigned short)'Q') & 0x8000)
			fPlayerA -= (1.0f) * fElapedTime;
		
		// Handle CW Rotation
		if (GetAsyncKeyState((unsigned short)'E') & 0x8000)
			fPlayerA += (1.0f) * fElapedTime;

		// Move Forwards unless blocked
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			fPlayerX += sinf(fPlayerA) * 2.0f * fElapedTime;
			if (map[nPlayerPos()] == '#')
				fPlayerX -= sinf(fPlayerA) * 2.0f * fElapedTime;
			fPlayerY += cosf(fPlayerA) * 2.0f * fElapedTime;
			if (map[nPlayerPos()] == '#')
				fPlayerY -= cosf(fPlayerA) * 2.0f * fElapedTime;
		}
		// Move Backwards unless blocked
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			fPlayerX -= sinf(fPlayerA) * 2.0f * fElapedTime;
			if (map[nPlayerPos()] == '#')
				fPlayerX += sinf(fPlayerA) * 2.0f * fElapedTime;
			fPlayerY -= cosf(fPlayerA) * 2.0f * fElapedTime;
			if (map[nPlayerPos()] == '#')
				fPlayerY += cosf(fPlayerA) * 2.0f * fElapedTime;
		}
		// Handle strafing
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000) {
			fPlayerX -= cosf(fPlayerA) * 2.0f * fElapedTime;
			if (map[nPlayerPos()] == '#')
				fPlayerX += cosf(fPlayerA) * 2.0f * fElapedTime;
			fPlayerY -= sinf(fPlayerA) * 2.0f * fElapedTime;
			if (map[nPlayerPos()] == '#')
				fPlayerY += sinf(fPlayerA) * 2.0f * fElapedTime;
		}
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000) {
			fPlayerX += cosf(fPlayerA) * 2.0f * fElapedTime;
			if (map[nPlayerPos()] == '#')
				fPlayerX -= cosf(fPlayerA) * 2.0f * fElapedTime;
			fPlayerY += sinf(fPlayerA) * 2.0f * fElapedTime;
			if (map[nPlayerPos()] == '#')
				fPlayerY -= sinf(fPlayerA) * 2.0f * fElapedTime;
		}

		// Create the current frame row at a time
		for (int x = 0; x < nScreenWidth; x++) // For each column
		{
			// Calculate the projected ray angle into world space
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;
			
			// Calulate the projected rays distance to wall
			float fDistanceToWall = 0.0f;
			bool bHitBoundary;
			bool bHitWall = false;
			float fEyeX = sinf(fRayAngle); // Unit vector for ray in player space
			float fEyeY = cosf(fRayAngle); // Direction the player is looking

			while (!bHitWall && fDistanceToWall < fDepth)
			{
				fDistanceToWall += 0.1f;

				// Calculate coordinates of the ray from player
				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				// Test if ray is currently out of bounds
				if (nTestX < 0 || nTestX > nMapWidth || nTestY < 0 || nTestY > nMapHeight) {
					bHitWall = true;	// Just set distance to max depth
					fDistanceToWall = fDepth;
				}
				else {	// We are in bounds
					if (map[nTestY * nMapWidth + nTestX] == '#') {
						// We have hit a wall
						bHitWall = true;
						vector<pair<float, float>> p; // distance to corner, angle between vectors (dot)
						for (int tx = 0; tx < 2; tx++) {
							for (int ty = 0; ty < 2; ty++) // for the 4 corners
							{
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								float d = sqrt(vx*vx + vy * vy);
								float dot = (fEyeX*vx / d) + (fEyeY*vy / d);
								p.push_back(make_pair(d, dot));
							}
						// Sort pairs from closest to farthest
						sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right) { return left.first < right.first; });

						float fBound = 0.01;
						if (acos(p.at(0).second) < fBound) bHitBoundary = true;
						if (acos(p.at(1).second) < fBound) bHitBoundary = true;
						if (acos(p.at(3).second) < fBound) bHitBoundary = true;
						}
					}
				}
			}
			
			// As wall gets further it is smaller - perspective
			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;
			
			// As wall gets further away shading gets darker
			short nShade = ' ';
			if (fDistanceToWall < fDepth / 4.0f)		nShade = 0x2588;		// Very Close
			else if (fDistanceToWall < fDepth / 3.0f)	nShade = 0x2593;
			else if (fDistanceToWall < fDepth / 2.0f)	nShade = 0x2592;
			else if (fDistanceToWall < fDepth)			nShade = 0x2591;		// Too far away
			else										nShade = ' ';

			if (bHitBoundary) nShade = 'I';

			// Apply shade to ceiling, walls and floor
			for (int y = 0; y < nScreenHeight; y++)
			{
				if (y <= nCeiling)
					screen[y*nScreenWidth + x] = ' ';
				else if (y > nCeiling && y <= nFloor)
					screen[y*nScreenWidth + x] = nShade;
				else {
					float b = 1.0f - ((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f);
					if		(b < 0.25)	nShade = 'O';
					else if (b < 0.5)	nShade = 'o';
					else if (b < 0.75)	nShade = '.';
					else				nShade = ' ';
					screen[y*nScreenWidth + x] = nShade;
				}
			}

		} 
		screen[nScreenWidth*nScreenHeight - 1] = '\0';
		// Draw the current frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth*nScreenHeight, { 0,0 }, &dwBytesWritten);
	}
}
