#pragma once

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This file contains most of the symbols required to start and run the Chroma Editor.
 *
 *  The Editor uses a six-stage drawing system:
 *   - Background
 *   - Bars
 *   - Text Area
 *   - Text
 *   - Effects
 *
 * The dimensions are derived dynamically from the Print Info settings.
 *
 * The layout is as such:
 *  - 3% horizontal header, #BBBBBB.
 *    - Primary text, #000084.
 *    - Secondary text, #000000.
 *  - 97% background, #000084.
 *  - 15% optional banner, #00AAAA.
 *  - 80% text area, #BBBBBB.
 *  - Menus #FFFFFF.
 *   - Buttons #BBBBBB.
 *   - Drop shadow #000000.
 */

// Stores information about a single line.
struct EditorLine {
    size_t Line;
    size_t Length;
    char* Text;
};

// Function pointer for menu-clicked callback.
typedef void (*MenuCallback)(void);

// A single item in the menu.
struct MenuItem {
    size_t ParentID; // 0 for the root menu.
    size_t ID;
    MenuCallback Callback;
};

// The full menu that should be rendered. Can be nested.
typedef struct {
    size_t MenuSize;
    struct MenuItem* Menu;
    size_t ActiveMenuItem;
} EditorMenu;

struct EditorMessage {
    size_t TextLength;
    char* Text;

    size_t MessageWidth;
    size_t MessageHeight;
};

// Provides all the context for rendering the editor.
struct EditorLayout {
    size_t ScreenWidth;
    size_t ScreenHeight;

    size_t HeaderHeight;

    EditorMenu Menu;

    size_t TextBoxX;
    size_t TextBoxY;
    size_t TextBoxWidth;
    size_t TextBoxHeight;

    bool HasMessage;
    struct EditorMessage* CurrentMessage;
};

// Provides all the context for manipulating state.
typedef struct {
    struct EditorLayout Layout;

    size_t Length; // How many lines?
    size_t Size;   // How many characters?
    size_t CurrentLine;
    size_t CurrentColumn;
    struct EditorLine* Lines;
} EditorState;


// Given the kernel's keyboard handler ID, so that it may restore it at a later point.
void StartEditor(int KernelCallbackID);

// =========================== Drawing routines =========================== //

// Draw everything.
void DrawEditor(EditorState* currentState);

void DrawHeader();
void DrawBackground();
void DrawTextArea();
void DrawText();
void DrawLine();
void DrawMenus();
void DrawBoxes();
void DrawBoxShadows();

// =========================== State management ===========================
EditorState* GetState();

// Text editing.
void GetLine(EditorState* state, size_t line);
void SetLine(EditorState* state, struct EditorLine* line);
void AppendLine(EditorState* state, struct EditorLine* line);
void AppendToLine(EditorState* state, struct EditorLine* line, size_t textLength, char* text);
void RemoveLine(EditorState* state, size_t line);

#ifdef  __cplusplus
}
#endif