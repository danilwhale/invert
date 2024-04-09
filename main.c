#define GRAPHICS_API_OPENGL_11

#include "raylib.h"
#include "raymath.h"

#define BLOCKAGE_AMOUNT 10
#define EARTHQUAKE_PERIOD 1.0

typedef struct {
    int x;
    bool isInverted;
} Blockage;

Color invertColor(Color color);

Color lerpColor(Color a, Color b, float time);

void createNewBlockage(int index);

void updatePlayer();

void drawPlayer();

void restartGame();

void updateGame();

void drawGame();

void drawTextCentered(const char *text, int fontSize, int y);

bool isAnyKeyPressed();

Camera2D camera = {0};

bool isInverted = false;
Color accent;
Color target;

Blockage blockages[BLOCKAGE_AMOUNT];

bool dead = false;
int score = 0;

double lastEarthquakeTime = 0.0;

int main() {
    InitWindow(800, 600, "invert");
    SetTargetFPS(60);

    camera.offset = (Vector2) {
            (float) GetScreenWidth() / 2,
            (float) GetScreenHeight() / 2
    };
    camera.target = (Vector2) {
            (float) GetScreenWidth() / 2,
            (float) GetScreenHeight() / 2
    };
    camera.zoom = 1.0f;

    restartGame();

    while (!WindowShouldClose()) {
        if (!dead) updateGame();
        else if (isAnyKeyPressed()) {
            restartGame();
        }

        BeginDrawing();

        drawGame();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

Color invertColor(Color color) {
    return (Color) {255 - color.r, 255 - color.g, 255 - color.b, 255};
}

Color lerpColor(Color a, Color b, float time) {
    return ColorFromNormalized(
            (Vector4) {
                    Lerp((float) a.r / 255.0f, (float) b.r / 255.0f, time),
                    Lerp((float) a.g / 255.0f, (float) b.g / 255.0f, time),
                    Lerp((float) a.b / 255.0f, (float) b.b / 255.0f, time),
                    1.0f
            });
}

void createNewBlockage(int index) {
    Blockage *blockage = &blockages[index];
    blockage->x = GetScreenWidth() + index * 150;
    blockage->isInverted = GetRandomValue(0, 1) > 0;

    for (int i = 0; i < BLOCKAGE_AMOUNT; i++) {
        Blockage other = blockages[i];
        if (fabsf((float) other.x - (float) blockage->x) < 25.0f &&
            ((blockage->isInverted && !other.isInverted) ||
             (!blockage->isInverted && other.isInverted))) {
            blockage->x += 150;
        }
    }
}

void updatePlayer() {
    int quarter = GetScreenWidth() / 4;
    int y = isInverted ? GetScreenHeight() / 2 : GetScreenHeight() / 2 - 64;
    Rectangle rec = (Rectangle) {(float) quarter, (float) y, 16.0f, 64.0f};

    for (int i = 0; i < BLOCKAGE_AMOUNT; i++) {
        Blockage blockage = blockages[i];
        Rectangle blockageRec = (Rectangle) {
                (float) blockage.x,
                (float) (blockage.isInverted ? GetScreenHeight() / 2 : GetScreenHeight() / 2 - 128),
                1.0f,
                128.0f
        };

        if (CheckCollisionRecs(rec, blockageRec)) {
            dead = true;
        }
    }
}

void drawPlayer() {
    int quarter = GetScreenWidth() / 4;

    DrawRectangleLines(
            quarter,
            isInverted ? GetScreenHeight() / 2 : GetScreenHeight() / 2 - 64,
            16,
            64,
            accent);
}

void restartGame() {
    camera.rotation = 7.5f;

    for (int i = 0; i < BLOCKAGE_AMOUNT; i++) {
        createNewBlockage(i);
    }

    isInverted = false;
    target = BLACK;
    accent = BLACK;

    dead = false;
    score = 0;
}

void updateGame() {
    camera.rotation = Lerp(camera.rotation, 7.5f, 0.25f);
    accent = lerpColor(accent, target, 0.2f);

    if (GetTime() - lastEarthquakeTime > EARTHQUAKE_PERIOD) {
        camera.rotation += 2.0f * (GetRandomValue(0, 1) > 0 ? -1.0f : 1.0f);
        lastEarthquakeTime = GetTime();
    }

    if (IsKeyPressed(KEY_SPACE)) {
        isInverted = !isInverted;
        target = isInverted ? WHITE : BLACK;

        camera.rotation += (float) GetRandomValue(2, 8);
    }

    for (int i = 0; i < BLOCKAGE_AMOUNT; i++) {
        Blockage *blockage = &blockages[i];
        blockage->x -= 5;

        if (blockage->x <= 0) {
            createNewBlockage(i);
            score++;
        }
    }

    updatePlayer();
}

void drawGame() {
    ClearBackground(invertColor(accent));

    BeginMode2D(camera);

    DrawLine(
            -GetScreenWidth() / 2,
            GetScreenHeight() / 2,
            GetScreenWidth() * 2,
            GetScreenHeight() / 2,
            accent);

    for (int i = 0; i < BLOCKAGE_AMOUNT; i++) {
        Blockage blockage = blockages[i];
        DrawRectangle(
                blockage.x,
                blockage.isInverted ? GetScreenHeight() / 2 : GetScreenHeight() / 2 - 128,
                1,
                128,
                accent
        );
    }

    drawPlayer();

    DrawText(TextFormat("score: %d", score), 64, 64, 32, accent);

    if (dead) {
        drawTextCentered("you died", 48, GetScreenHeight() / 2 - 24);
        drawTextCentered("press any key to restart", 24, GetScreenHeight() - 18 - 48);
    }

    EndMode2D();
}

void drawTextCentered(const char *text, int fontSize, int y) {
    int width = MeasureText(text, fontSize);
    DrawText(text, GetScreenWidth() / 2 - width / 2, y, fontSize, accent);
}

bool isAnyKeyPressed() {
    for (int i = KEY_BACK; i < KEY_KB_MENU; i++) {
        if (IsKeyPressed(i)) {
            return true;
        }
    }

    return false;
}
