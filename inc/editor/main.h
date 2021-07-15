#pragma once

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

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

// Function pointer for menu-clicked callback.
typedef void (*MenuCallback)(void);

class Editor
{
public:
    // Stores information about a single line.
    struct EditorLine
    {
        size_t Line;
        size_t Length;
        char *Text;
    };

    // A single item in the menu.
    struct MenuItem
    {
        size_t ParentID; // 0 for the root menu.
        size_t ID;
        MenuCallback Callback;
    };

    // The full menu that should be rendered. Can be nested.
    typedef struct
    {
        size_t MenuSize;
        struct MenuItem *Menu;
        size_t ActiveMenuItem;
    } EditorMenu;

    struct EditorMessage
    {
        size_t TextLength;
        char *Text;

        size_t MessageWidth;
        size_t MessageHeight;
    };

    // Provides all the context for rendering the editor.
    struct EditorLayout
    {
        size_t ScreenWidth;
        size_t ScreenHeight;

        size_t HeaderHeight;

        EditorMenu Menu;

        size_t TextBoxX;
        size_t TextBoxY;
        size_t TextBoxWidth;
        size_t TextBoxHeight;

        bool HasMessage;
        struct EditorMessage *CurrentMessage;
    };

    // Given the kernel's keyboard handler ID, so that it may restore it at a later point.
    void StartEditor(int KernelCallbackID);

private:
    // =========================== Drawing routines =========================== //

    struct EditorLayout Layout;

    size_t Length = 0; // How many lines?
    size_t Size = 0;   // How many characters?
    size_t CurrentLine = 0;
    size_t CurrentColumn = 0;
    struct EditorLine *Lines;
    // Draw everything.
    void DrawEditor();

    void DrawHeader();
    void DrawBackground();
    void DrawTextArea();
    void DrawTextLine();
    void DrawText();
    void DrawMenus();
    void DrawBoxes();
    void DrawBoxShadows();

    // =========================== State management ===========================

    // Text editing.
    void GetLine(size_t line);
    void SetLine(struct EditorLine *line);
    void AppendLine(struct EditorLine *line);
    void AppendToLine(struct EditorLine *line, size_t textLength, char *text);
    void RemoveLine(size_t line);
};