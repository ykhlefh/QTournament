/* 
 * File:   MatchGroup.cpp
 * Author: volker
 * 
 * Created on August 22, 2014, 7:58 PM
 */

#include "MatchGroup.h"
#include "Tournament.h"
#include "DbTab.h"

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

  Category MatchGroup::getCategory() const
  {
    int catId = row[MG_CAT_REF].toInt();
    return Tournament::getCatMngr()->getCategoryById(catId);
  }

//----------------------------------------------------------------------------

  int MatchGroup::getGroupNumber() const
  {
    return row[MG_GRP_NUM].toInt();
  }

//----------------------------------------------------------------------------

  int MatchGroup::getRound() const
  {
    return row[MG_ROUND].toInt();
  }  

//----------------------------------------------------------------------------

  MatchList MatchGroup::getMatches() const
  {
    DbTab matchTab = db->getTab(TAB_MATCH);
    DbTab::CachingRowIterator it = matchTab.getRowsByColumnValue(MA_GRP_REF, getId());

    MatchList result;
    while (!(it.isEnd()))
    {
      result << Match(db, *it);
      ++it;
    }

    return result;
  }

//----------------------------------------------------------------------------

  int MatchGroup::getMatchCount() const
  {
    DbTab matchTab = db->getTab(TAB_MATCH);
    return matchTab.getMatchCountForColumnValue(MA_GRP_REF, getId());
  }

//----------------------------------------------------------------------------

  int MatchGroup::getStageSequenceNumber() const
  {
    QVariant result = row[MG_STAGE_SEQ_NUM];
    if (result.isNull())
    {
      return -1;  // group not staged
    }

    return result.toInt();
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