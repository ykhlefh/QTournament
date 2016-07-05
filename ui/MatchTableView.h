/*
 *    This is QTournament, a badminton tournament management program.
 *    Copyright (C) 2014 - 2016  Volker Knollmann
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

#ifndef MATCHTABLEVIEW_H
#define	MATCHTABLEVIEW_H

#include <memory>

#include <QTableView>
#include <QSortFilterProxyModel>
#include <QListWidget>
#include <QMenu>
#include <QAction>
#include <QStringListModel>
#include <QTimer>

#include "TournamentDB.h"
#include "delegates/MatchItemDelegate.h"
#include "models/MatchTabModel.h"
#include "Match.h"

using namespace QTournament;

class MatchTableView : public QTableView
{
  Q_OBJECT
  
public:
  //enum class FilterType : std::int8_t { IDLE = 1, STAGED = 2, NONE = 0 };

  MatchTableView (QWidget* parent);
  virtual ~MatchTableView ();
  unique_ptr<Match> getSelectedMatch() const;
  void updateSelectionAfterDataChange();
  void setDatabase(TournamentDB* _db);
  void updateRefereeColumn();
  
protected:
  static constexpr int MAX_NUMERIC_COL_WIDTH = 90;
  static constexpr int REL_NUMERIC_COL_WIDTH = 8;
  static constexpr int REL_CAT_COL_WIDTH = 9;
  static constexpr int REL_MATCH_COL_WIDTH = 63;
  static constexpr int REL_REFEREE_COL_WIDTH = 20;
  virtual void resizeEvent(QResizeEvent *event) override;
  void autosizeColumns();

private slots:
  void onSelectionChanged(const QItemSelection&selectedItem, const QItemSelection&deselectedItem);
  void onContextMenuRequested(const QPoint& pos);
  void onWalkoverP1Triggered();
  void onWalkoverP2Triggered();
  void onMatchDoubleClicked(const QModelIndex& index);
  void onAssignRefereeTriggered();
  void onRemoveRefereeTriggered();
  void onSectionHeaderDoubleClicked();
  void onMatchTimePredictionUpdate();

signals:
  void matchSelectionChanged(int newlySelectedMatchId);

private:
  static constexpr int PREDICTION_UPDATE_INTERVAL__MS = 10 * 1000; // update every 10 secs
  TournamentDB* db;
  QStringListModel* emptyModel;
  MatchTableModel* curDataModel;
  QSortFilterProxyModel* sortedModel;
  unique_ptr<MatchItemDelegate> matchItemDelegate;
  QAbstractItemDelegate* defaultDelegate;

  unique_ptr<QMenu> contextMenu;
  QAction* actPostponeMatch;
  QMenu* walkoverSelectionMenu;
  QMenu* courtSelectionMenu;
  QAction* actWalkoverP1;
  QAction* actWalkoverP2;

  QMenu* refereeMode_submenu;
  QAction* actAssignReferee;
  QAction* actRemoveReferee;

  unique_ptr<QTimer> predictionUpdateTimer;

  void initContextMenu();
  void updateContextMenu();
  void execWalkover(int playerNum);
  void showMatchBusyReason(const Match& ma);
};

#endif	/* MATCHTABLEVIEW_H */

