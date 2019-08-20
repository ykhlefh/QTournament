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

#include <QMessageBox>
#include <QResizeEvent>
#include <QHeaderView>
#include <QGuiApplication>
#include <QScrollBar>

#include "DlgSelectReferee.h"
#include "ui_DlgSelectReferee.h"

#include <SqliteOverlay/KeyValueTab.h>

#include "PlayerPair.h"
#include "Team.h"
#include "TeamMngr.h"
#include "PlayerMngr.h"
#include "HelperFunc.h"
#include "delegates/DelegateItemLED.h"
#include "DlgPlayerProfile.h"

DlgSelectReferee::DlgSelectReferee(TournamentDB* _db, const Match& _ma, REFEREE_ACTION _refAction, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgSelectReferee), db(_db), ma(_ma), refAction(_refAction)
{
  ui->setupUi(this);

  // initialize the match information section
  PlayerPair pp1 = ma.getPlayerPair1();
  PlayerPair pp2 = ma.getPlayerPair2();
  ui->laPair1Player1Name->setText(pp1.getPlayer1().getDisplayName_FirstNameFirst());
  ui->laPair1Player1Team->setText(pp1.getPlayer1().getTeam().getName());
  ui->laPair2Player1Name->setText(pp2.getPlayer1().getDisplayName_FirstNameFirst());
  ui->laPair2Player1Team->setText(pp2.getPlayer1().getTeam().getName());
  if (pp1.hasPlayer2())
  {
    ui->laPair1Player2Name->setText(pp1.getPlayer2().getDisplayName_FirstNameFirst());
    ui->laPair1Player2Team->setText(pp1.getPlayer2().getTeam().getName());
    ui->laPair2Player2Name->setText(pp2.getPlayer2().getDisplayName_FirstNameFirst());
    ui->laPair2Player2Team->setText(pp2.getPlayer2().getTeam().getName());
  } else {
    ui->laPair1Player2Name->setHidden(true);
    ui->laPair1Player2Team->setHidden(true);
    ui->laPair2Player2Name->setHidden(true);
    ui->laPair2Player2Team->setHidden(true);
  }

  // initialize the drop box for the selection mode
  ui->cbFilterMode->addItem(tr("All players"), static_cast<int>(RefereeMode::RefereeMode::AllPlayers));
  ui->cbFilterMode->addItem(tr("Recent finishers"), static_cast<int>(RefereeMode::RefereeMode::RecentFinishers));
  ui->cbFilterMode->addItem(tr("Special team member"), static_cast<int>(RefereeMode::RefereeMode::SpecialTeam));
  RefereeMode curRefMode = ma.get_EFFECTIVE_RefereeMode();
  switch (curRefMode)
  {
  case RefereeMode::RefereeMode::AllPlayers:
    ui->cbFilterMode->setCurrentIndex(0);
    break;

  case RefereeMode::RefereeMode::RecentFinishers:
    ui->cbFilterMode->setCurrentIndex(1);
    break;

  case RefereeMode::RefereeMode::SpecialTeam:
    ui->cbFilterMode->setCurrentIndex(2);
    break;

  case RefereeMode::RefereeMode::HandWritten:
    reject();  // nothing to do for us
    break;

  case RefereeMode::RefereeMode::None:
    reject();  // nothing to do for us
    break;
  }

  // get the tournament-wide default value for the referee-team, if any
  auto cfg = SqliteOverlay::KeyValueTab::getTab(db, TabCfg);
  int defaultRefereeTeamId = cfg.getInt(CfgKey_RefereeTeamId);
  initTeamList(defaultRefereeTeamId);


  // if the dialog is executed when the match is NOT BEING CALLED (--> we have a "pre-assignment)
  // then continuing without an umpire is no option
  ui->btnNone->setHidden(refAction != REFEREE_ACTION::MATCH_CALL);

  // fill the player table
  rebuildPlayerList();

  // set the headline
  QString hdr;
  QString style = "QLabel { color : %1; };";
  switch (refAction)
  {
  case REFEREE_ACTION::PRE_ASSIGN:
    hdr = tr("Pre-assignment");
    style = style.arg("green");
    break;

  case REFEREE_ACTION::MATCH_CALL:
    hdr = tr("!! Match call !!");
    style = style.arg("red");
    break;

  case REFEREE_ACTION::SWAP:
    hdr = tr("Umpire swap");
    style = style.arg("green");
    break;
  }
  ui->laHeadline->setText(hdr);
  ui->laHeadline->setStyleSheet(style);

  updateControls();
}

//----------------------------------------------------------------------------

DlgSelectReferee::~DlgSelectReferee()
{
  delete ui;
}

