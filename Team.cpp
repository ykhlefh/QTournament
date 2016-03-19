/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2015  Volker Knollmann
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

#include "Team.h"
#include "TournamentDataDefs.h"
#include "TournamentDB.h"
#include "TournamentErrorCodes.h"
#include "TeamMngr.h"
#include "Tournament.h"

namespace QTournament
{

  Team::Team(TournamentDB* db, int rowId)
  :TournamentDatabaseObject(db, TAB_TEAM, rowId)
  {
  }

//----------------------------------------------------------------------------

  Team::Team(TournamentDB* db, SqliteOverlay::TabRow row)
  :TournamentDatabaseObject(db, row)
  {
  }

//----------------------------------------------------------------------------

  QString Team::getName(int maxLen) const
  {
    QString fullName = QString::fromUtf8(row[GENERIC_NAME_FIELD_NAME].data());
    
    if (maxLen < 1)
    {
      return fullName;
    }
    
    if (maxLen == 1)
    {
      return fullName.left(1);
    }
    
    if (maxLen == 2)
    {
      return fullName.left(1) + ".";
    }
    
    if ((maxLen > 2) && (maxLen < fullName.length()))
    {
      return fullName.left(maxLen - 1) + ".";
    }
    
    return fullName;
  }

//----------------------------------------------------------------------------

  ERR Team::rename(const QString& nn)
  {
    auto tnmt = Tournament::getActiveTournament();
    return tnmt->getTeamMngr()->renameTeam(*this, nn);
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
