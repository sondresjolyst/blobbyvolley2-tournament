#pragma once

#include <vector>

#include <nlohmann/json.hpp>

#include "MatchEvents.h"

struct PlayerStats
{
	long long ballSteps = 0;
	int touches = 0;
	int errors = 0;
	int netHits = 0;
};

struct MatchStats
{
	PlayerStats left, right;
	int rallies = 0;
	int longestRally = 0;
	int rallyTouches = 0;
};

// what the counter needs to know about one physics step
struct StepView
{
	const std::vector<MatchEvent>& events;
	float ballX;
	bool inPlay;
	int touchesLeft;
	int touchesRight;
};

class MatchStatsCounter
{
public:
	void observe(const StepView& step);
	MatchStats snapshot() const;

private:
	MatchStats mStats;
	int mLastTouchesLeft = 0;
	int mLastTouchesRight = 0;
	int mRallyTouches = 0;
};

// the statsJson contract of the tournament site
nlohmann::json statsJson(const MatchStats& s, int player1, int player2);
