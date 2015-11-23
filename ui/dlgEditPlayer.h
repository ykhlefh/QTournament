/* 
 * File:   dlgEditPlayer.h
 * Author: volker
 *
 * Created on March 18, 2014, 8:00 PM
 */

#ifndef _DLGEDITPLAYER_H
#define	_DLGEDITPLAYER_H

#include "ui_dlgEditPlayer.h"

#include "Player.h"
#include "ExternalPlayerDB.h"

using namespace QTournament;

class DlgEditPlayer : public QDialog
{
  Q_OBJECT
public:
  DlgEditPlayer (QWidget *parent, Player* _selectedPlayer = nullptr);
  DlgEditPlayer (QWidget *parent, SEX _sexPreset, const Category& _catPreset);
  DlgEditPlayer (QWidget *parent, const ExternalPlayerDatabaseEntry& nameAndSexPreset, int _presetCatId=-1);
  virtual ~DlgEditPlayer ();
  QString getFirstName();
  QString getLastName();
  bool hasNameChange();
  SEX getSex();
  Team getTeam();
  QHash<Category, bool> getCategoryCheckState();
  
private:
  Ui::dlgEditPlayer ui;
  Player* selectedPlayer;
  void initFromPlayerData();
  void initTeamList();
  bool _hasNameChange;
  void updateCatList(QHash<Category, CAT_ADD_STATE> catStatus, int preselectCatId = -1);

  SEX sexPreset;
  int presetCatId = -1;

public slots:
  virtual void done (int result);
  
private slots:
  void onSexSelectionChanged(int preselectCatId = -1);
};

#endif	/* _DLGEDITPLAYER_H */