//----------------------------------------------------------------------------

std::optional<Player> DlgSelectReferee::getFinalPlayerSelection()
{
  return std::move(finalPlayerSelection);
}

//----------------------------------------------------------------------------

void DlgSelectReferee::onFilterModeChanged()
{
  rebuildPlayerList();
  updateControls();
}

//----------------------------------------------------------------------------

void DlgSelectReferee::onTeamSelectionChanged()
{
  rebuildPlayerList();
  updateControls();
}

//----------------------------------------------------------------------------

void DlgSelectReferee::onPlayerSelectionChanged()
{
  updateControls();
}

//----------------------------------------------------------------------------

void DlgSelectReferee::onBtnSelectClicked()
{
  // the following call must yield a valid player (not nullptr)
  // because otherwise we wouldn't enter this slot because
  // the select button is only available if a player is selected.
  finalPlayerSelection = ui->tabPlayers->getSelectedPlayer();

  // if we were in team selection mode, store the selected team as
  // the new default team
  int curFilterModeId = ui->cbFilterMode->currentData().toInt();
  RefereeMode curFilterMode = static_cast<RefereeMode>(curFilterModeId);
  if (curFilterMode == RefereeMode::RefereeMode::SpecialTeam)
  {
    int curTeamId = ui->cbTeamSelection->currentData().toInt();
    auto cfg = SqliteOverlay::KeyValueTab::getTab(db, TabCfg);
    cfg.set(CfgKey_RefereeTeamId, curTeamId);
  }

  accept();
}

//----------------------------------------------------------------------------

void DlgSelectReferee::onBtnNoneClicked()
{
  finalPlayerSelection = nullptr;
  accept();
}

//----------------------------------------------------------------------------

void DlgSelectReferee::onPlayerDoubleClicked()
{
  if (!(ui->tabPlayers->hasPlayerSelected())) return;

  // if the user pressed shift or ctrl while double clicking,
  // we show the extended player profile for the selected user
  if (QGuiApplication::keyboardModifiers() != Qt::NoModifier)
  {
    auto selPlayer = ui->tabPlayers->getSelectedPlayer();
    if (selPlayer == nullptr) return;
    DlgPlayerProfile dlg{*selPlayer};
    dlg.exec();
    return;
  }

  // double click without modifier means: select the player and exit
  onBtnSelectClicked();
}

//----------------------------------------------------------------------------

void DlgSelectReferee::updateControls()
{
  // disable the team selection drop box if the filter
  // type is not "special team"
  int curFilterModeId = ui->cbFilterMode->currentData().toInt();
  RefereeMode curFilterMode = static_cast<RefereeMode>(curFilterModeId);
  ui->cbTeamSelection->setHidden(curFilterMode != RefereeMode::RefereeMode::SpecialTeam);
  ui->laTeamSelection->setHidden(curFilterMode != RefereeMode::RefereeMode::SpecialTeam);

  // only enable the "select" button if a player is selected
  ui->btnSelect->setEnabled(ui->tabPlayers->hasPlayerSelected());
}

//----------------------------------------------------------------------------

void DlgSelectReferee::initTeamList(int defaultTeamId)
{
  TeamMngr tm{db};
  std::vector<Team> allTeams = tm.getAllTeams();

  // Sort the list aphabetically
  std::sort(allTeams.begin(), allTeams.end(), [](Team& t1, Team& t2) {
    return (t1.getName() < t2.getName());
  });

  ui->cbTeamSelection->clear();

  // add a "please select"-entry if we have no
  // pre-selected player. Read: if we have a new player
  if (defaultTeamId < 1)
  {
    ui->cbTeamSelection->addItem(tr("<Please select>"), -1);
  }

  for (int i=0; i < allTeams.size(); i++)
  {
    ui->cbTeamSelection->addItem(allTeams.at(i).getName(), allTeams.at(i).getId());
    if (allTeams.at(i).getId() == defaultTeamId) ui->cbTeamSelection->setCurrentIndex(i);
  }
}

//----------------------------------------------------------------------------

