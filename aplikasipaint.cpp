#include <iostream>
#include <SDL.h>
#include <string>
#include <vector>

// Constants
const int WIDTH = 640, HEIGHT = 480, CANVASTOP = 0, SIDEBAR_WIDTH = 60;
const char* windowTitle = "Painter";

// Window and renderer
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

// Color struct
struct _color {
    int R, G, B;
    void Set(int r, int g, int b) {
        R = r;
        G = g;
        B = b;
    }
};

// Positions
std::pair<int, int> prevPos;
std::pair<int, int> mousePos;

// Pen attributes
_color penColor;
bool pressed = false;
bool wasPressed = false;

class _box {
private:
    std::pair<int, int> pos;
    int w, h;
public:
    std::string Stats() {
        std::string s = "";
        s += std::to_string(pos.first) + " ";
        s += std::to_string(pos.second) + " ";
        s += std::to_string(w) + " ";
        s += std::to_string(h) + "\n";
        return s;
    }

    void SetBox(int x, int y, int W, int H) {
        pos.first = x;
        pos.second = y;
        w = W;
        h = H;
    }

    void DrawBox(int R, int G, int B) {
        SDL_Rect r;
        r.x = pos.first;
        r.y = pos.second;
        r.w = w;
        r.h = h;
        SDL_SetRenderDrawColor(renderer, R, G, B, 255);
        SDL_RenderFillRect(renderer, &r);
        SDL_RenderPresent(renderer);
    }

    bool CheckInbound(int x, int y) {
        return pos.first < x && pos.second < y && pos.first + w > x && pos.second + h > y;
    }
};

// Surface class for drawing
class _surface {
public:
    _box hitBox;
    _color bgColor;
    void DrawSurface() {
        hitBox.DrawBox(bgColor.R, bgColor.G, bgColor.B);
    }
};

_surface canvas;
_surface sidebar;
_surface ui;

void DrawCircle(int r, int x, int y) {
    for (int i = x - r + 1; i < x + r; i++) {
        for (int j = y - r + 1; j < y + r; j++) {
            if (canvas.hitBox.CheckInbound(i, j) && (i - x) * (i - x) + (j - y) * (j - y) <= r * r) {
                SDL_RenderDrawPoint(renderer, i, j);
            }
        }
    }
}

// Brush class for drawing
class _brush {
private:
    int brushSize = 4; // Default brush size 4
    _color brushColor;
public:
    void SetColor(_color c) {
        brushColor = c;
    }

    void ChangeSize(int c) {
        brushSize += c;
        if (brushSize <= 0) {
            brushSize = 1;
        }
    }

    void Draw() {
        if (canvas.hitBox.CheckInbound(mousePos.first, mousePos.second)) {
            SDL_SetRenderDrawColor(renderer, brushColor.R, brushColor.G, brushColor.B, 255);
            DrawCircle(brushSize, mousePos.first, mousePos.second);
            SDL_RenderPresent(renderer);
        }
    }
};

_brush brush;

// Button class for UI elements
class _button {
public:
    _surface surface;
    void Press(int x, int y) {
        if (surface.hitBox.CheckInbound(x, y)) {
            brush.SetColor(surface.bgColor);
        }
    }
};

std::vector<_button> buttons;

