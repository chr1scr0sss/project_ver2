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
    //textureRedKing = TextureLoader::loadTexture("Checker Red King.bmp", renderer);
    //textureRedRegular = TextureLoader::loadTexture("Checker Red Regular.bmp", renderer);
    //textureBlueKing = TextureLoader::loadTexture("Checker Blue King.bmp", renderer);
    //textureBlueRegular = TextureLoader::loadTexture("Checker Blue Regular.bmp", renderer);
    textureRedKing = TextureLoader::loadTexture("raspberryking.bmp", renderer);
    textureRedRegular = TextureLoader::loadTexture("raspberry.bmp", renderer);
    textureBlueKing = TextureLoader::loadTexture("blueberryking.bmp", renderer);
    textureBlueRegular = TextureLoader::loadTexture("blueberry.bmp", renderer);
}

void Checker::draw(SDL_Renderer* renderer, int squareSizePixels) {
    draw(renderer, squareSizePixels, posX, posY, false); // Ensure it calls the correct overload
}

void Checker::drawPossibleMoves(SDL_Renderer* renderer, int squareSizePixels, std::vector<Checker>& listCheckers, bool canOnlyMove2Squares) {
    // For each direction, check how far we can move
    for (int xDir : {-1, 1}) {
        for (int yDir : {-1, 1}) {
            int maxDistance = checkHowFarCanMoveInDirection(xDir, yDir, listCheckers);

            if (maxDistance > 0) {
                if (isAKing) {
                    // For kings, draw all valid positions along the diagonal
                    for (int dist = 1; dist <= maxDistance; dist++) {
                        int newX = posX + (xDir * dist);
                        int newY = posY + (yDir * dist);

                        // If in capture-only mode, only show positions that result in captures
                        if (!canOnlyMove2Squares ||
                            willCaptureInPath(posX, posY, newX, newY, xDir, yDir, listCheckers)) {
                            draw(renderer, squareSizePixels, newX, newY, true);
                        }
                    }
                }
                else {
                    // For regular pieces, just show the maximum valid move
                    if (!canOnlyMove2Squares || maxDistance == 2) {
                        draw(renderer, squareSizePixels, posX + (xDir * maxDistance),
                            posY + (yDir * maxDistance), true);
                    }
                }
            }
        }
    }
}

int Checker::checkHowFarCanMoveInAnyDirection(std::vector<Checker>& listCheckers) {
    // Check if the piece can make any valid moves
    int maxDistance = std::max({
        checkHowFarCanMoveInDirection(1, 1, listCheckers),
        checkHowFarCanMoveInDirection(-1, 1, listCheckers),
        checkHowFarCanMoveInDirection(1, -1, listCheckers),
        checkHowFarCanMoveInDirection(-1, -1, listCheckers)
        });

    return maxDistance;
}

// Function to determine if moving from one position to another will result in a capture
bool Checker::willCaptureInPath(int startX, int startY, int endX, int endY,
    int xDir, int yDir, std::vector<Checker>& listCheckers) {
    int x = startX;
    int y = startY;
    bool foundOpponent = false;

    while (x != endX || y != endY) {
        x += xDir;
        y += yDir;

        Checker* checkerSelected = findCheckerAtPosition(x, y, listCheckers);
        if (checkerSelected) {
            if (checkerSelected->team != team) {
                // Found an opponent piece
                foundOpponent = true;
            }
            else {
                // Found a friendly piece - can't move through it
                return false;
            }
        }
        else if (foundOpponent) {
            // Found an empty square after an opponent - this is a capture
            return true;
        }
    }

    return false;
}

