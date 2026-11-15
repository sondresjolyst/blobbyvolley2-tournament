#include "MatchStatsJson.h"

namespace
{
	nlohmann::json playerJson(int playerId, const PlayerStats& p)
	{
		return nlohmann::json{
			{"playerId", playerId},
			{"ballSteps", p.ballSteps},
			{"touches", p.touches},
			{"errors", p.errors},
			{"netHits", p.netHits}
		};
	}
}

nlohmann::json statsJson(const MatchStats& s, int player1, int player2)
{
	nlohmann::json j;
	j["players"] = nlohmann::json::array({playerJson(player1, s.left), playerJson(player2, s.right)});
	j["rallies"] = s.rallies;
	j["longestRally"] = s.longestRally;
	j["rallyTouches"] = s.rallyTouches;
	return j;
}
