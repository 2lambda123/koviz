#include "plotbookmodel.h"
#include <float.h>

PlotBookModel::PlotBookModel(QObject *parent) :
    QStandardItemModel(parent)
{
    _initModel();
}

PlotBookModel::PlotBookModel(int rows, int columns, QObject *parent) :
    QStandardItemModel(rows,columns,parent)
{
    _initModel();
}

QModelIndexList PlotBookModel::pageIdxs() const
{
    QModelIndexList idxs;
    QStandardItem* rootItem = this->invisibleRootItem();
    for ( int i = 2; i < rootItem->rowCount(); ++i ) {  // 2 for start/stop time
        QStandardItem* item = rootItem->child(i);
        QModelIndex idx = this->indexFromItem(item);
        idxs.append(idx);
    }
    return idxs;
}

QModelIndexList PlotBookModel::plotIdxs(const QModelIndex &pageIdx) const
{
    QModelIndexList idxs;
    int rc = rowCount(pageIdx);
    // Start at 3 because page title=0, startTime=1, stopTime=2
    for ( int i = 3 ; i < rc; ++i ) {
        idxs.append(index(i,0,pageIdx));
    }

    return idxs;
}

QModelIndex PlotBookModel::sessionStartIdx() const
{
    QStandardItem *rootItem = invisibleRootItem();
    QStandardItem *startItem = rootItem->child(0);
    QModelIndex startIdx = indexFromItem(startItem);
    return startIdx;
}

QModelIndex PlotBookModel::sessionStopIdx() const
{
    QStandardItem *rootItem = invisibleRootItem();
    QStandardItem *stopItem = rootItem->child(1);
    QModelIndex stopIdx = indexFromItem(stopItem);
    return stopIdx;
}


bool PlotBookModel::isPageIdx(const QModelIndex &idx) const
{
    // rows 0,1 are start/stop time
    return (idx.isValid() && !idx.parent().isValid()
            && idx.row() != 0 && idx.row() != 1 ) ;
}

void PlotBookModel::_initModel()
{
    QStandardItem *rootItem = invisibleRootItem();

    QStandardItem *startItem = new QStandardItem("start");
    startItem->setData(-DBL_MAX);
    rootItem->appendRow(startItem);
    QModelIndex startIdx = indexFromItem(startItem);
    setData(startIdx,-DBL_MAX);

    QStandardItem *stopItem = new QStandardItem("stop");
    stopItem->setData(DBL_MAX);
    rootItem->appendRow(stopItem);
    QModelIndex stopIdx = indexFromItem(stopItem);
    setData(stopIdx,DBL_MAX);
}

PlotBookModel::IdxEnum PlotBookModel::indexEnum(const QModelIndex &idx)
{
    PlotBookModel::IdxEnum ret = Invalid;

    int row = idx.row();
    QModelIndex pidx   = parent(idx);
    QModelIndex gpidx  = parent(pidx);
    QModelIndex g2pidx = parent(gpidx);
    QModelIndex g3pidx = parent(g2pidx);
    QModelIndex g4pidx = parent(g3pidx);

    if ( isPageIdx(idx) ) {
        ret = Page;
    } else if ( !pidx.isValid()  && row == 0 ) {
        ret = SessionStartTime;
    } else if ( !pidx.isValid()  && row == 1 ) {
        ret = SessionStopTime;
    } else if ( ! gpidx.isValid() && row == 0 ) {
        ret = PageTitle;
    } else if ( ! gpidx.isValid() && row == 1 ) {
        ret = PageStartTime;
    } else if ( ! gpidx.isValid() && row == 2 ) {
        ret = PageStopTime;
    } else if ( ! gpidx.isValid() && row >= 3 ) {
        ret = Plot;
    } else if ( ! g2pidx.isValid() ) {
        // Plot elements
        switch (row) {
            case 0: ret = PlotXAxisLabel; break;
            case 1: ret = PlotYAxisLabel; break;
            case 2: ret = Curves; break;
            case 3: ret = PlotTitle; break;
            case 4: ret = PlotXMin; break;
            case 5: ret = PlotXMax; break;
            case 6: ret = PlotYMin; break;
            case 7: ret = PlotYMax; break;
            case 8: ret = PlotStartTime; break;
            case 9: ret = PlotStopTime; break;
            default: ret = Invalid;
        }
    } else if ( ! g3pidx.isValid() ) {
        ret = Curve;
    } else if ( ! g4pidx.isValid() ) {
        // Curve Elements
        switch (row) {
            case 0: ret = CurveTime; break;
            case 1: ret = CurveX; break;
            case 2: ret = CurveY; break;
            case 3: ret = CurveTimeUnit; break;
            case 4: ret = CurveXUnit; break;
            case 5: ret = CurveYUnit; break;
            case 6: ret = CurveRun; break;
            case 7: ret = CurveLineColor; break;
            default: ret = Invalid; break;
        }
    }

    return ret;
}
