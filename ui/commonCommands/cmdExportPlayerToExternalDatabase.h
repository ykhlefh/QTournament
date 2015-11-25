#ifndef CMDEXPORTPLAYERTOEXTERNALDATABASE_H
#define CMDEXPORTPLAYERTOEXTERNALDATABASE_H

#include <QObject>

#include "AbstractCommand.h"
#include "Category.h"

using namespace QTournament;

class cmdExportPlayerToExternalDatabase : public QObject, AbstractCommand
{
  Q_OBJECT

public:
  cmdExportPlayerToExternalDatabase(QWidget* p, const Player& _pl);
  virtual ERR exec() override;
  virtual ~cmdExportPlayerToExternalDatabase() {}

protected:
  Player pl;
};

#endif // CMDEXPORTPLAYERTOEXTERNALDATABASE_H