void DlgSelectReferee::rebuildPlayerList()
{
  // determine the current filter and team selection
  int curFilterModeId = ui->cbFilterMode->currentData().toInt();
  RefereeMode curFilterMode = static_cast<RefereeMode>(curFilterModeId);
  int curTeamId = ui->cbTeamSelection->currentData().toInt();

  // if the current filter is "team" but there is no team selected,
  // stop here
  if ((curFilterMode == RefereeMode::RefereeMode::SpecialTeam) && (curTeamId < 1))
  {
    ui->tabPlayers->rebuildPlayerList(TaggedPlayerList(), ma.getMatchNumber(), curFilterMode);
    return;
  }

  // determine the list of players for display
  PlayerMngr pm{db};
  TeamMngr tm{db};
  TaggedPlayerList pList;
  if (curFilterMode == RefereeMode::RefereeMode::AllPlayers)
  {
    PlayerList purePlayerList = pm.getAllPlayers();

    // sort players alphabetically
    std::sort(purePlayerList.begin(), purePlayerList.end(), [](const Player& p1, const Player& p2) {
      return p1.getDisplayName() < p2.getDisplayName();
    });

    // convert to a tagged player list with all tags set to NEUTRAL
    for (const Player& p : purePlayerList)
    {
      pList.push_back(make_pair(p, RefereeSelectionDelegate::NeutralTag));
    }
  }
  if (curFilterMode == RefereeMode::RefereeMode::SpecialTeam)
  {
    Team selTeam = tm.getTeamById(curTeamId);
    PlayerList purePlayerList = tm.getPlayersForTeam(selTeam);

    std::sort(purePlayerList.begin(), purePlayerList.end(), [](const Player& p1, const Player& p2) {
      return p1.getDisplayName() < p2.getDisplayName();
    });

    // convert to a tagged player list with all tags set to NEUTRAL
    for (const Player& p : purePlayerList)
    {
      pList.push_back(make_pair(p, RefereeSelectionDelegate::NeutralTag));
    }
  }
  if (curFilterMode == RefereeMode::RefereeMode::RecentFinishers)
  {
    pList = getPlayerList_recentFinishers();
  }

  // if we currently calling the match or swapping the umpire, only players in state IDLE
  // may be selected
  if (refAction != REFEREE_ACTION::PRE_ASSIGN)
  {
    auto it = pList.begin();
    while (it != pList.end())
    {
      const TaggedPlayer& tp = *it;
      const Player& p = tp.first;
      if (p.getState() != ObjState::PL_Idle)
      {
        it = pList.erase(it);
      } else {
        ++it;
      }
    }
  }

  // add the players to the table
  ui->tabPlayers->rebuildPlayerList(pList, ma.getMatchNumber(), curFilterMode);
}

//----------------------------------------------------------------------------

