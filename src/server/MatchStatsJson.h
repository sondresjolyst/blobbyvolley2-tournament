#pragma once

#include <nlohmann/json.hpp>

#include "MatchStats.h"

// the statsJson contract of the tournament site
nlohmann::json statsJson(const MatchStats& s, int player1, int player2);
