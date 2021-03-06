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

#include <stdexcept>
#include <algorithm>

#include <SqliteOverlay/Transaction.h>
#include <QDebug>

#include "RankingMngr.h"
#include "CatRoundStatus.h"
#include "RankingEntry.h"
#include "Match.h"
#include "Score.h"
#include "MatchMngr.h"

using namespace SqliteOverlay;

namespace QTournament
{

  RankingMngr::RankingMngr(TournamentDB* _db)
  : TournamentDatabaseObjectManager(_db, TAB_RANKING)
  {
  }

//----------------------------------------------------------------------------

  RankingEntryList RankingMngr::createUnsortedRankingEntriesForLastRound(const Category &cat, ERR *err, PlayerPairList _ppList, bool reset)
  {
    // determine the round we should create the entries for
    CatRoundStatus crs = cat.getRoundStatus();
    int lastRound = crs.getFinishedRoundsCount();
    if (lastRound < 1)
    {
      if (err != nullptr) *err = ROUND_NOT_FINISHED;
      return RankingEntryList();
    }

    // establish the list of player pairs for that the ranking shall
    // be created; it's either ALL pairs or a caller-provided list
    PlayerPairList ppList;
    if (_ppList.empty())
    {
      ppList = cat.getPlayerPairs();
    } else {
      ppList = _ppList;
    }

    // make sure that all matches for these player pairs have
    // valid results. We need to check this in a separate loop
    // to avoid that the ranking entries have already been
    // halfway written to the database when we encounter an invalid
    // match
    MatchMngr mm{db};
    for (PlayerPair pp : ppList)
    {
      unique_ptr<Match> ma = mm.getMatchForPlayerPairAndRound(pp, lastRound);
      if (ma != nullptr)
      {
        ERR e;
        unique_ptr<MatchScore> score = ma->getScore(&e);
        if (e != OK)
        {
          if (err != nullptr) *err = e;
          return RankingEntryList();
        }
        if (score == nullptr)
        {
          if (err != nullptr) *err = NO_MATCH_RESULT_SET;
          return RankingEntryList();
        }
      }
    }

    //
    // Okay, we can be pretty sure that the rest of this method
    // succeeds and that we leave the database in a consistent state
    //

    // lock the database before writing
    DbLockHolder lh{db, DatabaseAccessRoles::MainThread};

    // iterate over the player pair list and create entries
    // based on match result and, possibly, previous ranking entries
    RankingEntryList result;
    for (PlayerPair pp : ppList)
    {
      // prepare the values for the new entry
      int wonMatches = 0;
      int drawMatches = 0;
      int lostMatches = 0;
      int wonGames = 0;
      int lostGames = 0;
      int wonPoints = 0;
      int lostPoints = 0;

      // get match results, if the player played in this round
      // (maybe the player had a bye; in this case we skip this section)
      unique_ptr<Match> ma = mm.getMatchForPlayerPairAndRound(pp, lastRound);
      if (ma != nullptr)
      {
        // determine whether our pp is player 1 or player 2
        int pp1Id = ma->getPlayerPair1().getPairId();
        int playerNum = (pp1Id == pp.getPairId()) ? 1 : 2;

        // create column values for match data
        ERR e;
        unique_ptr<MatchScore> score = ma->getScore(&e);
        if (score == nullptr) qDebug() << "!!! NULL !!!";
        if (e != OK) qDebug() << e;
        wonMatches = (score->getWinner() == playerNum) ? 1 : 0;
        lostMatches = (score->getLoser() == playerNum) ? 1 : 0;
        drawMatches = (score->getWinner() == 0) ? 1 : 0;

        // create column values for game data
        tuple<int, int> gameSum = score->getGameSum();
        int gamesTotal = get<0>(gameSum) + get<1>(gameSum);
        wonGames = (playerNum == 1) ? get<0>(gameSum) : get<1>(gameSum);
        lostGames = gamesTotal - wonGames;

        // create column values for point data
        tuple<int, int> scoreSum = score->getScoreSum();
        wonPoints = (playerNum == 1) ? get<0>(scoreSum) : get<1>(scoreSum);
        lostPoints = score->getPointsSum() - wonPoints;
      }

      // add values from previous round, if required
      unique_ptr<RankingEntry> prevEntry = nullptr;
      if (!reset)
      {
        // the next call may return nullptr, but this is fine.
        // it just means that there is no old entry to build upon
        // and thus we start with the scoring at zero
        prevEntry = getRankingEntry(pp, lastRound-1);
      }

      if (prevEntry != nullptr)
      {
        tuple<int, int, int, int> maStat = prevEntry->getMatchStats();
        wonMatches += get<0>(maStat);
        drawMatches += get<1>(maStat);
        lostMatches += get<2>(maStat);

        tuple<int, int, int> gameStat = prevEntry->getGameStats();
        wonGames += get<0>(gameStat);
        lostGames += get<1>(gameStat);

        tuple<int, int> pointStat = prevEntry->getPointStats();
        wonPoints += get<0>(pointStat);
        lostPoints += get<1>(pointStat);
      }

      // determine the match group number for this entry
      int grpNum;
      if (ma != nullptr)
      {
        // easiest and most likely case
        grpNum = ma->getMatchGroup().getGroupNumber();
      } else {
        // hmmmm, we need to determine the group number of a
        // playerPair that hasn't played in this round

        // case 1:
        // we are in some sort of round-robin round with
        // multiple match groups. In this case, the group
        // number can be derived directly from the PlayerPair
        if (mm.getMatchGroupsForCat(cat, lastRound).size() > 1)
        {
          grpNum = pp.getPairsGroupNum();
        } else {

          // case 2:
          // we are either in a round-robin phase with only
          // one group or in a KO-round or similar. So we have
          // only one match group. Thus, we can derive the
          // group number from the match group
          grpNum = mm.getMatchGroupsForCat(cat, lastRound).at(0).getGroupNumber();
        }
      }

      // prep the complete data set for the entry,
      // but leave the "rank" column empty
      ColumnValueClause cvc;
      cvc.addIntCol(RA_MATCHES_WON, wonMatches);
      cvc.addIntCol(RA_MATCHES_DRAW, drawMatches);
      cvc.addIntCol(RA_MATCHES_LOST, lostMatches);
      cvc.addIntCol(RA_GAMES_WON, wonGames);
      cvc.addIntCol(RA_GAMES_LOST, lostGames);
      cvc.addIntCol(RA_POINTS_WON, wonPoints);
      cvc.addIntCol(RA_POINTS_LOST, lostPoints);
      cvc.addIntCol(RA_PAIR_REF, pp.getPairId());
      cvc.addIntCol(RA_ROUND, lastRound);
      cvc.addIntCol(RA_CAT_REF, cat.getId());  // eases searching, but is redundant information
      cvc.addIntCol(RA_GRP_NUM, grpNum); // eases searching, but is redundant information

      // create the new entry and add an instance
      // of the entry to the result list
      int newId = tab->insertRow(cvc);
      result.push_back(RankingEntry(db, newId));
    }

    if (err != nullptr) *err = OK;
    return result;
  }

//----------------------------------------------------------------------------

