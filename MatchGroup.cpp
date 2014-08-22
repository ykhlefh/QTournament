/* 
 * File:   MatchGroup.cpp
 * Author: volker
 * 
 * Created on August 22, 2014, 7:58 PM
 */

#include "MatchGroup.h"
#include "Tournament.h"

namespace QTournament
{

  MatchGroup::MatchGroup(TournamentDB* db, int rowId)
  :GenericDatabaseObject(db, TAB_MATCH_GROUP, rowId)
  {
  }

//----------------------------------------------------------------------------

  MatchGroup::MatchGroup(TournamentDB* db, dbOverlay::TabRow row)
  :GenericDatabaseObject(db, row)
  {
  }

//----------------------------------------------------------------------------

  Category MatchGroup::getCategory()
  {
    int catId = row[MG_CAT_REF].toInt();
    return Tournament::getCatMngr()->getCategoryById(catId);
  }

//----------------------------------------------------------------------------

  int MatchGroup::getGroupNumber()
  {
    return row[MG_GRP_NUM].toInt();
  }

//----------------------------------------------------------------------------

  int MatchGroup::getRound()
  {
    return row[MG_ROUND].toInt();
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


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

}