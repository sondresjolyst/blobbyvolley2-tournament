#include "MatchStats.h"

#include "GameConstants.h"

void MatchStatsCounter::observe(const StepView& step)
{
	const bool ballLeft = step.ballX < NET_POSITION_X;
	PlayerStats& ballSide = ballLeft ? mStats.left : mStats.right;

	if (step.inPlay)
		ballSide.ballSteps++;

	// the rule counter drops back to 0 when the other side touches, so only a rise is a touch
	int newTouches = 0;
	if (step.touchesLeft > mLastTouchesLeft)
	{
		const int rise = step.touchesLeft - mLastTouchesLeft;
		mStats.left.touches += rise;
		newTouches += rise;
	}
	if (step.touchesRight > mLastTouchesRight)
	{
		const int rise = step.touchesRight - mLastTouchesRight;
		mStats.right.touches += rise;
		newTouches += rise;
	}
	mLastTouchesLeft = step.touchesLeft;
	mLastTouchesRight = step.touchesRight;
	mRallyTouches += newTouches;

	for (const MatchEvent& e : step.events)
	{
		PlayerStats* side = e.side == LEFT_PLAYER ? &mStats.left : (e.side == RIGHT_PLAYER ? &mStats.right : nullptr);
		switch (e.event)
		{
		case MatchEvent::BALL_HIT_NET:
			if (side)
				side->netHits++;
			break;
		case MatchEvent::BALL_HIT_NET_TOP:
			ballSide.netHits++;
			break;
		case MatchEvent::PLAYER_ERROR:
			if (side)
				side->errors++;
			mStats.rallies++;
			mStats.rallyTouches += mRallyTouches;
			if (mRallyTouches > mStats.longestRally)
				mStats.longestRally = mRallyTouches;
			mRallyTouches = 0;
			break;
		default:
			break;
		}
	}
}

MatchStats MatchStatsCounter::snapshot() const
{
	return mStats;
}