int Checker::tryToMoveToPosition(int x, int y, std::vector<Checker>& listCheckers, int& indexCheckerErase, bool canOnlyMove2Squares) {
    if (x == posX && y == posY) return 0; // Prevent self-move

    int xDirection = (x > posX) ? 1 : -1;
    int yDirection = (y > posY) ? 1 : -1;
    int xDistance = abs(x - posX);
    int yDistance = abs(y - posY);

    // Ensure movement is diagonal
    if (xDistance != yDistance) return 0;

    int maxAllowedDistance = checkHowFarCanMoveInDirection(xDirection, yDirection, listCheckers);

    // Check if the move is within allowed distance
    if (maxAllowedDistance <= 0 || xDistance > maxAllowedDistance) return 0;

    // If in capture-only mode, ensure we're making a capture
    if (canOnlyMove2Squares) {
        bool willCapture = willCaptureInPath(posX, posY, x, y, xDirection, yDirection, listCheckers);
        if (!willCapture) return 0;
    }

    int xMovable = posX;
    int yMovable = posY;
    bool jumpedOverPiece = false;

    // Check the path for obstacles and captures
    while (xMovable != x || yMovable != y) {
        xMovable += xDirection;
        yMovable += yDirection;

        Checker* checkerSelected = findCheckerAtPosition(xMovable, yMovable, listCheckers);
        if (checkerSelected) {
            if (checkerSelected->team != team) {
                if (jumpedOverPiece) return 0; // Can't jump over multiple pieces in a single move

                // Found opponent's piece - check next position
                int nextX = xMovable + xDirection;
                int nextY = yMovable + yDirection;

                // Make sure we're not going beyond the target position
                if ((xDirection > 0 && nextX > x) || (xDirection < 0 && nextX < x) ||
                    (yDirection > 0 && nextY > y) || (yDirection < 0 && nextY < y)) {
                    return 0; // Would go past the target
                }

                // Ensure the landing square is valid
                if (findCheckerAtPosition(nextX, nextY, listCheckers) == nullptr) {
                    jumpedOverPiece = true;
                    indexCheckerErase = std::distance(listCheckers.begin(),
                        std::find_if(listCheckers.begin(), listCheckers.end(),
                            [xMovable, yMovable](Checker& c) {
                                return c.posX == xMovable && c.posY == yMovable; }));
                }
                else {
                    return 0; // Can't jump if landing square is occupied
                }
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

    // Return 2 if we jumped over a piece, otherwise return the distance
    return jumpedOverPiece ? 2 : xDistance;
}

int Checker::getPosX() { return posX; }
int Checker::getPosY() { return posY; }
Checker::Team Checker::getTeam() { return team; }

// Fixed draw function for consistent rendering of both pieces and preview
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
        // Set transparency level - 128 for preview (half transparent), 255 for actual pieces
        SDL_SetTextureAlphaMod(textureDrawSelected, drawTransparent ? 128 : 255);

        // Calculate the position with offset
        int offsetX = 192;
        int offsetY = 192;
        SDL_Rect rect = {
            offsetX + (x * squareSizePixels),
            offsetY + (y * squareSizePixels),
            squareSizePixels,
            squareSizePixels
        };

        // Render the texture
        SDL_RenderCopy(renderer, textureDrawSelected, nullptr, &rect);

        // Reset alpha to full opacity for other renders
        if (drawTransparent) {
            SDL_SetTextureAlphaMod(textureDrawSelected, 255);
        }
    }
}

// Fixed checkHowFarCanMoveInDirection function with corrected king logic
int Checker::checkHowFarCanMoveInDirection(int xDirection, int yDirection, std::vector<Checker>& listCheckers) {
    if (abs(xDirection) != 1 || abs(yDirection) != 1) return 0;

    // Regular checkers can only move forward based on their team
    if (!isAKing && ((team == Team::red && yDirection < 0) || (team == Team::blue && yDirection > 0)))
        return 0;

    int x = posX + xDirection;
    int y = posY + yDirection;

    // If out of bounds, return 0
    if (x < 0 || x >= 10 || y < 0 || y >= 10) return 0;

    // Check first position
    Checker* firstChecker = findCheckerAtPosition(x, y, listCheckers);

    if (firstChecker) {
        // First position is occupied
        if (firstChecker->team == team) {
            // Blocked by friendly piece
            return 0;
        }
        else {
            // Found opponent's piece - check if we can capture
            int jumpX = x + xDirection;
            int jumpY = y + yDirection;

            // Check if the landing square is valid
            if (jumpX >= 0 && jumpX < 10 && jumpY >= 0 && jumpY < 10 &&
                !findCheckerAtPosition(jumpX, jumpY, listCheckers)) {
                // Can capture - kings stop after capturing just like regular pieces
                return 2;
            }
            return 0;
        }
    }

    // For kings, check how far we can move without capturing
    if (isAKing) {
        int maxDistance = 1; // We can at least move 1 square

        // Continue checking empty squares along the diagonal
        while (true) {
            x += xDirection;
            y += yDirection;

            if (x < 0 || x >= 10 || y < 0 || y >= 10) break; // Check bounds

            Checker* nextChecker = findCheckerAtPosition(x, y, listCheckers);
            if (nextChecker) {
                // Found a piece - if opponent's piece, check for capture
                if (nextChecker->team != team) {
                    int jumpX = x + xDirection;
                    int jumpY = y + yDirection;

                    // Check if we can capture
                    if (jumpX >= 0 && jumpX < 10 && jumpY >= 0 && jumpY < 10 &&
                        !findCheckerAtPosition(jumpX, jumpY, listCheckers)) {
                        // Kings can capture but stop at the capturing position (jumpX, jumpY)
                        // Return the distance to the capture landing position
                        return maxDistance + 2; // +2 represents jumping over the opponent piece
                    }
                }
                // Blocked by any piece (opponent with no capture or friendly)
                break;
            }

            // Empty square - increase maximum distance
            maxDistance++;
        }

        return maxDistance;
    }
    else {
        // Regular pieces can only move 1 square without capturing
        return 1;
    }
}

Checker* Checker::findCheckerAtPosition(int x, int y, std::vector<Checker>& listCheckers) {
    for (auto& checker : listCheckers)
        if (checker.posX == x && checker.posY == y)
            return &checker;
    return nullptr;
}
