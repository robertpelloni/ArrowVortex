#ifndef NDEBUG
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <System/System.h>
#include <System/Resources.h>
#include <System/File.h>
#include <System/Debug.h>

#include <Core/WideString.h>
#include <Core/StringUtils.h>
#include <Core/Shader.h>

#include <Editor/Editor.h>
#include <Editor/Menubar.h>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>
#ifdef _WIN32
#define _UNICODE
#include <System/OpenGL.h>
#include <winuser.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <commdlg.h>
#include <gl/gl.h>
#endif
#undef ERROR

#include <chrono>
#include <thread>
#include <numeric>
#include <stdio.h>
#include <ctime>
#include <bitset>
#include <list>
#include <vector>
#include <map>

#undef DELETE

// Enable visual styles.
#pragma comment(linker, \
                "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Various statics had to be pulled out of the System singleton since
// SDL needs non-member callback functions for event handling.
bool myInitSuccesful = false;
Vortex::InputEvents myEvents;
std::string myInput;
Vortex::vec2i myMousePos = {0, 0};
Vortex::vec2i mySize = {0, 0};
SDL_Window* window = nullptr;
Vortex::Cursor::Icon myCursor = Vortex::Cursor::ARROW;
bool myIsActive = false;
bool myIsTerminated = false;
bool myIsInsideMessageLoop = false;
std::vector<std::string> droppedFiles;
std::bitset<Vortex::Key::MAX_VALUE> myKeyState;
std::bitset<Vortex::Mouse::MAX_VALUE> myMouseState;

namespace Vortex {

std::chrono::duration<double> deltaTime;  // Defined in <Core/Core.h>

namespace {

static wchar_t sRunDir[MAX_PATH + 1] = {};
static wchar_t sExeDir[MAX_PATH + 1] = {};

// Swap interval extension for enabling/disabling vsync.
typedef BOOL(APIENTRY* PFNWGLSWAPINTERVALFARPROC)(int);
static PFNWGLSWAPINTERVALFARPROC wglSwapInterval;

static std::string outPath;

// Mapping of windows virtual keys to vortex key codes.
static const int VKtoKCmap[] = {
    SDLK_GRAVE,
    Key::ACCENT,
    SDLK_MINUS,
    Key::DASH,
    SDLK_EQUALS,
    Key::EQUAL,
    SDLK_LEFTBRACKET,
    Key::BRACKET_L,
    SDLK_RIGHTBRACKET,
    Key::BRACKET_R,
    SDLK_SEMICOLON,
    Key::SEMICOLON,
    SDLK_APOSTROPHE,
    Key::QUOTE,
    SDLK_BACKSLASH,
    Key::BACKSLASH,
    SDLK_COMMA,
    Key::COMMA,
    SDLK_PERIOD,
    Key::PERIOD,
    SDLK_SLASH,
    Key::SLASH,
    SDLK_SPACE,
    Key::SPACE,
    SDLK_ESCAPE,
    Key::ESCAPE,
    SDLK_LGUI,
    Key::SYSTEM_L,
    SDLK_RGUI,
    Key::SYSTEM_R,
    SDLK_TAB,
    Key::TAB,
    SDLK_CAPSLOCK,
    Key::CAPS,
    SDLK_RETURN,
    Key::RETURN,
    SDLK_BACKSPACE,
    Key::BACKSPACE,
    SDLK_PAGEUP,
    Key::PAGE_UP,
    SDLK_PAGEDOWN,
    Key::PAGE_DOWN,
    SDLK_HOME,
    Key::HOME,
    SDLK_END,
    Key::END,
    SDLK_INSERT,
    Key::INSERT,
    SDLK_DELETE,
    Key::DELETE,
    SDLK_PRINTSCREEN,
    Key::PRINT_SCREEN,
    SDLK_SCROLLLOCK,
    Key::SCROLL_LOCK,
    SDLK_PAUSE,
    Key::PAUSE,
    SDLK_LEFT,
    Key::LEFT,
    SDLK_RIGHT,
    Key::RIGHT,
    SDLK_UP,
    Key::UP,
    SDLK_DOWN,
    Key::DOWN,
    SDLK_NUMLOCKCLEAR,
    Key::NUM_LOCK,
    SDLK_KP_MEMDIVIDE,
    Key::NUMPAD_DIVIDE,
    SDLK_KP_MEMMULTIPLY,
    Key::NUMPAD_MULTIPLY,
    SDLK_KP_MEMSUBTRACT,
    Key::NUMPAD_SUBTRACT,
    SDLK_KP_MEMADD,
    Key::NUMPAD_ADD,
    SDLK_SEPARATOR,
    Key::NUMPAD_SEPERATOR,
};

// Translates a dialog button type to a windows message box type.
static int sDlgType[System::NUM_BUTTONS] = {MB_OK, MB_OKCANCEL, MB_YESNO,
                                            MB_YESNOCANCEL};

// Translates a dialog icon type to a windows message box icon.
static int sDlgIcon[System::NUM_ICONS] = {0, MB_ICONASTERISK, MB_ICONWARNING,
                                          MB_ICONHAND};

static void SDLCALL FileDialogCallback(void* userdata,
                                       const char* const* filelist,
                                       int filter) {
    if (filelist && filelist[0]) {
        outPath = filelist[0];
    }
}

// Shows an open/save message box and returns the path selected by the user.
static std::string ShowFileDialog(std::string title, std::string path,
                                  SDL_DialogFileFilter filters[],
                                  int num_filters, int* index, bool save) {
    if (save) {
        SDL_ShowSaveFileDialog(FileDialogCallback, nullptr, nullptr, filters,
                               num_filters, path.c_str());
    } else {
        SDL_ShowOpenFileDialog(FileDialogCallback, nullptr, nullptr, filters,
                               num_filters, path.c_str(), false);
    }
    return outPath;
}

// ================================================================================================
// SystemImpl :: Debug logging.

static bool LogCheckpoint(bool result, const char* description) {
    if (result) {
        Debug::log("%s :: OK\n", description);
    } else {
        char lpMsgBuf[100];
        DWORD code = GetLastError();
        FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
            code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lpMsgBuf, 60,
            nullptr);
        Debug::blockBegin(Debug::ERROR, description);
        Debug::log("windows error code %i: %s", code, lpMsgBuf);
        Debug::blockEnd();
    }
    return !result;
}

// ================================================================================================
// SystemImpl :: menu item.

};  // anonymous namespace

