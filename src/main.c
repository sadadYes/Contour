#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <time.h>

#define MAP_WIDTH 20
#define MAP_HEIGHT 20
#define ENEMY_SPEED 5 // Enemy movement speed in movements per second
#define MAX_ENEMIES 5
#define SPAWN_DISTANCE 2 // Maximum distance from player to spawn enemy

typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    Position playerPos;
    Position enemyPos[MAX_ENEMIES];
    int playerHealth;
    int enemyHealth[MAX_ENEMIES];
    int immunityFrames;
    int numEnemies;
    clock_t lastSpawnTime;
    clock_t lastAttackTime; // Attack cooldown
} GameState;

typedef enum {
    MAIN_MENU,
    GAMEPLAY,
    GAME_OVER
} GameScreen;

void handleInput(GameState* gameState, GameScreen* currentScreen) {
    if (*currentScreen == MAIN_MENU) {
        if (IsKeyPressed(KEY_ENTER)) {
            *currentScreen = GAMEPLAY;
        } else if (IsKeyPressed(KEY_ESCAPE)) {
            CloseWindow();
            exit(0);
        }
    } else if (*currentScreen == GAMEPLAY) {
        if (IsKeyPressed(KEY_UP)) {
            gameState->playerPos.y--;
        } else if (IsKeyPressed(KEY_DOWN)) {
            gameState->playerPos.y++;
        } else if (IsKeyPressed(KEY_LEFT)) {
            gameState->playerPos.x--;
        } else if (IsKeyPressed(KEY_RIGHT)) {
            gameState->playerPos.x++;
        } else if (IsKeyPressed(KEY_SPACE)) { // Player attacks nearby enemies
            // Check if attack cooldown has passed
            clock_t currentTime = clock();
            double timeElapsed = (double)(currentTime - gameState->lastAttackTime) / CLOCKS_PER_SEC;
            if (timeElapsed >= 1.0) {
                // Deal 1 damage to nearby enemies
                for (int i = 0; i < gameState->numEnemies; i++) {
                    int playerX = gameState->playerPos.x;
                    int playerY = gameState->playerPos.y;
                    int enemyX = gameState->enemyPos[i].x;
                    int enemyY = gameState->enemyPos[i].y;

                    if (abs(playerX - enemyX) <= 1 && abs(playerY - enemyY) <= 1) {
                        gameState->enemyHealth[i] -= 1;
                        printf("Player attacked enemy! Enemy health: %d\n", gameState->enemyHealth[i]);

                        if (gameState->enemyHealth[i] <= 0) {
                            // Enemy is defeated
                            gameState->numEnemies--;
                            gameState->enemyPos[i] = gameState->enemyPos[gameState->numEnemies];
                            gameState->enemyHealth[i] = gameState->enemyHealth[gameState->numEnemies];
                        }
                    }
                }

                gameState->lastAttackTime = currentTime;
            }
        }
    } else if (*currentScreen == GAME_OVER) {
        if (IsKeyPressed(KEY_R)) {
            // Restart game
            *currentScreen = GAMEPLAY;
            gameState->playerHealth = 20;
            gameState->playerPos.x = 0;
            gameState->playerPos.y = 0;
            gameState->numEnemies = 0;
            gameState->lastSpawnTime = clock();
        } else if (IsKeyPressed(KEY_M)) {
            // Exit to main menu
            *currentScreen = MAIN_MENU;
            gameState->playerHealth = 20;
            gameState->playerPos.x = 0;
            gameState->playerPos.y = 0;
            gameState->numEnemies = 0;
            gameState->lastSpawnTime = clock();
        } else if (IsKeyPressed(KEY_Q)) {
            // Quit game
            CloseWindow();
            exit(0);
        }
    }
}

void spawnEnemy(GameState* gameState) {
    if (gameState->numEnemies < MAX_ENEMIES) {
        int enemyIndex = gameState->numEnemies;
        int playerX = gameState->playerPos.x;
        int playerY = gameState->playerPos.y;
        int spawnX, spawnY;

        do {
            spawnX = playerX + (rand() % (2 * SPAWN_DISTANCE + 1) - SPAWN_DISTANCE);
            spawnY = playerY + (rand() % (2 * SPAWN_DISTANCE + 1) - SPAWN_DISTANCE);
        } while ((spawnX == playerX && spawnY == playerY) || spawnX < 0 || spawnX >= MAP_WIDTH || spawnY < 0 || spawnY >= MAP_HEIGHT);

        gameState->enemyPos[enemyIndex].x = spawnX;
        gameState->enemyPos[enemyIndex].y = spawnY;
        gameState->enemyHealth[enemyIndex] = 5;
        gameState->numEnemies++;
    }
}

