//keyboard.h

#define KEYBOARD_SET2
#ifdef KEYBOARD_SET2

typedef enum {
    KEY_NONE,

    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,

    KEY_ESC,
    KEY_GRAVE,
    KEY_TAB,

    KEY_LEFT_CTRL,
    KEY_LEFT_SHIFT,
    KEY_LEFT_ALT,

    KEY_Q,
    KEY_W,
    KEY_E,
    KEY_R,
    KEY_T,
    KEY_Y,
    KEY_U,
    KEY_I,
    KEY_O,
    KEY_P,

    KEY_A,
    KEY_S,
    KEY_D,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_J,
    KEY_K,
    KEY_L,

    KEY_Z,
    KEY_X,
    KEY_C,
    KEY_V,
    KEY_B,
    KEY_N,
    KEY_M,

    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_0,

    KEY_MINUS,
    KEY_EQUAL,
    KEY_LBRACKET,
    KEY_RBRACKET,
    KEY_BACKSLASH,

    KEY_SEMICOLON,
    KEY_APOSTROPHE,
    KEY_COMMA,
    KEY_DOT,
    KEY_SLASH,

    KEY_SPACE,
    KEY_ENTER,
    KEY_BACKSPACE,

    KEY_CAPSLOCK,
    KEY_RIGHT_SHIFT,

    KEY_NUMLOCK,
    KEY_SCROLLLOCK,

    KEY_KP_0,
    KEY_KP_1,
    KEY_KP_2,
    KEY_KP_3,
    KEY_KP_4,
    KEY_KP_5,
    KEY_KP_6,
    KEY_KP_7,
    KEY_KP_8,
    KEY_KP_9,
    KEY_KP_DOT,
    KEY_KP_PLUS,
    KEY_KP_MINUS,
    KEY_KP_STAR,

    KEY_RIGHT_ALT,
    KEY_RIGHT_CTRL,

    KEY_LEFT_GUI,
    KEY_RIGHT_GUI,
    KEY_APPS,

    KEY_PRINTSCREEN,
    KEY_PAUSE,

    KEY_INSERT,
    KEY_DELETE,
    KEY_HOME,
    KEY_END,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,

    KEY_ARROW_UP,
    KEY_ARROW_DOWN,
    KEY_ARROW_LEFT,
    KEY_ARROW_RIGHT,

    KEY_KP_SLASH,
    KEY_KP_ENTER,

    KEY_MEDIA_SEARCH,
    KEY_MEDIA_PREV,
    KEY_MEDIA_FAVOURITES,
    KEY_MEDIA_REFRESH,
    KEY_MEDIA_VOLUME_DOWN,
    KEY_MEDIA_MUTE,
    KEY_MEDIA_CALCULATOR,
    KEY_MEDIA_FORWARD,
    KEY_MEDIA_VOLUME_UP,
    KEY_MEDIA_PLAY_PAUSE,
    KEY_MEDIA_STOP,
    KEY_MEDIA_BACK,
    KEY_MEDIA_HOME,
    KEY_MEDIA_NEXT,
    KEY_MEDIA_SELECT,
    KEY_MEDIA_MY_COMPUTER,
    KEY_MEDIA_EMAIL,

    KEY_ACPI_POWER,
    KEY_ACPI_SLEEP,
    KEY_ACPI_WAKE,
} key_t;