typedef System::MenuItem MItem;

MItem* MItem::create() {
    return reinterpret_cast<MenuItem*>(CreatePopupMenu());
}

void MItem::addSeperator() {
    AppendMenuW(reinterpret_cast<HMENU>(this), MF_SEPARATOR, 0, nullptr);
}
void MItem::addItem(int item, const std::string& text) {
    AppendMenuW(reinterpret_cast<HMENU>(this), MF_STRING, item,
                Widen(text).str());
}

void MItem::addSubmenu(MItem* submenu, const std::string& text, bool grayed) {
    int flags = MF_STRING | MF_POPUP | (grayed * MF_GRAYED);
    AppendMenuW(reinterpret_cast<HMENU>(this), MF_STRING | MF_POPUP,
                reinterpret_cast<UINT_PTR>(submenu), Widen(text).str());
}

void MItem::replaceSubmenu(int pos, MItem* submenu, const std::string& text,
                           bool grayed) {
    int flags = MF_BYPOSITION | MF_STRING | MF_POPUP | (grayed * MF_GRAYED);
    DeleteMenu(reinterpret_cast<HMENU>(this), pos, MF_BYPOSITION);
    InsertMenuW(reinterpret_cast<HMENU>(this), pos, flags,
                reinterpret_cast<UINT_PTR>(submenu), Widen(text).str());
}

void MItem::setChecked(int item, bool state) {
    CheckMenuItem(reinterpret_cast<HMENU>(this), item,
                  state ? MF_CHECKED : MF_UNCHECKED);
}

