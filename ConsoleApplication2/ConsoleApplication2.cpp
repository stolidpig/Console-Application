#include <iostream>
#include <chrono>
#include <Windows.h>

using namespace std;

int nScreenWidth = 120;
int nScreenHeight = 40;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

int nMapWidth = 16;
int nMapHeight = 16;

float fFOV = 3.14159 / 4.0;
float fDepth = 16.0f;

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
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..######......#";
	map += L"#......##......#";
	map += L"#......####....#";
	map += L"#......######..#";
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
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			fPlayerA -= (1.0f * fElapedTime);
		
		// Handle CW Rotation
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			fPlayerA += (1.0f) * fElapedTime;

		// Move Forwards
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
			fPlayerX += sinf(fPlayerA) * 2.0f * fElapedTime;
			fPlayerY += cosf(fPlayerA) * 2.0f * fElapedTime;

		// Move Backwards
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
			fPlayerX -= sinf(fPlayerA) * 2.0f * fElapedTime;
			fPlayerY -= cosf(fPlayerA) * 2.0f * fElapedTime;
		

		// Create the current frame screen row at a time
		for (int x = 0; x < nScreenWidth; x++) // For each column
		{
			// Calculate the projected ray angle into world space
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;


			// Calulate the projected rays distance to wall
			float fDistanceToWall = 0.0f;
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
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY > nMapHeight) {
					bHitWall = true;	// Just set distance to max depth
					fDistanceToWall = fDepth;
				}
				else {	// We are in bounds
					if (map[nTestY * nMapWidth + nTestX] == '#') // We have hit a wall
						bHitWall = true;
				}
			}
			
			// As wall gets further it is smaller - perspective
			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;

			short nShade = ' ';

			// As wall gets further away shading gets darker
			if (fDistanceToWall < fDepth / 4.0f)		nShade = 0x2588;		// Very Close
			else if (fDistanceToWall < fDepth / 3.0f)	nShade = 0x2593;
			else if (fDistanceToWall < fDepth / 2.0f)	nShade = 0x2592;
			else if (fDistanceToWall < fDepth)			nShade = 0x2591;		// Too far away
			else										nShade = ' ';

			// Apply shade to ceiling, walls and floor
			for (int y = 0; y < nScreenHeight; y++)
			{
				if (y <= nCeiling)
					screen[y*nScreenWidth + x] = ' ';
				else if (y > nCeiling && y <= nFloor)
					screen[y*nScreenWidth + x] = nShade;
				else
					screen[y*nScreenWidth + x] = '.';
			}

		} 

		screen[nScreenWidth*nScreenHeight - 1] = '\0';

		// Draw the current frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth*nScreenHeight, { 0,0 }, &dwBytesWritten);

	}


}