static const key_t set2_table[256] = {
    [0x01] = KEY_F9,
    [0x03] = KEY_F5,
    [0x04] = KEY_F3,
    [0x05] = KEY_F1,
    [0x06] = KEY_F2,
    [0x07] = KEY_F12,

    [0x09] = KEY_F10,
    [0x0A] = KEY_F8,
    [0x0B] = KEY_F6,
    [0x0C] = KEY_F4,

    [0x0D] = KEY_TAB,
    [0x0E] = KEY_GRAVE,

    [0x11] = KEY_LEFT_ALT,
    [0x12] = KEY_LEFT_SHIFT,
    [0x14] = KEY_LEFT_CTRL,

    [0x15] = KEY_Q,
    [0x16] = KEY_1,

    [0x1A] = KEY_Z,
    [0x1B] = KEY_S,
    [0x1C] = KEY_A,
    [0x1D] = KEY_W,
    [0x1E] = KEY_2,

    [0x21] = KEY_C,
    [0x22] = KEY_X,
    [0x23] = KEY_D,
    [0x24] = KEY_E,
    [0x25] = KEY_4,
    [0x26] = KEY_3,

    [0x29] = KEY_SPACE,
    [0x2A] = KEY_V,
    [0x2B] = KEY_F,
    [0x2C] = KEY_T,
    [0x2D] = KEY_R,
    [0x2E] = KEY_5,

    [0x31] = KEY_N,
    [0x32] = KEY_B,
    [0x33] = KEY_H,
    [0x34] = KEY_G,
    [0x35] = KEY_Y,
    [0x36] = KEY_6,

    [0x3A] = KEY_M,
    [0x3B] = KEY_J,
    [0x3C] = KEY_U,
    [0x3D] = KEY_7,
    [0x3E] = KEY_8,

    [0x41] = KEY_COMMA,
    [0x42] = KEY_K,
    [0x43] = KEY_I,
    [0x44] = KEY_O,
    [0x45] = KEY_0,
    [0x46] = KEY_9,

    [0x49] = KEY_DOT,
    [0x4A] = KEY_SLASH,
    [0x4B] = KEY_L,
    [0x4C] = KEY_SEMICOLON,
    [0x4D] = KEY_P,
    [0x4E] = KEY_MINUS,

    [0x52] = KEY_APOSTROPHE,
    [0x54] = KEY_LBRACKET,
    [0x55] = KEY_EQUAL,

    [0x58] = KEY_CAPSLOCK,
    [0x59] = KEY_RIGHT_SHIFT,
    [0x5A] = KEY_ENTER,
    [0x5B] = KEY_RBRACKET,
    [0x5D] = KEY_BACKSLASH,

    [0x66] = KEY_BACKSPACE,

    [0x69] = KEY_KP_1,
    [0x6B] = KEY_KP_4,
    [0x6C] = KEY_KP_7,

    [0x70] = KEY_KP_0,
    [0x71] = KEY_KP_DOT,
    [0x72] = KEY_KP_2,
    [0x73] = KEY_KP_5,
    [0x74] = KEY_KP_6,
    [0x75] = KEY_KP_8,

    [0x76] = KEY_ESC,
    [0x77] = KEY_NUMLOCK,

    [0x78] = KEY_F11,
    [0x79] = KEY_KP_PLUS,
    [0x7A] = KEY_KP_3,
    [0x7B] = KEY_KP_MINUS,
    [0x7C] = KEY_KP_STAR,
    [0x7D] = KEY_KP_9,

    [0x7E] = KEY_SCROLLLOCK,

    [0x83] = KEY_F7,
};
static const key_t set2_e0_table[256] = {
    [0x10] = KEY_MEDIA_SEARCH,
    [0x11] = KEY_RIGHT_ALT,
    [0x14] = KEY_RIGHT_CTRL,
    [0x15] = KEY_MEDIA_PREV,
    [0x18] = KEY_MEDIA_FAVOURITES,

    [0x1F] = KEY_LEFT_GUI,

    [0x20] = KEY_MEDIA_REFRESH,
    [0x21] = KEY_MEDIA_VOLUME_DOWN,
    [0x23] = KEY_MEDIA_MUTE,

    [0x27] = KEY_RIGHT_GUI,
    [0x28] = KEY_MEDIA_STOP,
    [0x2B] = KEY_MEDIA_CALCULATOR,
    [0x2F] = KEY_APPS,

    [0x30] = KEY_MEDIA_FORWARD,
    [0x32] = KEY_MEDIA_VOLUME_UP,
    [0x34] = KEY_MEDIA_PLAY_PAUSE,

    [0x37] = KEY_ACPI_POWER,

    [0x38] = KEY_MEDIA_BACK,
    [0x3A] = KEY_MEDIA_HOME,
    [0x3B] = KEY_MEDIA_STOP,

    [0x3F] = KEY_ACPI_SLEEP,

    [0x40] = KEY_MEDIA_MY_COMPUTER,
    [0x48] = KEY_MEDIA_EMAIL,

    [0x4A] = KEY_KP_SLASH,
    [0x4D] = KEY_MEDIA_NEXT,
    [0x50] = KEY_MEDIA_SELECT,

    [0x5A] = KEY_KP_ENTER,
    [0x5E] = KEY_ACPI_WAKE,

    [0x69] = KEY_END,
    [0x6B] = KEY_ARROW_LEFT,
    [0x6C] = KEY_HOME,

    [0x70] = KEY_INSERT,
    [0x71] = KEY_DELETE,
    [0x72] = KEY_ARROW_DOWN,
    [0x74] = KEY_ARROW_RIGHT,
    [0x75] = KEY_ARROW_UP,

    [0x7A] = KEY_PAGE_DOWN,
    [0x7D] = KEY_PAGE_UP,
};

#endif



int keyboard_init(void);
key_t keyboard_poll_k();