void MItem::setEnabled(int item, bool state) {
    EnableMenuItem(reinterpret_cast<HMENU>(this), item,
                   state ? MF_ENABLED : MF_GRAYED);
}

namespace {

// ================================================================================================
// SystemImpl :: member data.

struct SystemImpl : public System {
    std::chrono::steady_clock::time_point myApplicationStartTime;
    std::map<SDL_Keycode, Key::Code> myKeyMap;
    std::string myTitle;
    SDL_GLContext myHRC = nullptr;
    SDL_Renderer* renderer = nullptr;
    std::string workingDirectory;

    // ================================================================================================
    // SystemImpl :: constructor and destructor.

    ~SystemImpl() {
        // Destroy the rendering context.
        if (myHRC) SDL_GL_DestroyContext(myHRC);

        // Destroy the window.
        if (window) SDL_DestroyWindow(window);
    }

    SystemImpl() : myTitle("ArrowVortex") {
        myApplicationStartTime = Debug::getElapsedTime();

        // Initialize the keymap, which maps windows virtual keys to vortex key
        // codes.
        int k = sizeof(VKtoKCmap) / sizeof(VKtoKCmap[0]);
        for (int i = 0; i < k; i += 2)
            myKeyMap.insert({static_cast<SDL_Keycode>(VKtoKCmap[i]),
                             static_cast<Key::Code>(VKtoKCmap[i + 1])});
        for (int i = 0; i < 26; ++i)
            myKeyMap.insert({static_cast<SDL_Keycode>(SDLK_A + i),
                             static_cast<Key::Code>(Key::A + i)});
        for (int i = 0; i < 10; ++i)
            myKeyMap.insert({static_cast<SDL_Keycode>(SDLK_0 + i),
                             static_cast<Key::Code>(Key::DIGIT_0 + i)});
        for (int i = 0; i < 15; ++i)
            myKeyMap.insert({static_cast<SDL_Keycode>(SDLK_F1 + i),
                             static_cast<Key::Code>(Key::F1 + i)});
        for (int i = 0; i < 9; ++i)
            myKeyMap.insert({static_cast<SDL_Keycode>(SDLK_KP_0 + i),
                             static_cast<Key::Code>(Key::NUMPAD_0 + i)});

        // Create a window handle.
        if (!SDL_CreateWindowAndRenderer("ArrowVortex", 800, 600,
                                         SDL_WINDOW_OPENGL, &window,
                                         &renderer)) {
            SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        }

        if (LogCheckpoint(window != nullptr, "creating window")) return;

        // Create the OpenGL rendering context.
        myHRC = SDL_GL_CreateContext(window);
        if (LogCheckpoint(myHRC != nullptr, "creating OpenGL context")) return;

        BOOL mc = SDL_GL_MakeCurrent(window, myHRC);
        if (LogCheckpoint(mc != 0, "activating OpenGL context")) return;

        VortexCheckGlError();

        // Initialize the OpenGL settings.
        glClearColor(0, 0, 0, 1);
        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnableClientState(GL_VERTEX_ARRAY);

        VortexCheckGlError();

        // Enable vsync for now, we will disable it later if the settings
        // require it.
        wglSwapInterval = reinterpret_cast<PFNWGLSWAPINTERVALFARPROC>(
            wglGetProcAddress("wglSwapIntervalEXT"));
        Debug::log("swap interval support :: %s\n",
                   wglSwapInterval ? "OK" : "MISSING");
        if (wglSwapInterval) {
            wglSwapInterval(1);
            VortexCheckGlError();
        }

        // Check for shader support.
        Shader::initExtension();
        Debug::logBlankLine();

        // Make sure the window is centered on the desktop.
        mySize = {1200, 900};
        SDL_SetWindowSize(window, mySize.x, mySize.y);
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED);

        // Show the window.
        myIsActive = true;
        myInitSuccesful = true;
    }

    // ================================================================================================
    // SystemImpl :: message loop.

    void createMenu() override {
#ifdef _WIN32
        HMENU menu = CreateMenu();
        gMenubar->init(reinterpret_cast<MenuItem*>(menu));
        SetMenu(GetActiveWindow(), menu);
#endif
    }