TaggedPlayerList DlgSelectReferee::getPlayerList_recentFinishers()
{
  PlayerMngr pm{db};
  PlayerPairList winners;
  PlayerPairList losers;
  PlayerPairList draws;
  pm.getRecentFinishers(MAX_NUM_LOSERS, winners, losers, draws);

  // process winners, losers and draws
  std::vector<pair<PlayerPairList&, int>> allLists = {
    {winners, RefereeSelectionDelegate::WinnerTag},
    {losers, RefereeSelectionDelegate::LoserTag},
    {draws, RefereeSelectionDelegate::NeutralTag},
  };
  TaggedPlayerList result;
  for (pair<PlayerPairList&, int> listDef : allLists)
  {
    PlayerList purePlayerList;
    for (const PlayerPair& pp : listDef.first)
    {
      Player p = pp.getPlayer1();

      // if this player is already a referee, skip this player
      if (p.getState() == ObjState::PL_Referee) continue;

      // Before we add this player to the result list,
      // make sure that the player is not already in it
      if (std::find(purePlayerList.begin(), purePlayerList.end(), p) == purePlayerList.end())
      {
        purePlayerList.push_back(p);
      }

      // if this is a doubles pair, check the second player as well
      if (pp.hasPlayer2())
      {
        p = pp.getPlayer2();
        if (p.getState() == ObjState::PL_Referee) continue;
        if (std::find(purePlayerList.begin(), purePlayerList.end(), p) == purePlayerList.end())
        {
          purePlayerList.push_back(p);
        }
      }
    }

    // convert the winner into a tagged player list
    // with the tag set to the appropriate value
    for (const Player& p : purePlayerList)
    {
      result.push_back(make_pair(p, listDef.second));
    }
  }

  return result;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

RefereeTableWidget::RefereeTableWidget(QWidget* parent)
  :GuiHelpers::AutoSizingTableWidget_WithDatabase{
     GuiHelpers::AutosizeColumnDescrList{
       {"", REL_WIDTH_STATE, -1, -1},
       {tr("Player name"), REL_WIDTH_NAME, -1, -1},
       {tr("Team"), REL_WIDTH_TEAM, -1, -1},
       {tr("Uses"), REL_WIDTH_OTHER, -1, MAX_OTHER_COL_WIDTH},
       {tr("Last match finished"), REL_WIDTH_OTHER, -1, MAX_OTHER_COL_WIDTH},
       {tr("Next match"), REL_WIDTH_OTHER, -1, MAX_OTHER_COL_WIDTH}
     }, parent}
{
  setRubberBandCol(NAME_COL_ID);
}

//----------------------------------------------------------------------------

void RefereeTableWidget::rebuildPlayerList(const TaggedPlayerList& pList, int selectedMatchNumer, RefereeMode _refMode)
{
  // store the current referee mode. We need this to properly
  // initiate the filtering column
  refMode = _refMode;

  // erase everything from the table
  clearContents();
  setRowCount(0);

  // grab the current database handle from the first entry
  // in the player list
  if (pList.empty())
  {
    setDatabase(nullptr);
    return;
  } else {
    setDatabase(pList.at(0).first.getDatabaseHandle());
  }

  // disable sorting while we're modifying the table
  setSortingEnabled(false);

  // populate the table rows
  PlayerMngr pm{db};
  setRowCount(pList.size());
  int idxRow = 0;
  for (const TaggedPlayer& tp : pList)
  {
    const Player& p = tp.first;

    // add the player's name
    QTableWidgetItem* newItem = new QTableWidgetItem(p.getDisplayName());
    newItem->setData(Qt::UserRole, p.getId());
    newItem->setData(Qt::UserRole + 1, tp.second);  // set the tag
    newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    setItem(idxRow, NAME_COL_ID, newItem);

    // add the player's team
    newItem = new QTableWidgetItem(p.getTeam().getName());
    newItem->setData(Qt::UserRole, p.getId());
    newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    setItem(idxRow, TEAM_COL_ID, newItem);

    // add the player's referee count
    int refereeCount = p.getRefereeCount();
    newItem = new QTableWidgetItem(QString::number(refereeCount));
    newItem->setData(Qt::UserRole, p.getId());
    newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    setItem(idxRow, REFEREE_COUNT_COL_ID, newItem);

    // add the time of the last finished match
    auto ma = pm.getLastFinishedMatchForPlayer(p);
    QString txt = "--";
    if (ma != nullptr)
    {
      QDateTime finishTime = ma->getFinishTime();
      txt = finishTime.toString("HH:mm");
    }
    newItem = new QTableWidgetItem(txt);
    newItem->setData(Qt::UserRole, p.getId());
    newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    setItem(idxRow, LAST_FINISH_TIME_COL_ID, newItem);

    // add the player's status as a color indication in
    // the first column
    newItem = new QTableWidgetItem("");
    newItem->setData(Qt::UserRole, p.getId());
    newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    setItem(idxRow, ObjState::COL_ID, newItem);

    // add the offset to the next match for the player
    ma = pm.getNextMatchForPlayer(p);
    txt = "--";
    if (ma != nullptr)
    {
      int matchNumOffset = ma->getMatchNumber() - selectedMatchNumer;

      if (matchNumOffset > 0) txt = "+ %1";
      if (matchNumOffset < 0)
      {
        txt = "- %1";
        matchNumOffset *= -1;   // make it positive and insert the "-" manually with a space between "-" and number
      }
      txt = txt.arg(matchNumOffset);
    }
    newItem = new QTableWidgetItem(txt);
    newItem->setData(Qt::UserRole, p.getId());
    newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    setItem(idxRow, NEXT_MATCH_DIST_COL_ID, newItem);

    idxRow++;
  }

  // set the right sorting mode
  if (refMode == RefereeMode::RefereeMode::RecentFinishers)
  {
    sortByColumn(LAST_FINISH_TIME_COL_ID, Qt::DescendingOrder);
  } else {
    // Default: sort by name
    sortByColumn(NAME_COL_ID, Qt::AscendingOrder);
  }
  setSortingEnabled(true);
}

//----------------------------------------------------------------------------

std::optional<QTournament::Player> RefereeTableWidget::getSelectedPlayer()
{
  // could we ever store a database handle? If not,
  // we can't determine the player
  if (db == nullptr) return nullptr;

  // is something selected?
  if (!(hasPlayerSelected())) return nullptr;

  // get the player item for the selected row
  QTableWidgetItem* playerItem = item(currentRow(), NAME_COL_ID);
  if (playerItem == nullptr) return nullptr;

  // the user data of the player item contains the player id
  int playerId = playerItem->data(Qt::UserRole).toInt();

  // return the associated player
  PlayerMngr pm{db};
  return pm.getPlayer2(playerId);
}

//----------------------------------------------------------------------------

bool RefereeTableWidget::hasPlayerSelected()
{
  return ((currentRow() >= 0) && (currentItem() != nullptr));
}

//----------------------------------------------------------------------------

void RefereeTableWidget::hook_onDatabaseOpened()
{
  setCustomDelegate(new RefereeSelectionDelegate(db, this));
}

//----------------------------------------------------------------------------