void updateGame(GameState* gameState, GameScreen* currentScreen) {
    // Update player position based on input
    handleInput(gameState, currentScreen);

    if (*currentScreen == GAMEPLAY) {
        // Update enemy position
        static clock_t lastMoveTime = 0;
        clock_t currentTime = clock();
        double timeElapsed = (double)(currentTime - lastMoveTime) / CLOCKS_PER_SEC;

        if (timeElapsed >= 1.0 / ENEMY_SPEED) {
            for (int i = 0; i < gameState->numEnemies; i++) {
                int playerX = gameState->playerPos.x;
                int playerY = gameState->playerPos.y;
                int enemyX = gameState->enemyPos[i].x;
                int enemyY = gameState->enemyPos[i].y;
                int direction;

                if (abs(playerX - enemyX) <= SPAWN_DISTANCE && abs(playerY - enemyY) <= SPAWN_DISTANCE) {
                    // Player is within range, move towards the player
                    if (playerX < enemyX) {
                        direction = 2; // Left
                    } else if (playerX > enemyX) {
                        direction = 3; // Right
                    } else if (playerY < enemyY) {
                        direction = 0; // Up
                    } else {
                        direction = 1; // Down
                    }
                } else {
                    // Player is not within range, continue roaming
                    direction = rand() % 4; // Randomly choose a direction (0: up, 1: down, 2: left, 3: right)
                }

                switch (direction) {
                    case 0: // Up
                        gameState->enemyPos[i].y--;
                        break;
                    case 1: // Down
                        gameState->enemyPos[i].y++;
                        break;
                    case 2: // Left
                        gameState->enemyPos[i].x--;
                        break;
                    case 3: // Right
                        gameState->enemyPos[i].x++;
                        break;
                }
            }

            lastMoveTime = currentTime;
        }

        // Check for collision between player and enemy
        for (int i = 0; i < gameState->numEnemies; i++) {
            if (gameState->playerPos.x == gameState->enemyPos[i].x && gameState->playerPos.y == gameState->enemyPos[i].y) {
                if (gameState->immunityFrames == 0) {
                    gameState->playerHealth--;
                    gameState->immunityFrames = 60; // 1 second immunity
                    printf("Player collided with enemy! Player health: %d\n", gameState->playerHealth);

                    if (gameState->playerHealth <= 0) {
                        *currentScreen = GAME_OVER;
                    }
                }
            }
        }

        // Decrease immunity frames
        if (gameState->immunityFrames > 0) {
            gameState->immunityFrames--;
        }

        // Spawn enemy
        clock_t currentTimeSpawn = clock();
        double timeElapsedSpawn = (double)(currentTimeSpawn - gameState->lastSpawnTime) / CLOCKS_PER_SEC;

        if (timeElapsedSpawn >= (double)(rand() % 30 + 1)) {
            spawnEnemy(gameState);
            gameState->lastSpawnTime = currentTimeSpawn;
        }
    }
}

void drawGame(GameState* gameState, GameScreen* currentScreen) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    if (*currentScreen == MAIN_MENU) {
        // Draw main menu
        DrawText("Contour", GetScreenWidth() / 2 - MeasureText("Contour", 40) / 2, GetScreenHeight() / 2 - 40, 40, BLACK);
        DrawText("Press ENTER to start", GetScreenWidth() / 2 - MeasureText("Press ENTER to start", 20) / 2, GetScreenHeight() / 2, 20, BLACK);
        DrawText("Press ESC to quit", GetScreenWidth() / 2 - MeasureText("Press ESC to quit", 20) / 2, GetScreenHeight() / 2 + 40, 20, BLACK);
    } else if (*currentScreen == GAMEPLAY) {
        // Draw player
        DrawRectangle(gameState->playerPos.x * 50, gameState->playerPos.y * 50, 50, 50, BLUE);

        // Draw player health bar
        DrawRectangle(0, 0, gameState->playerHealth * 10, 10, GREEN);

        // Draw enemies
        for (int i = 0; i < gameState->numEnemies; i++) {
            DrawRectangle(gameState->enemyPos[i].x * 50, gameState->enemyPos[i].y * 50, 50, 50, RED);

            // Draw enemy health bar
            DrawRectangle(gameState->enemyPos[i].x * 50, gameState->enemyPos[i].y * 50 - 20, gameState->enemyHealth[i] * 10, 10, YELLOW);
        }
    } else if (*currentScreen == GAME_OVER) {
        // Draw game over screen
        DrawText("Game Over", GetScreenWidth() / 2 - MeasureText("Game Over", 40) / 2, GetScreenHeight() / 2 - 40, 40, BLACK);
        DrawText("Press R to restart game", GetScreenWidth() / 2 - MeasureText("Press R to restart game", 20) / 2, GetScreenHeight() / 2, 20, BLACK);
        DrawText("Press M to exit to main menu", GetScreenWidth() / 2 - MeasureText("Press M to exit to main menu", 20) / 2, GetScreenHeight() / 2 + 40, 20, BLACK);
        DrawText("Press Q to quit game", GetScreenWidth() / 2 - MeasureText("Press Q to quit game", 20) / 2, GetScreenHeight() / 2 + 80, 20, BLACK);
    }

    EndDrawing();
}

int main() {
    // Initialization
    const int screenWidth = MAP_WIDTH * 50;
    const int screenHeight = MAP_HEIGHT * 50;

    InitWindow(screenWidth, screenHeight, "Contour");

    GameScreen currentScreen = MAIN_MENU;

    GameState gameState;
    gameState.playerPos.x = 0;
    gameState.playerPos.y = 0;
    gameState.playerHealth = 20;
    gameState.immunityFrames = 0;
    gameState.numEnemies = 0;
    gameState.lastSpawnTime = clock();
    gameState.lastAttackTime = clock(); // Initialize attack cooldown

    // Main game loop
    while (!WindowShouldClose()) {
        // Update
        handleInput(&gameState, &currentScreen);
        updateGame(&gameState, &currentScreen);

        // Draw
        drawGame(&gameState, &currentScreen);
    }

    // De-Initialization
    CloseWindow();

    return 0;
}
