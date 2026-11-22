//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

/** @file util/window_id.hpp
 *
 *  @brief Enumerations of all window ids used.
 */

// ----------------------------------------------------------------------------- : Menu ids

/// Window ids for menus and toolbars
enum MenuID {
  ID_MENU_MIN = 0,
  ID_MENU_MAX = 999,

  //  File menu
  ID_FILE_NEW = wxID_NEW,
  ID_FILE_OPEN = wxID_OPEN,
  ID_FILE_SAVE = wxID_SAVE,
  ID_FILE_SAVE_AS = wxID_SAVEAS,
  ID_FILE_SAVE_AS_DIRECTORY = 12,
  ID_FILE_EXIT = wxID_EXIT,
  ID_FILE_EXPORT = 2,
  ID_FILE_EXPORT_IMAGE = 4,
  ID_FILE_EXPORT_IMAGES = 5,
  ID_FILE_PRINT = wxID_PRINT,
  ID_FILE_PRINT_PREVIEW = wxID_PREVIEW,
  ID_FILE_INSPECT = 8,
  ID_FILE_RELOAD = 9,
  ID_FILE_RECENT = wxID_FILE1,
  ID_FILE_RECENT_MAX = wxID_FILE9,

  // Edit menu
  ID_EDIT_UNDO = wxID_UNDO,
  ID_EDIT_REDO = wxID_REDO,
  ID_EDIT_CUT = wxID_CUT,
  ID_EDIT_COPY = wxID_COPY,
  ID_EDIT_PASTE = wxID_PASTE,
  ID_EDIT_SELECT_ALL = wxID_SELECTALL,
  ID_EDIT_FIND = wxID_FIND,
  ID_EDIT_FIND_NEXT = 103,
  ID_EDIT_REPLACE = wxID_REPLACE,
  ID_EDIT_AUTO_REPLACE = 104,
  ID_EDIT_PREFERENCES = 105,

  // Window menu (MainWindow)
  ID_WINDOW_NEW = 201,
  ID_WINDOW_MIN = 202,
  ID_WINDOW_CARDS = ID_WINDOW_MIN + 0,
  ID_WINDOW_SET = ID_WINDOW_MIN + 1,
  ID_WINDOW_STYLE = ID_WINDOW_MIN + 2,
  // Setsmith: Keywords, Statistics and Random Pack panels removed.
  ID_WINDOW_MAX = 220,

  // Window menu: full screen
  ID_WINDOW_FULLSCREEN, // Setsmith: F11 turns full screen on and off
  ID_HELP_ABOUT = wxID_ABOUT,

};


// ----------------------------------------------------------------------------- : Child ids

/// Ids for menus on child panels (SetWindowPanel)
enum ChildMenuID {
  ID_CHILD_MIN = 6000,
  ID_CHILD_MAX = 16999,

  // Cards menu
  ID_CARD_ADD = 6001,
  ID_CARD_ADD_MULT,
  ID_CARD_REMOVE,
  ID_CARD_PREV,
  ID_CARD_NEXT,
  ID_CARD_SEARCH,
  ID_CARD_ROTATE,
  ID_CARD_ROTATE_0,
  ID_CARD_ROTATE_90,
  ID_CARD_ROTATE_180,
  ID_CARD_ROTATE_270,
  // CardList
  ID_SELECT_COLUMNS,

  // Format menu
  ID_FORMAT_BOLD = 6201,
  ID_FORMAT_ITALIC,
  ID_FORMAT_UNDERLINE,
  ID_FORMAT_SYMBOL,
  ID_FORMAT_REMINDER,
  ID_INSERT_SYMBOL,

  // Spelling errors
  ID_SPELLING_ADD_TO_DICT = 6301,
  ID_SPELLING_NO_SUGGEST,
  ID_SPELLING_SUGGEST,
  ID_SPELLING_SUGGEST_MAX = 6399,

  // On cards panel
  ID_COLLAPSE_NOTES = 8001,
  ID_CARD_FILTER,
  
  // Style panel
  ID_STYLE_USE_FOR_ALL = 8011,
  ID_STYLE_USE_CUSTOM,

  // Statistics panel
  ID_FIELD_LIST = 8301,
  
  // SymbolFont (Format menu)
  ID_INSERT_SYMBOL_MENU_MIN =  9001,
  ID_INSERT_SYMBOL_MENU_MAX = 10000,
  
  // AddCardsScript (Card menu)
  ID_ADD_CARDS_MENU_MIN = 10001,
  ID_ADD_CARDS_MENU_MAX = 11000,
};


// ----------------------------------------------------------------------------- : Control/window ids

/// Window ids for controls
enum ControlID {
  ID_CONTROL_MIN = 1000,
  ID_CONTROL_MAX = 1999,

  // New Set Window
  ID_STYLESHEET_FILTER,
  ID_GAME_FILTER,

  // Controls
  ID_VIEWER = 1001,
  ID_EDITOR,
  ID_CARD_LIST,
  ID_GAME_LIST,
  ID_STYLESHEET_LIST,
  ID_NOTES,
  // Card list column select
  ID_MOVE_UP,
  ID_MOVE_DOWN,
  ID_SHOW,
  ID_HIDE,
  // Card select
  ID_SELECT_CARDS,
  ID_SELECTION_CHOICE,
  ID_SELECTION_CHOICE_MAX = ID_SELECTION_CHOICE + 100,
  ID_SELECT_ALL,
  ID_SELECT_NONE,
  // Settings
  ID_NOTEBOOK,
  // Image slicer
  ID_PREVIEW,
  ID_SELECTOR,
  ID_SIZE,
  ID_LEFT,
  ID_TOP,
  ID_WIDTH,
  ID_HEIGHT,
  ID_SELECTION_CENTER,
  ID_SELECTION_CENTER_HORIZONTALLY,
  ID_SELECTION_CENTER_VERTICALLY,
  ID_FIX_ASPECT,
  ID_ZOOM,
  ID_ZOOM_X,
  ID_ZOOM_Y,
  ID_EXPORT_ZOOM,
  ID_SHARPEN,
  ID_SHARPEN_AMOUNT,
  // Internal window
  ID_INTERNAL_SCALE,
  // Auto replace window
  ID_USE_AUTO_REPLACE,
  ID_ITEM_VALUE,
  ID_ADD_ITEM,
  ID_REMOVE_ITEM,
  ID_DEFAULTS,
};

