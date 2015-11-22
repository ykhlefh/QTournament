#ifndef CMDBULKADDPLAYERTOCAT_H
#define CMDBULKADDPLAYERTOCAT_H

#include <QObject>

#include "AbstractCommand.h"
#include "Category.h"

using namespace QTournament;

class cmdBulkAddPlayerToCategory : public AbstractCommand, public QObject
{
public:
  cmdBulkAddPlayerToCategory(QWidget* p, const Category& _cat);
  virtual ERR exec() override;
  virtual ~cmdBulkAddPlayerToCategory() {}

protected:
  Category cat;
};

#endif // CMDREGISTERPLAYER_H