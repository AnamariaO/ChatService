#pragma once

#include <csignal>
#include <iostream>
#include <cstdlib>

template<typename T>
class SignalHandler {
public:
    static void registerHandler(T* instance, void (T::*stopFunc)()) {
        object = instance;
        stopMethod = stopFunc;
        std::signal(SIGINT, handleSignal);
    }

private:
    static inline T* object = nullptr;
    static inline void (T::*stopMethod)() = nullptr;

    static void handleSignal(int signum) {
        std::cout << "\n[System] Caught signal " << signum << ". Cleaning up...\n";
        if (object && stopMethod) {
            (object->*stopMethod)();
        }
        std::_Exit(EXIT_SUCCESS); // Immediate exit to avoid destructor race conditions
    }
};