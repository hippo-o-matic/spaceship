#pragma once

#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "window.h"
#include "sprite.h"
#include "camera2d.h"

#include "game/tileGrid.h"
#include "editor/gridEditor.h"
#include "game/cloader.h"
#include "game/ship.h"

extern bool show_debug_menu;
TileGrid gridTest();