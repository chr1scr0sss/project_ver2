#include "Checker.h"
#include <algorithm> // Required for std::max

SDL_Texture* Checker::textureRedKing = nullptr;
SDL_Texture* Checker::textureRedRegular = nullptr;
SDL_Texture* Checker::textureBlueKing = nullptr;
SDL_Texture* Checker::textureBlueRegular = nullptr;

Checker::Checker(int setPosX, int setPosY, Team setTeam)
    : posX(setPosX), posY(setPosY), team(setTeam), isAKing(false) { // Initialize isAKing
}

void Checker::loadTextures(SDL_Renderer* renderer) {
    textureRedKing = TextureLoader::loadTexture("Checker Red King.bmp", renderer);
    textureRedRegular = TextureLoader::loadTexture("Checker Red Regular.bmp", renderer);
    textureBlueKing = TextureLoader::loadTexture("Checker Blue King.bmp", renderer);
    textureBlueRegular = TextureLoader::loadTexture("Checker Blue Regular.bmp", renderer);
}

void Checker::draw(SDL_Renderer* renderer, int squareSizePixels) {
    draw(renderer, squareSizePixels, posX, posY, false); // Ensure it calls the correct overload
}

void Checker::drawPossibleMoves(SDL_Renderer* renderer, int squareSizePixels, std::vector<Checker>& listCheckers, bool canOnlyMove2Squares) {
    // Check each of the four directions
    int distance = checkHowFarCanMoveInDirection(1, 1, listCheckers);
    if (distance > 0 && (!canOnlyMove2Squares || distance == 2))
        draw(renderer, squareSizePixels, posX + distance, posY + distance, true);

    distance = checkHowFarCanMoveInDirection(1, -1, listCheckers);
    if (distance > 0 && (!canOnlyMove2Squares || distance == 2))
        draw(renderer, squareSizePixels, posX + distance, posY - distance, true);

    distance = checkHowFarCanMoveInDirection(-1, 1, listCheckers);
    if (distance > 0 && (!canOnlyMove2Squares || distance == 2))
        draw(renderer, squareSizePixels, posX - distance, posY + distance, true);

    distance = checkHowFarCanMoveInDirection(-1, -1, listCheckers);
    if (distance > 0 && (!canOnlyMove2Squares || distance == 2))
        draw(renderer, squareSizePixels, posX - distance, posY - distance, true);
}

int Checker::checkHowFarCanMoveInAnyDirection(std::vector<Checker>& listCheckers) {
    return std::max({
        checkHowFarCanMoveInDirection(1, 1, listCheckers),
        checkHowFarCanMoveInDirection(-1, 1, listCheckers),
        checkHowFarCanMoveInDirection(1, -1, listCheckers),
        checkHowFarCanMoveInDirection(-1, -1, listCheckers)
        });
}

int Checker::tryToMoveToPosition(int x, int y, std::vector<Checker>& listCheckers, int& indexCheckerErase, bool canOnlyMove2Squares) {
    if (x == posX && y == posY) return 0; // Prevent self-move

    int xDirection = (x > posX) ? 1 : -1;
    int yDirection = (y > posY) ? 1 : -1;

    int xDistance = abs(x - posX);
    int yDistance = abs(y - posY);

    // Ensure movement is diagonal
    if (xDistance != yDistance) return 0;

    int distance = checkHowFarCanMoveInDirection(xDirection, yDirection, listCheckers);

    if (distance > 0 && (!canOnlyMove2Squares || distance == 2)) {
        // Ensure we're not trying to move further than allowed
        if (xDistance > distance) return 0;

        int xMovable = posX;
        int yMovable = posY;
        bool jumpedOverPiece = false;

        while (xMovable != x || yMovable != y) {
            xMovable += xDirection;
            yMovable += yDirection;

            Checker* checkerSelected = findCheckerAtPosition(xMovable, yMovable, listCheckers);
            if (checkerSelected) {
                if (checkerSelected->team != team) {
                    if (jumpedOverPiece) return 0; // Can't jump over multiple pieces
                    jumpedOverPiece = true;
                    indexCheckerErase = std::distance(listCheckers.begin(),
                        std::find_if(listCheckers.begin(), listCheckers.end(),
                            [xMovable, yMovable](Checker& c) { return c.posX == xMovable && c.posY == yMovable; }));
                }
                else {
                    return 0; // Blocked by friendly piece
                }
            }
        }

        // Move the checker
        posX = x;
        posY = y;

        // If the checker reaches the promotion row, promote it to a king
        if ((team == Team::red && posY == 9) || (team == Team::blue && posY == 0)) {
            isAKing = true;
        }

        // After moving, stop showing potential moves for this checker
        return xDistance; // Return the actual distance moved
    }

    return 0;
}