    // ================================================================================================
    // SystemImpl :: clipboard functions.

    bool setClipboardText(const std::string& text) override {
        return SDL_SetClipboardText(text.c_str());
    }

    std::string getClipboardText() const override {
        return std::string(SDL_GetClipboardText());
    }

    // ================================================================================================
    // SystemImpl :: message handling.

    int getKeyFlags() const override {
        int kc[6] = {Key::SHIFT_L, Key::SHIFT_R, Key::CTRL_L,
                     Key::CTRL_R,  Key::ALT_L,   Key::ALT_R};
        int kf[6] = {Keyflag::SHIFT, Keyflag::SHIFT, Keyflag::CTRL,
                     Keyflag::CTRL,  Keyflag::ALT,   Keyflag::ALT};

        int flags = 0;
        for (int i = 0; i < 6; ++i)
            if (myKeyState.test(kc[i])) flags |= kf[i];

        return flags;
    }

    Key::Code translateKeyCode(SDL_Keycode vkCode) override {
        static const UINT lshift = MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC);

        switch (vkCode) {
            case SDL_SCANCODE_LSHIFT:
                return Key::SHIFT_L;
            case SDL_SCANCODE_RSHIFT:
                return Key::SHIFT_R;
            case SDL_SCANCODE_LALT:
                return Key::ALT_L;
            case SDL_SCANCODE_RALT:
                return Key::ALT_R;
            case SDL_SCANCODE_LCTRL:
                return Key::CTRL_L;
            case SDL_SCANCODE_RCTRL:
                return Key::CTRL_R;
        }

        if (myKeyMap.contains(vkCode)) return myKeyMap[vkCode];
        return Key::NONE;
    }

    void handleKeyPress(Key::Code kc, bool repeated) override {
        bool handled = false;
        int kf = getKeyFlags();
        myEvents.addKeyPress(kc, kf, repeated);
        myKeyState.set(kc);
    }

    // ================================================================================================
    // SystemImpl :: dialog boxes.

    Result showMessageDlg(const std::string& title, const std::string& text,
                          Buttons b, Icon i) override {
        SDL_MessageBoxData box;
        SDL_MessageBoxButtonData buttons[3];
        box.flags = SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT;
        box.window = nullptr;
        box.title = title.c_str();
        box.message = text.c_str();
        box.colorScheme = nullptr;
        switch (b) {
            case (T_OK):
                box.numbuttons = 1;
                buttons[0].buttonID = R_OK;
                buttons[0].text = "OK";
                buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
                break;
            case (T_OK_CANCEL):
                box.numbuttons = 2;
                buttons[0].buttonID = R_OK;
                buttons[0].text = "OK";
                buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
                buttons[1].buttonID = R_CANCEL;
                buttons[1].text = "CANCEL";
                buttons[1].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
                break;
            case (T_YES_NO):
                box.numbuttons = 2;
                buttons[0].buttonID = R_YES;
                buttons[0].text = "YES";
                buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
                buttons[1].buttonID = R_NO;
                buttons[1].text = "NO";
                buttons[1].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
                break;
            case (T_YES_NO_CANCEL):
                box.numbuttons = 3;
                buttons[0].buttonID = R_YES;
                buttons[0].text = "YES";
                buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
                buttons[1].buttonID = R_NO;
                buttons[1].text = "NO";
                buttons[1].flags = 0;
                buttons[2].buttonID = R_CANCEL;
                buttons[2].text = "CANCEL";
                buttons[2].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
                break;
            default:
                break;
        }
        box.buttons = buttons;
        int result = 0;
        if (!SDL_ShowMessageBox(&box, &result)) {
            HudError("Failed to open message box with error %s",
                     SDL_GetError());
            return R_CANCEL;
        }
        return static_cast<Result>(result);
    }

    std::string openFileDlg(const std::string& title,
                            SDL_DialogFileFilter filters[], int num_filters,
                            const std::string& filename) override {
        return ShowFileDialog(title, filename, filters, num_filters, nullptr,
                              false);
    }

    std::string saveFileDlg(const std::string& title,
                            SDL_DialogFileFilter filters[], int num_filters,
                            int* index, const std::string& filename) override {
        return ShowFileDialog(title, filename, filters, num_filters, index,
                              true);
    }

    // ================================================================================================
    // SystemImpl :: misc/get/set functions.

    void openWebpage(const std::string& link) override {
        SDL_OpenURL(link.c_str());
    }

    void setWorkingDir(const std::string& path) override {
#ifdef _WIN32
        SetCurrentDirectoryW(Widen(path).str());
#endif
    }

    void setCursor(Cursor::Icon c) override { myCursor = c; }

    void disableVsync() override {
        if (wglSwapInterval) {
            Debug::log("[NOTE] turning off v-sync\n");
            wglSwapInterval(0);
        }
    }

    double getElapsedTime() const override {
        return Debug::getElapsedTime(myApplicationStartTime);
    }

    std::string getExeDir() const override { return Narrow(sExeDir); }

    std::string getRunDir() const override { return Narrow(sRunDir); }

    Cursor::Icon getCursor() const override { return myCursor; }

    bool isKeyDown(Key::Code key) const override {
        return myKeyState.test(key);
    }

    bool isMouseDown(Mouse::Code button) const override {
        return myMouseState.test(button);
    }

    vec2i getMousePos() const override { return myMousePos; }

    void setWindowTitle(const std::string& text) override {
        SDL_SetWindowTitle(window, text.c_str());
    }

    vec2i getWindowSize() const override { return mySize; }

    const std::string& getWindowTitle() const override { return myTitle; }

    InputEvents& getEvents() override { return myEvents; }

    bool isActive() const override { return myIsActive; }

    void terminate() override { myIsTerminated = true; }

};  // SystemImpl.
};  // anonymous namespace.

