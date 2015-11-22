#include <QObject>
#include <QMessageBox>

#include "Tournament.h"
#include "cmdMoveOrCopyPlayerToCategory.h"
#include "ui/DlgSelectPlayer.h"

cmdMoveOrCopyPlayerToCategory::cmdMoveOrCopyPlayerToCategory(QWidget* p, const Player& _pl, const Category& _srcCat, const Category& _dstCat, bool _isMove)
  :AbstractCommand(p), pl(_pl), srcCat(_srcCat), dstCat(_dstCat), isMove(_isMove)
{

}

//----------------------------------------------------------------------------

ERR cmdMoveOrCopyPlayerToCategory::exec()
{
  auto cm = Tournament::getCatMngr();

  // check if the player is in the source category
  if (!(srcCat.hasPlayer(pl)))
  {
    QString msg = tr("The player is not assigned to the source category of this operation.");
    QMessageBox::warning(parentWidget, tr("Move or copy player"), msg);
    return PLAYER_NOT_IN_CATEGORY;
  }

  // if this is a move operation: make sure we can actually delete
  // the player from the source category
  if (isMove && (!(srcCat.canRemovePlayer(pl))))
  {
    QString msg = tr("The player cannot be removed from the source category of this operation.");
    QMessageBox::warning(parentWidget, tr("Move player"), msg);
    return PLAYER_NOT_REMOVABLE_FROM_CATEGORY;
  }

  // if this is a copy operation and the player is already in
  // the target category, there is nothing to do for us
  bool dstCatHasPlayer = dstCat.hasPlayer(pl);
  if (!isMove && dstCatHasPlayer)
  {
    return OK;
  }

  // if this is a move operation and the player is already in
  // the target category, this boils down to a simple deletion
  if (isMove && dstCatHasPlayer)
  {
    ERR err = cm->removePlayerFromCategory(pl, srcCat);

    if (err != OK)   // shouldn't happen after the previous check, but anyway...
    {
      QString msg = tr("The player cannot be removed from the source category of this operation.");
      QMessageBox::warning(parentWidget, tr("Move player"), msg);
    }

    return err;
  }

  // try to add the player to the target category
  ERR err = cm->addPlayerToCategory(pl, dstCat);
  if (err != OK)
  {
    QString msg = tr("The player cannot be added to the target category of this operation.");
    QMessageBox::warning(parentWidget, tr("Move or copy player"), msg);
    return err;
  }

  // if this is a move operation, delete the player from the source
  if (isMove)
  {
    ERR err = cm->removePlayerFromCategory(pl, srcCat);

    if (err != OK)   // shouldn't happen after the previous check, but anyway...
    {
      QString msg = tr("The player cannot be removed from the source category of this operation.");
      QMessageBox::warning(parentWidget, tr("Move player"), msg);
    }

    return err;
  }

  return OK;
}
