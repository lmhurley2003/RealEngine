#include "program.hpp"
#include "commandArgs.hpp"

Program curProgram() { return  Program(parameters, {}, {}, { {MAIN_RENDER, {{VERTEX, "triTestVert"}, {FRAGMENT, "triTestFrag"}}}}); };