int Checker::getPosX() { return posX; }
int Checker::getPosY() { return posY; }
Checker::Team Checker::getTeam() { return team; }

void Checker::draw(SDL_Renderer* renderer, int squareSizePixels, int x, int y, bool drawTransparent) {
    SDL_Texture* textureDrawSelected = nullptr;

    switch (team) {
    case Team::red:
        textureDrawSelected = (isAKing ? textureRedKing : textureRedRegular);
        break;
    case Team::blue:
        textureDrawSelected = (isAKing ? textureBlueKing : textureBlueRegular);
        break;
    }

    if (textureDrawSelected) {
        SDL_SetTextureAlphaMod(textureDrawSelected, drawTransparent ? 128 : 255);

        int offsetX = 192;
        int offsetY = 192;
        SDL_Rect rect = {
            offsetX + (x * squareSizePixels),
            offsetY + (y * squareSizePixels),
            squareSizePixels,
            squareSizePixels
        };

        SDL_RenderCopy(renderer, textureDrawSelected, nullptr, &rect);
    }
}

int Checker::checkHowFarCanMoveInDirection(int xDirection, int yDirection, std::vector<Checker>& listCheckers) {
    if (abs(xDirection) != 1 || abs(yDirection) != 1) return 0;

    // Regular checkers can only move forward based on their team
    if (!isAKing && ((team == Team::red && yDirection < 0) || (team == Team::blue && yDirection > 0)))
        return 0;

    int x = posX + xDirection;
    int y = posY + yDirection;
    bool jumpedOverPiece = false;

    // Track how far we can move
    int maxDistance = 0;

    while (x >= 0 && x < 10 && y >= 0 && y < 10) {
        Checker* checkerSelected = findCheckerAtPosition(x, y, listCheckers);

        if (checkerSelected) {
            if (checkerSelected->team == team) break; // Blocked by friendly piece
            if (jumpedOverPiece) break; // Already jumped over a piece

            // Check if we can jump over this piece
            int jumpX = x + xDirection;
            int jumpY = y + yDirection;

            if (jumpX >= 0 && jumpX < 10 && jumpY >= 0 && jumpY < 10 &&
                !findCheckerAtPosition(jumpX, jumpY, listCheckers)) {
                // We can jump over this piece
                maxDistance = isAKing ? (abs(jumpX - posX)) : 2;

                // For kings, we stop after capturing one piece
                // This is the key change to restrict kings to one capture per turn
                return maxDistance;
            }
            break; // Can't jump
        }
        else {
            // Empty square, can potentially move here
            maxDistance = abs(x - posX);

            // If not a king, we can only move 1 square normally (without jumping)
            if (!isAKing && !jumpedOverPiece && maxDistance > 1) {
                return 1;
            }
        }

        x += xDirection;
        y += yDirection;
    }

    // For non-king pieces, they can only move 1 square or jump (2 squares)
    if (!isAKing) {
        return maxDistance > 0 ? std::min(maxDistance, jumpedOverPiece ? 2 : 1) : 0;
    }

    return maxDistance;
}

Checker* Checker::findCheckerAtPosition(int x, int y, std::vector<Checker>& listCheckers) {
    for (auto& checker : listCheckers)
        if (checker.posX == x && checker.posY == y)
            return &checker;
    return nullptr;
}

