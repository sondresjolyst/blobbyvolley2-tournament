#include <boost/test/unit_test.hpp>

#include <vector>

#include "server/MatchStats.h"

namespace
{
	const std::vector<MatchEvent> noEvents;

	void step(MatchStatsCounter& c, float x, bool inPlay, int tl, int tr, const std::vector<MatchEvent>& ev = noEvents)
	{
		StepView v{ev, x, inPlay, tl, tr};
		c.observe(v);
	}
}

BOOST_AUTO_TEST_SUITE( match_stats )

BOOST_AUTO_TEST_CASE( waiting_before_serve_counts_no_possession )
{
	MatchStatsCounter c;
	for (int i = 0; i < 500; ++i)
		step(c, 100.f, false, 0, 0);
	BOOST_CHECK_EQUAL(c.snapshot().left.ballSteps, 0);
}

BOOST_AUTO_TEST_CASE( possession_counts_in_play_steps_per_side )
{
	MatchStatsCounter c;
	for (int i = 0; i < 30; ++i)
		step(c, 100.f, true, 0, 0);
	for (int i = 0; i < 20; ++i)
		step(c, 600.f, true, 0, 0);
	BOOST_CHECK_EQUAL(c.snapshot().left.ballSteps, 30);
	BOOST_CHECK_EQUAL(c.snapshot().right.ballSteps, 20);
}

BOOST_AUTO_TEST_CASE( touches_follow_the_rule_counter )
{
	MatchStatsCounter c;
	step(c, 100.f, true, 0, 0);
	step(c, 100.f, true, 1, 0);
	step(c, 100.f, true, 1, 0);
	step(c, 100.f, true, 2, 0);
	step(c, 600.f, true, 0, 1);
	BOOST_CHECK_EQUAL(c.snapshot().left.touches, 2);
	BOOST_CHECK_EQUAL(c.snapshot().right.touches, 1);
}

BOOST_AUTO_TEST_CASE( error_ends_a_rally )
{
	MatchStatsCounter c;
	for (int i = 1; i <= 3; ++i)
		step(c, 100.f, true, i, 0);
	std::vector<MatchEvent> e1{MatchEvent(MatchEvent::PLAYER_ERROR, RIGHT_PLAYER)};
	step(c, 600.f, true, 3, 0, e1);
	step(c, 100.f, false, 0, 0);
	for (int i = 1; i <= 5; ++i)
		step(c, 100.f, true, i, 0);
	std::vector<MatchEvent> e2{MatchEvent(MatchEvent::PLAYER_ERROR, LEFT_PLAYER)};
	step(c, 100.f, true, 5, 0, e2);
	const MatchStats s = c.snapshot();
	BOOST_CHECK_EQUAL(s.rallies, 2);
	BOOST_CHECK_EQUAL(s.longestRally, 5);
	BOOST_CHECK_EQUAL(s.rallyTouches, 8);
	BOOST_CHECK_EQUAL(s.right.errors, 1);
	BOOST_CHECK_EQUAL(s.left.errors, 1);
}

BOOST_AUTO_TEST_CASE( net_top_goes_to_the_ball_side )
{
	MatchStatsCounter c;
	std::vector<MatchEvent> e{MatchEvent(MatchEvent::BALL_HIT_NET_TOP, NO_PLAYER)};
	step(c, 450.f, true, 0, 0, e);
	BOOST_CHECK_EQUAL(c.snapshot().right.netHits, 1);
	BOOST_CHECK_EQUAL(c.snapshot().left.netHits, 0);
}

BOOST_AUTO_TEST_CASE( stats_json_matches_the_contract )
{
	MatchStats s;
	s.left.ballSteps = 4120;
	s.left.touches = 31;
	s.right.errors = 4;
	s.rallies = 14;
	s.longestRally = 9;
	s.rallyTouches = 58;
	const nlohmann::json j = statsJson(s, 12, 7);
	BOOST_REQUIRE_EQUAL(j["players"].size(), 2u);
	BOOST_CHECK_EQUAL(j["players"][0]["playerId"], 12);
	BOOST_CHECK_EQUAL(j["players"][1]["playerId"], 7);
	BOOST_CHECK_EQUAL(j["players"][0]["ballSteps"], 4120);
	BOOST_CHECK_EQUAL(j["players"][0]["touches"], 31);
	BOOST_CHECK_EQUAL(j["players"][1]["errors"], 4);
	BOOST_CHECK_EQUAL(j["players"][0]["netHits"], 0);
	BOOST_CHECK_EQUAL(j["players"][0].size(), 5u);
	BOOST_CHECK_EQUAL(j.size(), 4u);
	BOOST_CHECK_EQUAL(j["rallies"], 14);
	BOOST_CHECK_EQUAL(j["longestRally"], 9);
	BOOST_CHECK_EQUAL(j["rallyTouches"], 58);
}

BOOST_AUTO_TEST_SUITE_END()
