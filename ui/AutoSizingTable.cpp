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

#include <cmath>

#include <QHeaderView>
#include <QScrollBar>
#include <QResizeEvent>

#include "AutoSizingTable.h"

namespace GuiHelpers
{

  bool ColumnAutoSizer::setRubberBandCol(int colId)
  {
    if ((colId >= 0) && (colId < colList.size()))
    {
      rubberBandCol = colId;
      return true;
    }

    return false;
  }

  //----------------------------------------------------------------------------

  vector<int> ColumnAutoSizer::getColWidths(int useableWidth) const
  {
    // sum up all relative column widths
    int totalWidthUnits = 0;
    for (const AutosizeColumnDescr& cd : colList)
    {
      totalWidthUnits += cd.relWidth;
    }

    // determine the "physical" width increment per relative unit
    double widthUnit = (useableWidth * 1.0) / (totalWidthUnits * 1.0);

    // prepare the result vector
    vector<int> result;
    double usedWidth = 0;
    int usedWidth_int = 0;

    // calc the widths for each column
    for (const AutosizeColumnDescr& cd : colList)
    {
      // this ugly procedure should distribute rounding
      // errors more evenly over the columns, especially
      // if we have many columns with the same relative
      // width
      double oldWidth = usedWidth;
      usedWidth += cd.relWidth * widthUnit;
      double w_ = usedWidth - oldWidth;

      int w = round(w_);
      usedWidth_int += w;
      result.push_back(w);
    }

    // adjust the total column width to exactly fit the
    // useableWidth
    if (rubberBandCol < 0)
    {
      // default: adjust the last column to account for
      // overall rounding errors
      int widthError = usedWidth_int - useableWidth;
      if (widthError != 0)
      {
        int lastColWidth = result.back();
        lastColWidth -= widthError;
        result.pop_back();
        result.push_back(lastColWidth);
      }
    } else {
      // if we have a rubberband column, apply min/max
      // column widths and assign the remaining width
      // to the rubberband column
      usedWidth_int = 0;
      int idx = 0;
      while (idx < colList.size())
      {
        if (idx != rubberBandCol)
        {
          const AutosizeColumnDescr& cd = colList[idx];

          int w = result[idx];
          if ((cd.absMinWidth > 0) && (w < cd.absMinWidth)) w = cd.absMinWidth;
          if ((cd.absMaxWidth > 0) && (w > cd.absMaxWidth)) w = cd.absMaxWidth;
          result[idx] = w;
          usedWidth_int += w;
        }
        ++idx;
      }
      result[rubberBandCol] = useableWidth - usedWidth_int;
    }

    return result;
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------

  AutoSizingTableWidget::AutoSizingTableWidget(const AutosizeColumnDescrList& colDescr, QWidget* parent)
    :QTableWidget{parent}, ColumnAutoSizer{colDescr}, defaultDelegate{nullptr}, customDelegate{nullptr}
  {
    // set the column headers
    QStringList hdr;
    for (const AutosizeColumnDescr& cd : colList)
    {
      hdr.push_back(cd.colName);
    }
    setColumnCount(hdr.length());
    setHorizontalHeaderLabels(hdr);
    verticalHeader()->hide();

    // disable the horizontal scrollbar
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // set selection mode to "row" and "single"
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    // store the default delegate for later
    defaultDelegate = itemDelegate();
  }

  //----------------------------------------------------------------------------

  AutoSizingTableWidget::~AutoSizingTableWidget()
  {
    if (defaultDelegate != nullptr) delete defaultDelegate;
  }

  //----------------------------------------------------------------------------

  void AutoSizingTableWidget::setCustomDelegate(QAbstractItemDelegate* custDelegate)
  {
    if (custDelegate == nullptr)
    {
      restoreDefaultDelegate();
      return;
    }

    customDelegate.reset(custDelegate);
    setItemDelegate(customDelegate.get());
  }

  //----------------------------------------------------------------------------

  void AutoSizingTableWidget::restoreDefaultDelegate()
  {
    setItemDelegate(defaultDelegate);
    customDelegate.reset();
  }

  //----------------------------------------------------------------------------

  void AutoSizingTableWidget::resizeEvent(QResizeEvent* _event)
  {
    // call parent handler
    QTableWidget::resizeEvent(_event);

    // resize all columns
    autosizeColumns();

    // finish event processing
    _event->accept();
  }

  //----------------------------------------------------------------------------

  void AutoSizingTableWidget::autosizeColumns()
  {
    int widthAvail = width();
    if ((verticalScrollBar() != nullptr) && (verticalScrollBar()->isVisible()))
    {
      widthAvail -= verticalScrollBar()->width();
    }
    auto colWidths = getColWidths(widthAvail);
    int idx = 0;
    while (idx < colWidths.size())
    {
      setColumnWidth(idx, colWidths[idx]);
      ++idx;
    }
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------

  AutoSizingTableWidget_WithDatabase::AutoSizingTableWidget_WithDatabase(const AutosizeColumnDescrList& colDescr, QWidget* parent)
    :AutoSizingTableWidget{colDescr, parent}, db{nullptr}
  {
    setDatabase(nullptr);
  }

  //----------------------------------------------------------------------------

  void AutoSizingTableWidget_WithDatabase::setDatabase(QTournament::TournamentDB* _db)
  {
    if (_db == db) return;
    db = _db;

    clearContents();
    setRowCount(0);

    if (db != nullptr)
    {
      // call custom initialization function
      // for derived classes
      hook_onTournamentOpened();

      // resize columns and rows to content once (we do not want permanent automatic resizing)
      horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
      verticalHeader()->resizeSections(QHeaderView::ResizeToContents);

    } else {
      restoreDefaultDelegate();

      // call custom initialization function
      // for derived classes
      hook_onTournamentClosed();
    }

    setEnabled(db != nullptr);

    // initialize column widths
    autosizeColumns();
  }

}
