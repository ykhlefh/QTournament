#include <QList>

#include "MatchResultList_byGroup.h"
#include "SimpleReportGenerator.h"
#include "TableWriter.h"

#include "PlayerMngr.h"
#include "TeamMngr.h"
#include "Team.h"
#include "CatRoundStatus.h"
#include "ui/GuiHelpers.h"
#include "MatchGroup.h"

namespace QTournament
{


MatchResultList_ByGroup::MatchResultList_ByGroup(TournamentDB* _db, const QString& _name, const Category& _cat, int _grpNum)
  :AbstractReport(_db, _name), cat(_cat), grpNum(_grpNum)
{
  // make sure that the requested group is a round-robin group
  // and the group is actually existing
  if (grpNum < 1)
  {
    throw std::runtime_error("Requested match results report for invalid group.");
  }
  for (MatchGroup mg : Tournament::getMatchMngr()->getMatchGroupsForCat(cat, 1))
  {
    if (mg.getGroupNumber() == grpNum) return;  // okay, the round-robin group number exists at least in the first round
  }

  throw std::runtime_error("Requested match results report for invalid group.");
}

//----------------------------------------------------------------------------

upSimpleReport MatchResultList_ByGroup::regenerateReport() const
{
  // collect the match groups with the requested match group number and
  // search in all rounds
  MatchMngr* mm = Tournament::getMatchMngr();
  MatchGroupList mgl = mm->getMatchGroupsForCat(cat);
  MatchGroupList filteredList;
  for (MatchGroup mg: mgl)
  {
    if (mg.getGroupNumber() == grpNum) filteredList.append(mg);
  }

  // sort match groups by round number
  if (filteredList.size() > 1)
  {
    std::sort(filteredList.begin(), filteredList.end(), [](MatchGroup& mg1, MatchGroup& mg2){
      if (mg1.getRound() < mg2.getRound()) return true;
      return false;
    });
  }

  upSimpleReport result = createEmptyReport_Portrait();
  QString repName = cat.getName() + tr(" -- Results of Group ") + QString::number(grpNum);
  setHeaderAndHeadline(result.get(), repName);
  prepTabsForMatchResults(result);

  for (MatchGroup mg : filteredList)
  {
    int round = mg.getRound();
    printIntermediateHeader(result, tr("Round ") + QString::number(round));

    for (Match ma : mg.getMatches())
    {
      if (ma.getState() != STAT_MA_FINISHED) continue;
      printMatchResult(result, ma, tr("Round ") + QString::number(round) + tr(" (cont.)"));
    }

    if (mg.getState() != STAT_MG_FINISHED)
    {
      result->skip(1.0);
      result->writeLine(tr("Note: this round is not finished yet; results for this group can be incomplete."));
    }
  }

  // set header and footer
  setHeaderAndFooter(result, repName);

  return result;
}

//----------------------------------------------------------------------------

QStringList MatchResultList_ByGroup::getReportLocators() const
{
  QStringList result;

  QString loc = tr("Results::");
  loc += cat.getName() + "::by group::";
  loc += tr("Group ") + QString::number(grpNum);

  result.append(loc);

  return result;
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


}