void AddButtons() {
    int buttonHeight = 50;
    int padding = 10;
    int x = WIDTH - SIDEBAR_WIDTH + padding;
    int y = padding;

    // Add color buttons
    std::vector<_color> colors = { {255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {0, 0, 0} };
    for (const auto& color : colors) {
        _button newButt;
        newButt.surface.hitBox.SetBox(x, y, SIDEBAR_WIDTH - 2 * padding, buttonHeight);
        newButt.surface.bgColor = color;
        buttons.push_back(newButt);
        y += buttonHeight + padding;
    }

    // Add eraser button
    _button eraserButton;
    eraserButton.surface.hitBox.SetBox(x, y, SIDEBAR_WIDTH - 2 * padding, buttonHeight);
    eraserButton.surface.bgColor.Set(255, 255, 255);  // White for eraser
    buttons.push_back(eraserButton);
    y += buttonHeight + padding;

    // Add brush size increase button
    _button increaseBrushButton;
    increaseBrushButton.surface.hitBox.SetBox(x, y, SIDEBAR_WIDTH - 2 * padding, buttonHeight);
    increaseBrushButton.surface.bgColor.Set(100, 100, 100);  // Gray for increase size
    buttons.push_back(increaseBrushButton);
    y += buttonHeight + padding;

    // Add brush size decrease button
    _button decreaseBrushButton;
    decreaseBrushButton.surface.hitBox.SetBox(x, y, SIDEBAR_WIDTH - 2 * padding, buttonHeight);
    decreaseBrushButton.surface.bgColor.Set(150, 150, 150);  // Light gray for decrease size
    buttons.push_back(decreaseBrushButton);
}

void Click() {
    if (sidebar.hitBox.CheckInbound(mousePos.first, mousePos.second)) {
        for (size_t i = 0; i < buttons.size(); i++) {
            if (buttons[i].surface.hitBox.CheckInbound(mousePos.first, mousePos.second)) {
                if (i < 4) {
                    brush.SetColor(buttons[i].surface.bgColor);  // Color buttons
                }
                else if (i == 4) {
                    brush.SetColor(canvas.bgColor);  // Eraser button
                }
                else if (i == 5) {
                    brush.ChangeSize(1);  // Increase brush size
                }
                else if (i == 6) {
                    brush.ChangeSize(-1);  // Decrease brush size
                }
            }
        }
    }
}

void CanvasAndUISetup() {
    canvas.hitBox.SetBox(0, CANVASTOP, WIDTH - SIDEBAR_WIDTH, HEIGHT - CANVASTOP);
    sidebar.hitBox.SetBox(WIDTH - SIDEBAR_WIDTH, CANVASTOP, SIDEBAR_WIDTH, HEIGHT - CANVASTOP);
    ui.hitBox.SetBox(0, 0, WIDTH, CANVASTOP);

    canvas.bgColor.Set(255, 255, 255);
    sidebar.bgColor.Set(200, 200, 200);
    ui.bgColor.Set(200, 200, 200);

    canvas.DrawSurface();
    sidebar.DrawSurface();
    ui.DrawSurface();

    AddButtons();
    for (_button b : buttons) {
        b.surface.DrawSurface();
    }
}

void WindowSetup() {
    SDL_SetMainReady();
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer);
    SDL_RenderSetScale(renderer, 1, 1);
    SDL_SetWindowTitle(window, windowTitle);
}

void FrameUpdate() {
    while (true) {
        SDL_Event event;
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                break;
            }
            switch (event.type) {
            case SDL_MOUSEMOTION: {
                int x, y;
                prevPos = mousePos;
                SDL_GetMouseState(&x, &y);
                mousePos = { x, y };
                if (event.button.button == SDL_BUTTON_LMASK) {
                    brush.Draw();
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN: {
                if (event.button.button == SDL_BUTTON_LMASK) {
                    Click();
                }
                break;
            }
            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                case SDLK_e: {
                    brush.SetColor(canvas.bgColor);  // Set to eraser
                    break;
                }
                case SDLK_r: {
                    canvas.DrawSurface();  // Reset canvas
                    break;
                }
                case SDLK_EQUALS: {
                    brush.ChangeSize(1);  // Increase brush size
                    break;
                }
                case SDLK_MINUS: {
                    brush.ChangeSize(-1);  // Decrease brush size
                    break;
                }
                default:
                    break;
                }
                break;
            }
            default:
                break;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    WindowSetup();
    CanvasAndUISetup();
    brush.SetColor(ui.bgColor);
    FrameUpdate();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