  unique_ptr<RankingEntry> RankingMngr::getRankingEntry(const PlayerPair &pp, int round) const
  {
    WhereClause wc;
    wc.addIntCol(RA_CAT_REF, pp.getCategory(db)->getId());
    wc.addIntCol(RA_PAIR_REF, pp.getPairId());
    wc.addIntCol(RA_ROUND, round);

    return getSingleObjectByWhereClause<RankingEntry>(wc);
  }

//----------------------------------------------------------------------------

  RankingEntryListList RankingMngr::getSortedRanking(const Category &cat, int round) const
  {
    // make sure we have ranking entries
    WhereClause wc;
    wc.addIntCol(RA_CAT_REF, cat.getId());
    wc.addIntCol(RA_ROUND, round);
    RankingEntryList rel = getObjectsByWhereClause<RankingEntry>(wc);
    if (rel.empty())
    {
      return RankingEntryListList();
    }

    RankingEntryListList result;

    // derive a list of all match groups we need to
    // retrieve
    //
    // Note: the list of applicable match groups cannot
    // be derived from tnmt->getMatchMngr()->getMatchGroupsForCat(cat, lastRound)
    // because this doesn't fetch group numbers of those
    // round-robin groups that haven't played in this round.
    // This effect occurs if we have different group sizes in the category
    // (i.e., Group 1 has already finished, Group 2 still has to play one round)
    QList<int> applicableMatchGroupNumbers;
    for (RankingEntry re : rel)
    {
      int grpNum = re.getGroupNumber();
      if (!(applicableMatchGroupNumbers.contains(grpNum)))
      {
        applicableMatchGroupNumbers.append(grpNum);
      }
    }
    std::sort(applicableMatchGroupNumbers.begin(), applicableMatchGroupNumbers.end());

    // get separate lists for every match group.
    //
    // In non-round-robin matches, this does no harm because
    // there is only one (artificial) match group in those cases
    for (int grpNum : applicableMatchGroupNumbers)
    {
      WhereClause wc;
      wc.addIntCol(RA_CAT_REF, cat.getId());
      wc.addIntCol(RA_ROUND, round);
      wc.addIntCol(RA_GRP_NUM, grpNum);
      wc.setOrderColumn_Asc(RA_GRP_NUM);
      wc.setOrderColumn_Asc(RA_RANK);

      result.push_back(getObjectsByWhereClause<RankingEntry>(wc));
    }

    return result;
  }

//----------------------------------------------------------------------------

