#pragma once
#include <vector>
#include <algorithm>
#include "SDL2/SDL.h"
#include "TextureLoader.h"
class Checker
{
public:
	enum class Team {
		red,
		blue
	};
public:
	Checker(int setPosX, int setPosY, Team setTeam);
	static void loadTextures(SDL_Renderer* renderer);
	void draw(SDL_Renderer* renderer, int squareSizePixels);
	void drawPossibleMoves(SDL_Renderer* renderer, int squareSizePixels, std::vector<Checker>& listCheckers, bool canOnlyMove2Squares);
	int checkHowFarCanMoveInAnyDirection(std::vector<Checker>& listCheckers);
	int tryToMoveToPosition(int x, int y, std::vector<Checker>& listCheckers, int& indexCheckerErase, bool canOnlyMove2Squares);
	int getPosX();
	int getPosY();
	Team getTeam();
	static void resetCurrentMoveDirection();
	bool canCaptureInAnyDirection(std::vector<Checker>& listCheckers); //newly added
private:
	bool willCaptureInPath(int startX, int startY, int endX, int endY, int xDir, int yDir, std::vector<Checker>& listCheckers);
	void draw(SDL_Renderer* renderer, int squareSizePixels, int x, int y, bool drawTransparent = false);
	int checkHowFarCanMoveInDirection(int xDirection, int yDirection, std::vector<Checker>& listCheckers);
	Checker* findCheckerAtPosition(int x, int y, std::vector<Checker>& listCheckers);
	int posX, posY;
	Team team;
	bool isAKing = false;
	static SDL_Texture* textureRedKing, * textureRedRegular,
		* textureBlueKing, * textureBlueRegular;
	static int currentMoveDirection; // 0=none, 1=downRight, 2=downLeft, 3=upRight, 4=upLeft
};
