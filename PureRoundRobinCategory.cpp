/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2017  Volker Knollmann
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PureRoundRobinCategory.h"
#include "KO_Config.h"
#include "CatRoundStatus.h"
#include "RankingEntry.h"
#include "RankingMngr.h"
#include "assert.h"
#include "BracketGenerator.h"
#include "MatchMngr.h"

#include <QDebug>

using namespace SqliteOverlay;

namespace QTournament
{

  PureRoundRobinCategory::PureRoundRobinCategory(TournamentDB* db, int rowId)
  : Category(db, rowId)
  {
  }

//----------------------------------------------------------------------------

  PureRoundRobinCategory::PureRoundRobinCategory(TournamentDB* db, SqliteOverlay::TabRow row)
  : Category(db, row)
  {
  }

  //----------------------------------------------------------------------------

  int PureRoundRobinCategory::getRoundCountPerIteration() const
  {
    OBJ_STATE stat = getState();
    if ((stat == STAT_CAT_CONFIG) || (stat == STAT_CAT_FROZEN))
    {
      return -1;   // category not yet fully configured; can't calc rounds
    }

    PlayerPairList allPairs = getPlayerPairs();
    int nPairs = allPairs.size();

    return ((nPairs % 2) == 0) ? (nPairs-1) : nPairs;
  }

  //----------------------------------------------------------------------------

  int PureRoundRobinCategory::getIterationCount() const
  {
    return getParameter_int(ROUND_ROBIN_ITERATIONS);  // simple wrapper function
  }

  //----------------------------------------------------------------------------

  unique_ptr<PureRoundRobinCategory> PureRoundRobinCategory::getFromGenericCat(const Category& cat)
  {
    MATCH_SYSTEM msys = cat.getMatchSystem();

    return (msys == ROUND_ROBIN) ? unique_ptr<PureRoundRobinCategory>(new PureRoundRobinCategory(cat.db, cat.row)) : nullptr;
  }

  //----------------------------------------------------------------------------

  ModMatchResult PureRoundRobinCategory::canModifyMatchResult(const Match& ma) const
  {
    // the match has to be in FINISHED state
    if (ma.getState() != STAT_MA_FINISHED) return ModMatchResult::NotPossible;

    // if this match does not belong to us, we're not responsible
    if (ma.getCategory().getMatchSystem() != ROUND_ROBIN) return ModMatchResult::NotPossible;

    // in round robins, we can modify the results of all matches
    // at any time
    return ModMatchResult::WinnerLoser;
  }

  //----------------------------------------------------------------------------

  ModMatchResult PureRoundRobinCategory::modifyMatchResult(const Match& ma, const MatchScore& newScore) const
  {
    ModMatchResult mmr = canModifyMatchResult(ma);
    if (mmr != ModMatchResult::WinnerLoser) return mmr;

    //
    // if we can modify the score, we can also change winner/loser
    // information. Thus, we can set any score that's presented to us
    // without further checks
    //

    // store the old match result
    MatchScore oldScore = *(ma.getScore());  // is guaranteed to be != nullptr

    // update the match
    MatchMngr mm{db};
    ERR e = mm.updateMatchScore(ma, newScore, true);
    if (e != OK) return ModMatchResult::NotPossible;

    // update all affected ranking entries
    RankingMngr rm{db};
    e = rm.updateRankingsAfterMatchResultChange(ma, oldScore);
    return (e == OK) ? ModMatchResult::ModDone : ModMatchResult::NotPossible;
  }

//----------------------------------------------------------------------------

  ERR PureRoundRobinCategory::canFreezeConfig()
  {
    if (getState() != STAT_CAT_CONFIG)
    {
      return CONFIG_ALREADY_FROZEN;
    }
    
    // make sure there no unpaired players in singles or doubles
    if ((getMatchType() != SINGLES) && (hasUnpairedPlayers()))
    {
      return UNPAIRED_PLAYERS;
    }
    
    // make sure we have at least three players
    PlayerPairList pp = getPlayerPairs();
    if (pp.size() < 3)
    {
      return INVALID_PLAYER_COUNT;
    }
    
    return OK;
  }

//----------------------------------------------------------------------------

  bool PureRoundRobinCategory::needsInitialRanking()
  {
    return false;
  }

//----------------------------------------------------------------------------