  int RankingMngr::getHighestRoundWithRankingEntryForPlayerPair(const Category& cat, const PlayerPair& pp) const
  {
    WhereClause wc;
    wc.addIntCol(RA_CAT_REF, cat.getId());
    wc.addIntCol(RA_PAIR_REF, pp.getPairId());
    wc.setOrderColumn_Desc(RA_ROUND);

    auto re = getSingleObjectByWhereClause<RankingEntry>(wc);
    if (re == nullptr) return -1;

    return re->getRound();
  }

  //----------------------------------------------------------------------------

  ERR RankingMngr::updateRankingsAfterMatchResultChange(const Match& ma, const MatchScore& oldScore, bool skipSorting) const
  {
    if (ma.getState() != STAT_MA_FINISHED) return WRONG_STATE;

    Category cat = ma.getCategory();
    int catId = cat.getId();
    int firstRoundToModify = ma.getMatchGroup().getRound();

    // determine the score differences (delta) for each affected player pair
    MatchScore newScore = *(ma.getScore());  // is guaranteed to be != nullptr
    tuple<int, int, int> deltaMatches_P1{0,0,0};  // to be added to PlayerPair1
    tuple<int, int, int> deltaMatches_P2{0,0,0};  // to be added to PlayerPair2

    int oldWinner = oldScore.getWinner();
    int newWinner = newScore.getWinner();
    if ((oldWinner == 0) && (newWinner == 1))
    {
      deltaMatches_P1 = tuple<int, int, int>{1, 0, -1};
      deltaMatches_P2 = tuple<int, int, int>{0, 1, -1};
    }
    if ((oldWinner == 0) && (newWinner == 2))
    {
      deltaMatches_P1 = tuple<int, int, int>{0, 1, -1};
      deltaMatches_P2 = tuple<int, int, int>{1, 0, -1};
    }
    if ((oldWinner == 1) && (newWinner == 0))
    {
      deltaMatches_P1 = tuple<int, int, int>{-1, 0, 1};
      deltaMatches_P2 = tuple<int, int, int>{0, -1, 1};
    }
    if ((oldWinner == 2) && (newWinner == 0))
    {
      deltaMatches_P1 = tuple<int, int, int>{0, -1, 1};
      deltaMatches_P2 = tuple<int, int, int>{-1, 0, 1};
    }
    if ((oldWinner == 1) && (newWinner == 2))
    {
      deltaMatches_P1 = tuple<int, int, int>{-1, 1, 0};
      deltaMatches_P2 = tuple<int, int, int>{1, -1, 0};
    }
    if ((oldWinner == 2) && (newWinner == 1))
    {
      deltaMatches_P1 = tuple<int, int, int>{1, -1, 0};
      deltaMatches_P2 = tuple<int, int, int>{-1, 1, 0};
    }

    tuple<int, int> gameSumOld = oldScore.getGameSum();
    tuple<int, int> gameSumNew = newScore.getGameSum();
    int gamesTotalOld = get<0>(gameSumOld) + get<1>(gameSumOld);
    int gamesTotalNew = get<0>(gameSumNew) + get<1>(gameSumNew);

    int deltaWonGamesP1 = -get<0>(gameSumOld) + get<0>(gameSumNew);
    int deltaLostGamesP1 = -(gamesTotalOld - get<0>(gameSumOld)) + (gamesTotalNew - get<0>(gameSumNew));
    int deltaWonGamesP2 = -get<1>(gameSumOld) + get<1>(gameSumNew);
    int deltaLostGamesP2 = -(gamesTotalOld - get<1>(gameSumOld)) + (gamesTotalNew - get<1>(gameSumNew));
    tuple<int, int> deltaGames_P1{deltaWonGamesP1, deltaLostGamesP1};  // to be added to PlayerPair1
    tuple<int, int> deltaGames_P2{deltaWonGamesP2, deltaLostGamesP2};  // to be added to PlayerPair2

    tuple<int, int> scoreSumOld = oldScore.getScoreSum();
    tuple<int, int> scoreSumNew = newScore.getScoreSum();
    int oldWonPoints_P1 = get<0>(scoreSumOld);
    int newWonPoints_P1 = get<0>(scoreSumNew);
    int deltaWonPoints_P1 = newWonPoints_P1 - oldWonPoints_P1;
    int oldLostPoints_P1 = oldScore.getPointsSum() - oldWonPoints_P1;
    int newLostPoints_P1 = newScore.getPointsSum() - newWonPoints_P1;
    int deltaLostPoints_P1 = newLostPoints_P1 - oldLostPoints_P1;
    tuple<int, int> deltaPoints_P1{deltaWonPoints_P1, deltaLostPoints_P1};
    tuple<int, int> deltaPoints_P2{deltaLostPoints_P1, deltaWonPoints_P1};

    // determine who actually is P1 and P2
    int pp1Id = ma.getPlayerPair1().getPairId();
    int pp2Id = ma.getPlayerPair2().getPairId();

    // find the first entry to modify

    // derive the group number of the affected ranking entries
    //
    // we may only modify subsequent entries with the same
    // group number as the initial number. Thus, we prevent
    // a modification of e.g. the ranking entries in a KO-phase
    // after we started modifications in the round robin phase.
    //
    // we get the group number from the first entry of the
    // first player pair to be modified
    WhereClause w;
    w.addIntCol(RA_CAT_REF, catId);
    w.addIntCol(RA_PAIR_REF, pp1Id);
    w.addIntCol(RA_ROUND, firstRoundToModify);
    auto re = getSingleObjectByWhereClause<RankingEntry>(w);
    if (re == nullptr) return OK;  // no ranking entries yet
    int grpNum = re->getGroupNumber();

    //
    // a helper function that does the actual modification
    //
    auto doMod = [&](int pairId, const tuple<int, int, int>& matchDelta,
                     const tuple<int, int>& gamesDelta, const tuple<int, int>& pointsDelta)
    {
      // let's build a where clause that captures all entries
      // to modified
      w.clear();
      w.addIntCol(RA_CAT_REF, catId);
      w.addIntCol(RA_PAIR_REF, pairId);
      w.addIntCol(RA_ROUND, ">=", firstRoundToModify);
      if (grpNum > 0)
      {
        w.addIntCol(RA_GRP_NUM, grpNum);   // a dedicated group number (1, 2, 3...)
      } else {
        w.addIntCol(RA_GRP_NUM, "<", 0);   // a functional number (iteration, quarter finals, ...)
      }
      DbTab::CachingRowIterator it = tab->getRowsByWhereClause(w);
      while (!(it.isEnd()))
      {
        TabRow r = *it;

        vector<tuple <string, int>> colDelta = {
          {RA_MATCHES_WON, get<0>(matchDelta)},
          {RA_MATCHES_LOST, get<1>(matchDelta)},
          {RA_MATCHES_DRAW, get<2>(matchDelta)},
          {RA_GAMES_WON, get<0>(gamesDelta)},
          {RA_GAMES_LOST, get<1>(gamesDelta)},
          {RA_POINTS_WON, get<0>(pointsDelta)},
          {RA_POINTS_LOST, get<1>(pointsDelta)},
        };

        for (const tuple<string, int>& cd : colDelta)
        {
          int dbErr;
          int oldVal = r.getInt(get<0>(cd));
          r.update(get<0>(cd), oldVal + get<1>(cd), &dbErr);
          if (dbErr != SQLITE_DONE) return false;
        }

        ++it;
      }

      return true;
    };
    //------------------------- end of helper func -------------------

    // lock the database before writing
    DbLockHolder lh{db, DatabaseAccessRoles::MainThread};

    // start a new transaction to make sure that
    // the database remains consistent in case something goes wrong
    bool isDbErr;
    auto tg = db->acquireTransactionGuard(false, &isDbErr);
    if (isDbErr) return DATABASE_ERROR;

    // modify the ranking entries
    bool isOkay = doMod(pp1Id, deltaMatches_P1, deltaGames_P1, deltaPoints_P1);
    if (!isOkay)
    {
      return DATABASE_ERROR; // triggers implicit rollback through tg's dtor
    }
    isOkay = doMod(pp2Id, deltaMatches_P2, deltaGames_P2, deltaPoints_P2);
    if (!isOkay)
    {
      return DATABASE_ERROR;  // triggers implicit rollback through tg's dtor
    }

    // now we have to re-sort the entries, round by round
    // UNLESS the caller decided to skip the sorting.
    //
    // skipping the sorting (and thus assigning ranks) is only
    // usefull in bracket matches where ranks are not derived
    // from points but from bracket logic
    if (!skipSorting)
    {
      auto specializedCat = cat.convertToSpecializedObject();
      auto lessThanFunc = specializedCat->getLessThanFunction();
      int round = firstRoundToModify;
      while (true)
      {
        w.clear();
        w.addIntCol(RA_CAT_REF, catId);
        w.addIntCol(RA_ROUND, round);
        w.addIntCol(RA_GRP_NUM, grpNum);

        // get the ranking entries
        RankingEntryList rankList = getObjectsByWhereClause<RankingEntry>(w);
        if (rankList.empty()) break;   // no more rounds to modify

        // call the standard sorting algorithm
        std::sort(rankList.begin(), rankList.end(), lessThanFunc);

        // write the sort results back to the database
        int rank = 1;
        for (RankingEntry re : rankList)
        {
          re.row.update(RA_RANK, rank);
          ++rank;
        }

        ++round;
      }
    }

    // Done. Finish the transaction
    isOkay = tg ? tg->commit() : true;
    return isOkay ? OK : DATABASE_ERROR;
  }

