#pragma once
#include <queue>
#include <unordered_map>
#include <GLFW/glfw3.h>
#include <cassert>

//TODO change to SDL2 ? matches better with explicit event handling scheme
struct Input {
    //window resize events handled by 
    struct Event {
        enum EventType_t {
            KEY_DOWN,  //includeing mouse keys
            KEY_RELEASE, //includeing mouse keys
            KEY_REPEAT, //button held down until repeat
            JOYSTICK_CONNECT,
            CURSOR_MOVE,
            CURSUR_LEAVE_OR_ARRIVE,
            SCROLL
        };
        EventType_t type;
        //don't need scancode ?
        union {
            int key;
            int jid; //id of joystick
            int cursorEntered;
            double xPos;
            double xOffset; //for scroll, usually unneded 
        };
        union {
            int mods;
            int connection; //joysitck connection, ether connect or disconnect
            double yPos;
            double yOffset; //actual scroll input for mouse
        };

        Event(EventType_t _type, int _keyOrJID, int _modsOrConnection) : type(_type), key(_keyOrJID), mods(_modsOrConnection) {}
        Event(EventType_t _type, double _xPosOrOffset, double _yPosOrOffset) : type(_type), xPos(_xPosOrOffset), yPos(_yPosOrOffset) {}
        Event(EventType_t _type, int _cursorEntered) : type(_type), cursorEntered(_cursorEntered), mods(0) {}

        bool shiftHeld() {
            assert(type == KEY_DOWN || type == KEY_RELEASE);
            return (mods & GLFW_MOD_SHIFT);

        }

        bool altHeld() {
            assert(type == KEY_DOWN || type == KEY_RELEASE);
            return (mods & GLFW_MOD_ALT);

        }

        //what is super key?
        bool superHeld() {
            assert(type == KEY_DOWN || type == KEY_RELEASE);
            return (mods & GLFW_MOD_ALT);
        }

        bool capsLockOn() {
            assert(type == KEY_DOWN || type == KEY_RELEASE);
            return (mods & GLFW_MOD_CAPS_LOCK);
        }

        bool numLockOn() {
            assert(type == KEY_DOWN || type == KEY_RELEASE);
            return (mods & GLFW_MOD_NUM_LOCK);
        }
    };

    enum ActionType_t : uint32_t {
        LEFT_CLICK = 0,
        RIGHT_CLICK = 1,
        UP = 2,
        DOWN = 3,
        LEFT = 4,
        RIGHT = 5,
        CURSOR_LEFT = 6,
        CURSUR_RIGHT = 7,
        TYPE = 8,
        ACCEPT = 9,
        BACK = 10, //including backspace
        DEBUG_CONSOLE = 11 //TODO remove ? keep as easter egg ?
    };

    //map from glfw inputs to actions
    static std::unordered_map<int, ActionType_t> actionMap;

    static std::queue<Event> inputEventsQueue;

    //callbacks given to GLFW, just add apporpriate event to queue, which will appropriately be handled in mainLoop
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void joystickCallback(int jid, int event);
    static void cursorEnterCallback(GLFWwindow* window, int entered);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
};