System* gSystem = nullptr;

};  // namespace Vortex
using namespace Vortex;

std::string System::getLocalTime() {
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    std::string time = asctime(localtime(&t));
    if (time.back() == '\n') Str::pop_back(time);
    return time;
}

std::string System::getBuildData() {
    std::string date(__DATE__);
    if (date[4] == ' ') date.begin()[4] = '0';
    return date;
}

static void ApplicationStart() {
    // TODO: Set the executable directory as the working dir.

    // Log the application start-up time.
    Debug::openLogFile();
    Debug::log("Starting ArrowVortex :: %s\n", System::getLocalTime().c_str());
    Debug::log("Build: %s\n", System::getBuildData().c_str());
    Debug::logBlankLine();
}

static void ApplicationEnd() {
    // Log the application termination time.
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    Debug::logBlankLine();
    Debug::log("Closing ArrowVortex :: %s", System::getLocalTime().c_str());
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
    ApplicationStart();
#ifndef NDEBUG
    Debug::openConsole();
#endif
    gSystem = new SystemImpl;

    Editor::create();
    // gSystem->forwardArgs(); // arg1 is simfile path
    gSystem->createMenu();  // TODO: multiplatform

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    using namespace std::chrono;

    if (!myInitSuccesful) return SDL_APP_FAILURE;

#ifndef NDEBUG
    long long frames = 0;
    auto lowcounts = 0;
    std::list<double> fpsList, sleepList, frameList, inputList, waitList;
    // Adjust frameGuess to your VSync target if you are testing with VSync
    // enabled
    auto frameGuess = 960;
#endif

    // Non-vsync FPS max target
    auto frameTarget = duration<double>(1.0 / 960.0);

    // Enter the message loop.
    MSG message;
    auto prevTime = Debug::getElapsedTime();
    // while (!myIsTerminated) {
    auto startTime = Debug::getElapsedTime();

    myEvents.clear();

    // Check if there were text input events.
    if (!myInput.empty()) {
        myEvents.addTextInput(myInput.c_str());
        myInput.clear();
    }

    // Set up the OpenGL view.
    glViewport(0, 0, mySize.x, mySize.y);
    glLoadIdentity();
    glOrtho(0, mySize.x, mySize.y, 0, -1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset the mouse cursor.
    myCursor = Cursor::ARROW;

#ifndef NDEBUG
    auto inputTime = Debug::getElapsedTime();

    VortexCheckGlError();
#endif

    gEditor->tick();

    // Display.
    SDL_GL_SwapWindow(window);

#ifndef NDEBUG
    auto renderTime = Debug::getElapsedTime();
#endif
    // Tick function.
    duration<double> frameTime = Debug::getElapsedTime() - prevTime;
    auto waitTime = frameTarget.count() - frameTime.count();

    if (wglSwapInterval) {
        while (Debug::getElapsedTime() - prevTime < frameTarget) {
            std::this_thread::yield();
        }
    }

    // End of frame
    auto curTime = Debug::getElapsedTime();
    deltaTime = duration<double>(static_cast<float> min(
        max(0, duration<double>(curTime - prevTime).count()), 0.25));
    prevTime = curTime;

#ifndef NDEBUG
    // Do frame statistics
    // Note that these will be wrong with VSync enabled.
    fpsList.push_front(deltaTime.count());
    waitList.push_front(duration<double>(curTime - renderTime).count());
    frameList.push_front(duration<double>(renderTime - inputTime).count());
    inputList.push_front(duration<double>(inputTime - startTime).count());

    if (abs(deltaTime.count() - 1.0 / static_cast<double>(frameGuess)) /
            (1.0 / static_cast<double>(frameGuess)) >
        0.01) {
        lowcounts++;
    }
    if (fpsList.size() >= frameGuess * 2) {
        fpsList.pop_back();
        frameList.pop_back();
        inputList.pop_back();
        waitList.pop_back();
    }
    auto min = *std::min_element(fpsList.begin(), fpsList.end());
    auto max = *std::max_element(fpsList.begin(), fpsList.end());
    auto maxIndex = std::distance(
        fpsList.begin(), std::max_element(fpsList.begin(), fpsList.end()));
    auto siz = fpsList.size();
    auto avg = std::accumulate(fpsList.begin(), fpsList.end(), 0.0) / siz;
    auto varianceFunc = [&avg, &siz](double accumulator, double val) {
        return accumulator + (val - avg) * (val - avg);
    };
    auto std = sqrt(
        std::accumulate(fpsList.begin(), fpsList.end(), 0.0, varianceFunc) /
        siz);
    auto frameAvg = std::accumulate(frameList.begin(), frameList.end(), 0.0) /
                    frameList.size();
    auto frameMax = frameList.begin();
    std::advance(frameMax, maxIndex);
    auto inputMax = inputList.begin();
    std::advance(inputMax, maxIndex);
    auto waitMax = waitList.begin();
    std::advance(waitMax, maxIndex);
    if (frames % (frameGuess * 2) == 0) {
        Debug::log(
            "frame total average: %f, frame render average %f, std dev "
            "%f, lowest FPS %f, highest FPS %f, highest FPS render "
            "time %f, highest FPS input time %f, highest FPS wait time "
            "%f, lag frames %d\n",
            avg, frameAvg, std, 1.0 / max, 1.0 / min, *frameMax, *inputMax,
            *waitMax, lowcounts);
        lowcounts = 0;
    }
    frames++;
#endif
    //}
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    static const Mouse::Code mcodes[4] = {Mouse::NONE, Mouse::LMB, Mouse::MMB,
                                          Mouse::RMB};
    int mc = 0;
    while (SDL_PollEvent(event)) {
        switch (event->type) {
            case SDL_EVENT_QUIT: {
                gEditor->onExitProgram();
                return SDL_APP_SUCCESS;
            }
            case SDL_EVENT_WINDOW_MOUSE_ENTER:
            case SDL_EVENT_WINDOW_FOCUS_GAINED: {
                myMouseState.reset();
                myKeyState.reset();
                break;
            }
            case SDL_EVENT_WINDOW_MOUSE_LEAVE:
            case SDL_EVENT_WINDOW_FOCUS_LOST: {
                myEvents.addWindowInactive();
                myMouseState.reset();
                myKeyState.reset();
                break;
            }
            case SDL_EVENT_WINDOW_RESIZED: {
                SDL_Window* win = SDL_GetWindowFromID(event->window.windowID);
                vec2i next = {event->window.data1, event->window.data2};
                if (next.x > 0 && next.y > 0) mySize = next;
                break;
            }
            case SDL_EVENT_MOUSE_MOTION: {
                if (myIsInsideMessageLoop) {
                    float x, y;
                    SDL_GetMouseState(&x, &y);
                    myMousePos.x = static_cast<int>(x);
                    myMousePos.y = static_cast<int>(y);
                    myEvents.addMouseMove(myMousePos.x, myMousePos.y);
                }
                break;
            }
            case SDL_EVENT_MOUSE_WHEEL: {
                if (myIsInsideMessageLoop) {
                    bool up = (event->wheel.direction == SDL_MOUSEWHEEL_NORMAL);
                    myEvents.addMouseScroll(up, event->wheel.x, event->wheel.y,
                                            gSystem->getKeyFlags());
                }
                break;
            }
            case SDL_EVENT_KEY_DOWN: {
                if (myIsInsideMessageLoop) {
                    Key::Code kc = gSystem->translateKeyCode(event->key.key);
                    gSystem->handleKeyPress(kc, event->key.repeat);
                }
                break;
            }
            case SDL_EVENT_KEY_UP: {
                if (myIsInsideMessageLoop) {
                    Key::Code kc = gSystem->translateKeyCode(event->key.key);
                    myEvents.addKeyRelease(kc, gSystem->getKeyFlags());
                    myKeyState.reset(kc);
                }
                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event->button.button == SDL_BUTTON_LEFT) {
                    SDL_CaptureMouse(true);
                    if (myIsInsideMessageLoop) {
                        float x, y;
                        SDL_GetMouseState(&x, &y);
                        myEvents.addMousePress(Mouse::LMB, static_cast<int>(x),
                                               static_cast<int>(y),
                                               gSystem->getKeyFlags(), false);
                        myMouseState.set(Mouse::LMB);
                    }
                }
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event->button.button == SDL_BUTTON_LEFT) {
                    SDL_CaptureMouse(false);
                    SDL_CaptureMouse(false);
                    if (myIsInsideMessageLoop) {
                        float x, y;
                        SDL_GetMouseState(&x, &y);
                        myEvents.addMouseRelease(
                            Mouse::LMB, static_cast<int>(x),
                            static_cast<int>(y), gSystem->getKeyFlags());
                        myMouseState.reset(Mouse::LMB);
                    }
                }
                break;
            case SDL_EVENT_TEXT_INPUT: {
                auto wp = event->text.text[0];
                if (wp >= 32)
                    myInput.push_back(wp);
                else if (wp == '\r')
                    myInput.push_back('\n');
                break;
            }
            case SDL_EVENT_DROP_BEGIN: {
                if (myIsInsideMessageLoop) {
                    droppedFiles.clear();
                }
                break;
            }
            case SDL_EVENT_DROP_FILE: {
                if (myIsInsideMessageLoop) {
                    droppedFiles.push_back(event->drop.data);
                }
                break;
            }
            case SDL_EVENT_DROP_COMPLETE: {
                if (myIsInsideMessageLoop && !droppedFiles.empty()) {
                    std::vector<const char*> filePtrs;
                    for (const auto& file : droppedFiles) {
                        filePtrs.push_back(file.c_str());
                    }
                    myEvents.addFileDrop(filePtrs.data(),
                                         static_cast<int>(filePtrs.size()),
                                         event->drop.x, event->drop.y);
                }
                break;
            }
                // case WM_COMMAND: {
                //     if (myIsInsideMessageLoop) {
                //         gEditor->onMenuAction(LOWORD(wp));
                //     }
                //     break;
                // }
        };  // end of message switch.
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    delete static_cast<SystemImpl*>(gSystem);
    ApplicationEnd();

#ifdef CRTDBG_MAP_ALLOC
    _CrtDumpMemoryLeaks();
#endif
}
