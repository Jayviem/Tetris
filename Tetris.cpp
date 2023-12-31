#include <iostream>
#include <thread>
#include <vector>
using namespace std;

#include <stdio.h>
#include <Windows.h>

wstring tetromino[7];
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char* pField = nullptr;

int nScreenWidth = 120;	// Console Screen Size X (columns)
int nScreenHeight = 30;	// Console Screen Size Y (rows)

int Rotate(int px, int py, int r)
{
	switch (r % 4)
	{
	case 0: return py * 4 + px;				// 0 degrees
	case 1: return 12 + py - (px * 4);		// 90 degrees
	case 2: return 15 - (py * 4) - px;		// 180 degrees
	case 3: return 3 - py + (px * 4);		// 270 degrees
	}
	return 0;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
	for (int px = 0; px < 4; px++)
		for (int py = 0; py < 4; py++)
		{
			// Get index into piece
			int pi = Rotate(px, py, nRotation);

			// Get index into field
			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			if (nPosX + px >= 0 && nPosX + px < nFieldWidth)
			{
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
				{
					if (tetromino[nTetromino][pi] == L'X' && pField[fi] != 0)
						return false; // fail on first hit
				}
			}
		}


	return true;
}

int main()
{
	// Create shapes (assets)
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");

	tetromino[1].append(L"..X.");
	tetromino[1].append(L".XX.");
	tetromino[1].append(L".X..");
	tetromino[1].append(L"....");

	tetromino[2].append(L".X..");
	tetromino[2].append(L".XX.");
	tetromino[2].append(L"..X.");
	tetromino[2].append(L"....");

	tetromino[3].append(L"XXX.");
	tetromino[3].append(L".X..");
	tetromino[3].append(L"....");
	tetromino[3].append(L"....");

	tetromino[4].append(L"....");
	tetromino[4].append(L"X...");
	tetromino[4].append(L"X...");
	tetromino[4].append(L"XX..");

	tetromino[5].append(L"....");
	tetromino[5].append(L"...X");
	tetromino[5].append(L"...X");
	tetromino[5].append(L"..XX");

	tetromino[6].append(L".XX.");
	tetromino[6].append(L".XX.");
	tetromino[6].append(L"....");
	tetromino[6].append(L"....");

	pField = new unsigned char[nFieldWidth * nFieldHeight]; // create play field buffer
	for (int x = 0; x < nFieldWidth; x++) // Board limit (boundary)
		for (int y = 0; y < nFieldHeight; y++)
			pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

	//console screen
	wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	//game loop / logic stuff
	bool bGameOver = false;

	int nCurrentPiece = 0;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;
	int nCurrentY = 0;

	bool bKey[4]; // keys input
	bool bRotateHold = false; // control crazy rotation due to timer reset

	int nSpeed = 20; // game speed
	int nSpeedCounter = 0; // counting game ticks
	bool bForceDown = false; // force the piece down
	int nPieceCount = 0; // increase speed/dificulty
	int nScore = 0; // count score

	vector<int> vLines; // vector para eliminar a linha horizontal completa

	while (!bGameOver)
	{
		// GAME TIMING ======================================================================================================
		this_thread::sleep_for(50ms); // game tick
		nSpeedCounter++; // counting game ticks
		bForceDown = (nSpeedCounter == nSpeed); // force the piece down

		
		// INPUT ============================================================================================================
		for (int k = 0; k < 4; k++)								// create commands -> Right arrow x27	-	Left arrow x25	-	Down arrow x28	-	Z key
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;

		// GAME LOGIC =======================================================================================================
		if (bKey[1]) // move to the left
		{
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY))
			{
				nCurrentX = nCurrentX - 1;
			}
		}
		if (bKey[0]) // move to the right
		{
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY))
			{
				nCurrentX = nCurrentX + 1;
			}
		}
		if (bKey[2]) // move down
		{
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY+ 1))
			{
				nCurrentY = nCurrentY + 1;
			}
		}
			// it could be like:
			/*
			nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
			nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;		
			nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;
			*/
		if (bKey[3]) // rotate with hold not infinite rotation
		{
			nCurrentRotation += (bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHold = false;
		}
		else
			bRotateHold = true;


		if (bForceDown) // forcing pieces down
		{
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
				nCurrentY++; // if it can go down, keep going
			else
			{
				//lock the current piece in the field when it can't go further down
				for (int px = 0; px < 4; px++)
					for (int py = 0; py < 4; py++)
						if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.')
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;

				nPieceCount++; // increase speed/dificulty
				if (nPieceCount % 10 == 0)
					if (nSpeed >= 10) nSpeed--;

				// check if there are full horizontal lines
				for (int py = 0; py < 4; py++)
					if (nCurrentY + py < nFieldHeight - 1)
					{
						bool bLine = true;
						for (int px = 1; px < nFieldWidth - 1; px++)
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

						if (bLine)
						{
							// Remove Line, set to =
							for (int px = 1; px < nFieldWidth - 1; px++)
								pField[(nCurrentY + py) * nFieldWidth + px] = 8; // preencher a linha e meter um "=" nos caracteres preenchidos

							vLines.push_back(nCurrentY + py); // eliminar a linha horizontal completa
						}
					}

				nScore += 25; // count score
				if (!vLines.empty()) nScore += (1 << vLines.size()) * 100; // give 100 points for one line

				// choose next piece
				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;

				// if piece does not fit, game over
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}

			nSpeedCounter = 0;
		}

		// RENDER OUTPUT ====================================================================================================
		



		// draw field
		for (int x = 0; x < nFieldWidth; x++)
			for (int y = 0; y < nFieldHeight; y++)
				screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];

		// draw current piece
		for (int px = 0; px < 4; px++)
			for (int py = 0; py < 4; py++)
				if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
					screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;

		// draw score
		swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore); // show the current score

		// animation of the horizontal line completed
		if (!vLines.empty())
		{
			// display animation frame with a delay on purpose
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms); // delay a bit

			for (auto& v : vLines) // clear the line
				for (int px = 1; px < nFieldWidth - 1; px++)
				{
					for (int py = v; py > 0; py--)
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					pField[px] = 0;
				}

			vLines.clear(); //clear line(s)
		}

		// display frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth* nScreenHeight, { 0,0 }, & dwBytesWritten);
	}

	// GAME OVER
	CloseHandle(hConsole);
	cout << "GAME OVER \n\n Score: " << nScore << endl;
	system("pause");
}

// credits: Javidx9 on https://www.youtube.com/watch?v=8OK8_tHeCIA
