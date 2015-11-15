/* 
 * File:   CatTabWidget.h
 * Author: volker
 *
 * Created on March 24, 2014, 7:13 PM
 */

#ifndef _CATTABWIDGET_H
#define	_CATTABWIDGET_H

#include <QMenu>
#include <QAction>

#include "ui_CatTabWidget.h"

class CatTabWidget : public QDialog
{
  Q_OBJECT
public:
  CatTabWidget();
  virtual ~CatTabWidget();
private:
  Ui::CatTabWidget ui;
  void updateControls();
  void updatePairs();
  int unpairedPlayerId1;
  int unpairedPlayerId2;

  unique_ptr<QMenu> lwUnpairedContextMenu;
  QAction* actRemovePlayer;
  QAction* actAddPlayer;
  QAction* actRegister;
  QAction* actUnregister;

  void initContextMenu();
  upPlayer lwUnpaired_getSelectedPlayer() const;

public slots:
  void onCatModelChanged();
  void onCatSelectionChanged(const QItemSelection &, const QItemSelection &);
  void onCbDrawChanged(bool newState);
  void onDrawScoreChanged(int newVal);
  void onWinScoreChanged(int newVal);
  void onUnpairedPlayersSelectionChanged();
  void onBtnPairClicked();
  void onPairedPlayersSelectionChanged();
  void onBtnSplitClicked();
  void onMatchTypeButtonClicked(int btn);
  void onSexClicked(int btn);
  void onDontCareClicked();
  void onBtnAddCatClicked();
  void onBtnRunCatClicked();
  void onMatchSystemChanged(int newId);
  void onGroupConfigChanged(const KO_Config& newCfg);
  void onPlayerAddedToCategory(const Player& p, const Category& c);
  void onPlayerRemovedFromCategory(const Player& p, const Category& c);
  void onPlayerRenamed(const Player& p);
  void onCatStateChanged(const Category& c, const OBJ_STATE fromState, const OBJ_STATE toState);
  void onPlayerStateChanged(int playerId, int seqNum, const OBJ_STATE fromState, const OBJ_STATE toState);
  void onRemovePlayerFromCat();
  void onAddPlayerToCat();
  void onUnpairedContextMenuRequested(const QPoint& pos);
  void onRegisterPlayer();
  void onUnregisterPlayer();
} ;

#endif	/* _CATTABWIDGET_H */
