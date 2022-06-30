/// \file game_logic.h
/// Interface for playing qix.

#ifndef GAME_LOGIC_H_INCLUDED
#define GAME_LOGIC_H_INCLUDED

#define _POSIX_C_SOURCE 200112L

#include "image.h"
#include "mapping.h"

#include <stdbool.h>

/// Initializes game logic. Needs to be called when before new game starts.
void init_gamelogic();

/// Starts a new game, needs to be initialized first. If not initialized, exits.
/// Requires initialization of peripherals called by memory_map_boot(),
/// otherwise behaviour is undefined.
void start_new_game();

#endif // GAME_LOGIC_H_INCLUDED