  bool PureRoundRobinCategory::needsGroupInitialization()
  {
    return false;
  }

//----------------------------------------------------------------------------

  ERR PureRoundRobinCategory::prepareFirstRound(ProgressQueue *progressNotificationQueue)
  {
    if (getState() != STAT_CAT_IDLE) return WRONG_STATE;

    MatchMngr mm{db};

    // make sure we have not been called before; to this end, just
    // check that there have no matches been created for us so far
    auto allGrp = mm.getMatchGroupsForCat(*this);
    // do not return an error here, because obviously we have been
    // called successfully before and we only want to avoid
    // double initialization
    if (allGrp.size() != 0) return OK;

    // alright, this is a virgin category. Generate
    // all round robin matches at once
    PlayerPairList allPairs = getPlayerPairs();
    if (progressNotificationQueue != nullptr)
    {
      int nPlayers = allPairs.size();
      int nMatches = (nPlayers / 2) * calcTotalRoundsCount();
      progressNotificationQueue->reset(nMatches);
    }
    int iterationCount = getIterationCount();
    int roundsPerIteration = getRoundCountPerIteration();
    ERR e;
    for (int i=0; i < iterationCount; ++i)
    {
      int firstRoundNum = (i * roundsPerIteration) + 1;
      e = generateGroupMatches(allPairs, GROUP_NUM__ITERATION, firstRoundNum, progressNotificationQueue);
      if (e != OK) return e;
    }
    return OK;
  }

//----------------------------------------------------------------------------

  int PureRoundRobinCategory::calcTotalRoundsCount() const
  {
    int roundsPerIteration = getRoundCountPerIteration();
    if (roundsPerIteration < 1) return -1;  // category not yet configured

    // done
    return getIterationCount() * roundsPerIteration;
  }

//----------------------------------------------------------------------------

  // this return a function that should return true if "a" goes before "b" when sorting. Read:
  // return a function that return true true if the score of "a" is better than "b"
  std::function<bool (RankingEntry& a, RankingEntry& b)> PureRoundRobinCategory::getLessThanFunction()
  {
    return [](RankingEntry& a, RankingEntry& b) {
      // first criterion: delta between won and lost matches
      tuple<int, int, int, int> matchStatsA = a.getMatchStats();
      tuple<int, int, int, int> matchStatsB = b.getMatchStats();
      int deltaA = get<0>(matchStatsA) - get<2>(matchStatsA);
      int deltaB = get<0>(matchStatsB) - get<2>(matchStatsB);
      if (deltaA > deltaB) return true;
      if (deltaA < deltaB) return false;

      // second criteria: delta between won and lost games
      tuple<int, int, int> gameStatsA = a.getGameStats();
      tuple<int, int, int> gameStatsB = b.getGameStats();
      deltaA = get<0>(gameStatsA) - get<1>(gameStatsA);
      deltaB = get<0>(gameStatsB) - get<1>(gameStatsB);
      if (deltaA > deltaB) return true;
      if (deltaA < deltaB) return false;

      // second criteria: delta between won and lost points
      tuple<int, int> pointStatsA = a.getPointStats();
      tuple<int, int> pointStatsB = b.getPointStats();
      deltaA = get<0>(pointStatsA) - get<1>(pointStatsA);
      deltaB = get<0>(pointStatsB) - get<1>(pointStatsB);
      if (deltaA > deltaB) return true;
      if (deltaA < deltaB) return false;

      // TODO: add a direct comparison as additional criteria?

      // Default: "a" is not strictly better than "b", so we return false
      return false;
    };
  }

//----------------------------------------------------------------------------

  ERR PureRoundRobinCategory::onRoundCompleted(int round)
  {
    RankingMngr rm{db};
    ERR err;

    rm.createUnsortedRankingEntriesForLastRound(*this, &err);
    if (err != OK) return err;  // shouldn't happen
    rm.sortRankingEntriesForLastRound(*this, &err);
    if (err != OK) return err;  // shouldn't happen

    return OK;
  }

//----------------------------------------------------------------------------

  PlayerPairList PureRoundRobinCategory::getRemainingPlayersAfterRound(int round, ERR* err) const
  {
    // No knock-outs, never
    if (err != nullptr) *err = OK;
    return getPlayerPairs();
  }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}
