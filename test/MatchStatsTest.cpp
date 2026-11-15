#include <boost/test/unit_test.hpp>

#include <vector>

#include "DuelMatch.h"
#include "FileSystem.h"
#include "InputSource.h"
#include "PlayerInput.h"
#include "server/MatchStats.h"

namespace
{
	const std::vector<MatchEvent> noEvents;

	void step(MatchStatsCounter& c, float x, bool inPlay, int tl, int tr, const std::vector<MatchEvent>& ev = noEvents)
	{
		StepView v{ev, x, inPlay, tl, tr};
		c.observe(v);
	}

	struct DataFixture
	{
		DataFixture() : mFS("./test")
		{
			mFS.addToSearchPath(BLOBBY_TEST_DATA_DIR);
		}
		FileSystem mFS;
	};
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

BOOST_FIXTURE_TEST_CASE( counts_a_real_match, DataFixture )
{
	DuelMatch match(false, "default.lua", 15);
	auto left = std::make_shared<InputSource>();
	auto right = std::make_shared<InputSource>();
	match.setInputSources(left, right);

	MatchStatsCounter counter;
	uint32_t seed = 12345;
	PlayerInput leftInput;
	PlayerInput rightInput;
	for(int i = 0; i < 30000 && match.getScore(LEFT_PLAYER) + match.getScore(RIGHT_PLAYER) < 5; ++i)
	{
		if(i % 15 == 0)
		{
			seed = seed * 1664525u + 1013904223u;
			const int bits = seed >> 24;
			leftInput = PlayerInput((bits & 3) == 1, (bits & 3) == 2, (bits & 4) != 0);
			rightInput = PlayerInput((bits & 24) == 8, (bits & 24) == 16, (bits & 32) != 0);
		}
		left->setInput(leftInput);
		right->setInput(rightInput);
		match.step();
		counter.observe({match.getEvents(), match.getBallPosition().x,
			match.getBallActive() && !match.getBallDown(),
			match.getTouches(LEFT_PLAYER), match.getTouches(RIGHT_PLAYER)});
	}

	const MatchStats s = counter.snapshot();
	BOOST_CHECK_EQUAL(match.getScore(LEFT_PLAYER) + match.getScore(RIGHT_PLAYER), 5);
	BOOST_CHECK_EQUAL(s.rallies, 5);
	BOOST_CHECK_GT(s.left.touches + s.right.touches, 0);
	BOOST_CHECK_GT(s.left.ballSteps, 0);
	BOOST_CHECK_GT(s.right.ballSteps, 0);
}

BOOST_AUTO_TEST_SUITE_END()
