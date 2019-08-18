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

#include "Player.h"
#include "Team.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "TournamentErrorCodes.h"
#include "PlayerMngr.h"
#include "CatMngr.h"
#include "TeamMngr.h"
#include "MatchMngr.h"
#include "CourtMngr.h"

namespace QTournament
{

  Player::Player(const TournamentDB& _db, int rowId)
  :TournamentDatabaseObject(_db, TAB_PLAYER, rowId)
  {
  }

//----------------------------------------------------------------------------

  Player::Player(const TournamentDB& _db, const SqliteOverlay::TabRow& _row)
  :TournamentDatabaseObject(_db, _row)
  {
  }

//----------------------------------------------------------------------------

  QString Player::getDisplayName(int maxLen) const
  {
    QString first = QString::fromUtf8(row[PL_FNAME].data());
    QString last = QString::fromUtf8(row[PL_LNAME].data());
    
    QString fullName = last + ", " + first;
    
    if (maxLen < 1)   // no length restriction
    {
      return fullName;
    }
    
    if (maxLen < 6)  // invalid length restriction
    {
      throw std::invalid_argument("Can't generate display name of less than six characters");
    }
    
    if (fullName.length() <= maxLen)   // sufficient length restriction
    {
      return fullName;
    }
    
    // okay, we have to shorten the name somehow. Preferably, we keep as much
    // of the last name as possible and shorten the first name
    while (fullName.length() > maxLen)
    {
      if (first.length() > 1)
      {
        first = first.left(first.length() - 1);
      } else {
        last = last.left(last.length() - 1);
      }
      
      fullName = last + ", " + first;
    }
    
    return fullName;
    
  }

//----------------------------------------------------------------------------

  QString Player::getDisplayName_FirstNameFirst() const
  {
    QString first = QString::fromUtf8(row[PL_FNAME].data());
    QString last = QString::fromUtf8(row[PL_LNAME].data());

    return first + " " + last;
  }

//----------------------------------------------------------------------------

  ERR Player::rename(const QString& newFirst, const QString& newLast)
  {
    PlayerMngr pm{db};
    return pm.renamePlayer(*this, newFirst, newLast);
  }

//----------------------------------------------------------------------------

  QString Player::getFirstName() const
  {
    return QString::fromUtf8(row[PL_FNAME].data());
  }

//----------------------------------------------------------------------------

  QString Player::getLastName() const
  {
    return QString::fromUtf8(row[PL_LNAME].data());
  }

//----------------------------------------------------------------------------

  SEX Player::getSex() const
  {
    int sexInt = row.getInt(PL_SEX);
    return static_cast<SEX>(sexInt);
  }

//----------------------------------------------------------------------------

  Team Player::getTeam() const
  {
    auto teamRef = row.getInt2(PL_TEAM_REF);
    
    // if we don't use teams, throw an exception
    if (!teamRef)
    {
      throw std::runtime_error("Query for team of a player occurred; however, we're not using teams in this tournament!");
    }
    
    TeamMngr tm{db};
    return tm.getTeamById(*teamRef);
  }

//----------------------------------------------------------------------------

  std::vector<Category> Player::getAssignedCategories() const
  {
    SqliteOverlay::WhereClause wc;
    wc.addCol(P2C_PLAYER_REF, getId());

    CategoryList result;
    CatMngr cmngr{db};
    for (SqliteOverlay::TabRowIterator it{db, TAB_P2C, wc}; it.hasData(); ++it)
    {
      int catId = it->getInt(P2C_CAT_REF);
      result.push_back(cmngr.getCategoryById(catId));
    }

    return result;
  }

//----------------------------------------------------------------------------

  int Player::getRefereeCount() const
  {
    return row.getInt(PL_REFEREE_COUNT);
  }

  //----------------------------------------------------------------------------

  std::optional<Court> Player::getRefereeCourt() const
  {
    if (getState() != STAT_PL_REFEREE) return {};

    // find the court on which the player is umpire
    SqliteOverlay::DbTab matchTab{db, TAB_MATCH, false};
    SqliteOverlay::WhereClause wc;
    wc.addCol(GENERIC_STATE_FIELD_NAME, static_cast<int>(STAT_MA_RUNNING));
    wc.addCol(MA_REFEREE_REF, getId());

    // do we have a matching match entry?
    auto r = matchTab.getSingleRowByWhereClause2(wc);
    if (!r) return {};

    MatchMngr mm{db};
    auto ma = mm.getMatch(r->id());
    if (!ma) return {};  // shouldn't happen

    return ma->getCourt();
  }

  //----------------------------------------------------------------------------

  std::optional<Court> Player::getMatchCourt() const
  {
    if (getState() != STAT_PL_PLAYING) return {};

    // find the court on which the player is playing
    //
    // for this, we simply search all courts because this is
    // easier that searching in all player pairs the player
    // could be part of
    CourtMngr cm{db};
    for (const Court& co : cm.getAllCourts())
    {
      if (co.getState() != STAT_CO_BUSY) continue;
      auto ma = co.getMatch();
      if (!ma) continue;

      for (const Player& pl : ma->determineActualPlayers())
      {
        if (pl == *this) return co;
      }
    }

    return {};
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

}