  string RankingMngr::getSyncString(vector<int> rows)
  {
    vector<string> cols = {"id", RA_ROUND, RA_PAIR_REF, RA_CAT_REF, RA_GRP_NUM, RA_GAMES_WON, RA_GAMES_LOST,
                          RA_MATCHES_WON, RA_MATCHES_LOST, RA_MATCHES_DRAW, RA_POINTS_WON, RA_POINTS_LOST, RA_RANK};

    return db->getSyncStringForTable(TAB_RANKING, cols, rows);
  }

//----------------------------------------------------------------------------

  RankingEntryListList RankingMngr::sortRankingEntriesForLastRound(const Category& cat, ERR* err) const
  {
    // determine the round we should create the entries for
    CatRoundStatus crs = cat.getRoundStatus();
    int lastRound = crs.getFinishedRoundsCount();
    if (lastRound < 1)
    {
      if (err != nullptr) *err = ROUND_NOT_FINISHED;
      return RankingEntryListList();
    }

    // make sure we have (unsorted) ranking entries
    WhereClause wc;
    wc.addIntCol(RA_CAT_REF, cat.getId());
    wc.addIntCol(RA_ROUND, lastRound);
    RankingEntryList rel = getObjectsByWhereClause<RankingEntry>(wc);
    if (rel.empty())
    {
      if (err != nullptr) *err = MISSING_RANKING_ENTRIES;
      return RankingEntryListList();
    }

    // derive a list of all match groups we need to
    // sort
    //
    // Note: the list of applicable match groups cannot
    // be derived from tnmt->getMatchMngr()->getMatchGroupsForCat(cat, lastRound)
    // because this doesn't fetch group numbers of those
    // round-robin groups that haven't played in this round.
    // This effect occurs if we have different group sizes in the category
    // (i.e., Group 1 has already finished, Group 2 still has to play one round)
    QList<int> applicableMatchGroupNumbers;
    for (RankingEntry re : rel)
    {
      int grpNum = re.getGroupNumber();
      if (!(applicableMatchGroupNumbers.contains(grpNum)))
      {
        applicableMatchGroupNumbers.append(grpNum);
      }
    }

    // get the category-specific comparison function
    auto specializedCat = cat.convertToSpecializedObject();
    auto lessThanFunc = specializedCat->getLessThanFunction();

    // prepare the result object
    RankingEntryListList result;

    // lock the database before writing
    DbLockHolder lh{db, DatabaseAccessRoles::MainThread};

    // apply separate sorting for every match group.
    //
    // In non-round-robin matches, this does no harm because
    // there is only one (artificial) match group in those cases
    for (int grpNum : applicableMatchGroupNumbers)
    {
      WhereClause wcWithGroupNum = wc;
      wcWithGroupNum.addIntCol(RA_GRP_NUM, grpNum);

      // get the ranking entries
      RankingEntryList rankList = getObjectsByWhereClause<RankingEntry>(wcWithGroupNum);

      // call the standard sorting algorithm
      std::sort(rankList.begin(), rankList.end(), lessThanFunc);

      // write the sort results back to the database
      int rank = 1;
      for (RankingEntry re : rankList)
      {
        re.row.update(RA_RANK, rank);
        ++rank;
      }

      // add the sorted group list to the result
      result.push_back(rankList);
    }

    if (err != nullptr) *err = OK;
    return result;
  }

//----------------------------------------------------------------------------

  ERR RankingMngr::forceRank(const RankingEntry& re, int rank) const
  {
    if (rank < 1) return INVALID_RANK;

    // lock the database before writing
    DbLockHolder lh{db, DatabaseAccessRoles::MainThread};

    re.row.update(RA_RANK, rank);

    return OK;
  }

  //----------------------------------------------------------------------------

  ERR RankingMngr::clearRank(const RankingEntry& re) const
  {
    DbLockHolder lh{db, DatabaseAccessRoles::MainThread};
    re.row.updateToNull(RA_RANK);

    return OK;
  }

//----------------------------------------------------------------------------

  void RankingMngr::fillRankGaps(const Category& cat, int round, int maxRank)
  {
    int catId = cat.getId();

    // a little helper function for creating a dummy ranking entry
    auto insertRankingEntry = [&](int g, int r) {
      ColumnValueClause cvc;
      cvc.addIntCol(RA_MATCHES_WON, -1);
      cvc.addIntCol(RA_MATCHES_DRAW, -1);
      cvc.addIntCol(RA_MATCHES_LOST, -1);
      cvc.addIntCol(RA_GAMES_WON, -1);
      cvc.addIntCol(RA_GAMES_LOST, -1);
      cvc.addIntCol(RA_POINTS_WON, -1);
      cvc.addIntCol(RA_POINTS_LOST, -1);
      cvc.addNullCol(RA_PAIR_REF);
      cvc.addIntCol(RA_ROUND, round);
      cvc.addIntCol(RA_CAT_REF, catId);
      cvc.addIntCol(RA_GRP_NUM, g);
      cvc.addIntCol(RA_RANK, r);
      tab->insertRow(cvc);
    };

    RankingEntryListList rll = getSortedRanking(cat, round);
    if (rll.empty()) return;

    // lock the database before writing
    DbLockHolder lh{db, DatabaseAccessRoles::MainThread};

    // for each match group, go through the
    // list of ranking entries and fill gaps
    // or append entries up to maxRank
    for (RankingEntryList rl : rll)
    {
      int lastSeenRank = 0;
      int grpNum = rl.at(0).getGroupNumber();

      for (RankingEntry re : rl)
      {
        int curRank = re.getRank();
        if (curRank == RankingEntry::NO_RANK_ASSIGNED) continue;

        // fill gaps, e.g., insert a dummy rank 4 and 5 between existing,
        // "real" entries 3 and 6
        while (curRank > (lastSeenRank+1))
        {
          ++lastSeenRank;
          insertRankingEntry(grpNum, lastSeenRank);
        }
        lastSeenRank = curRank;
      }

      // append entries up to maxRank
      while (lastSeenRank < maxRank)
      {
        ++lastSeenRank;
        insertRankingEntry(grpNum, lastSeenRank);
      }
    }
  }

//----------------------------------------------------------------------------

  unique_ptr<RankingEntry> RankingMngr::getRankingEntry(const Category& cat, int round, int grpNum, int rank) const
  {
    WhereClause wc;
    wc.addIntCol(RA_CAT_REF, cat.getId());
    wc.addIntCol(RA_ROUND, round);
    wc.addIntCol(RA_GRP_NUM, grpNum);
    wc.addIntCol(RA_RANK, rank);

    return getSingleObjectByWhereClause<RankingEntry>(wc);
  }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


}
