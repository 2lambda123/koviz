#include "bookmodel.h"
#include <float.h>
#include "libsnap/unit.h"

PlotBookModel::PlotBookModel(MonteModel *monteModel, QObject *parent) :
    QStandardItemModel(parent),
    _monteModel(monteModel)
{
    _initModel();
}

PlotBookModel::PlotBookModel(MonteModel *monteModel,
                             int rows, int columns, QObject *parent) :
    QStandardItemModel(rows,columns,parent),
    _monteModel(monteModel)
{
    _initModel();
}

QStandardItem *PlotBookModel::addChild(QStandardItem *parentItem,
                                       const QString &childTitle,
                                       const QVariant &childValue)
{
    QStandardItem *columnZeroItem = new QStandardItem(childTitle);
    QStandardItem *columnOneItem = new QStandardItem(childValue.toString());

    QList<QStandardItem*> items;
    items << columnZeroItem << columnOneItem;
    parentItem->appendRow(items);

    QModelIndex columnOneIdx = indexFromItem(columnOneItem);
    setData(columnOneIdx,childValue);

    return columnZeroItem;
}

// Returns page index for idx
// If idx is a child of a page, it will return the parent page index
QModelIndex PlotBookModel::_pageIdx(const QModelIndex& idx) const
{
    QModelIndex pgIdx;

    QModelIndex p1Idx = idx.parent();
    QModelIndex p2Idx = p1Idx.parent();
    QModelIndex p3Idx = p2Idx.parent();
    QModelIndex p4Idx = p3Idx.parent();

    if ( isIndex(idx, "Page" ) ) {
        pgIdx = idx;
    } else if ( isIndex(p1Idx, "Page" ) ) {
        pgIdx = p1Idx;
    } else if ( isIndex(p2Idx, "Page" ) ) {
        pgIdx = p2Idx;
    } else if ( isIndex(p3Idx, "Page" ) ) {
        pgIdx = p3Idx;
    } else if ( isIndex(p4Idx, "Page" ) ) {
        pgIdx = p4Idx;
    }

    return pgIdx;
}


QModelIndexList PlotBookModel::pageIdxs() const
{
    return getIndexList(QModelIndex(), "Page");
}

QModelIndex PlotBookModel::_plotIdx(const QModelIndex &idx) const
{
    QModelIndex pltIdx;

    QModelIndex p1Idx = idx.parent();
    QModelIndex p2Idx = p1Idx.parent();
    QModelIndex p3Idx = p2Idx.parent();

    if ( isIndex(idx, "Plot" ) ) {
        pltIdx = idx;
    } else if ( isIndex(p1Idx, "Plot" ) ) {
        pltIdx = p1Idx;
    } else if ( isIndex(p2Idx, "Plot" ) ) {
        pltIdx = p2Idx;
    } else if ( isIndex(p3Idx, "Plot" ) ) {
        pltIdx = p3Idx;
    } else {
        qDebug() << "snap [bad scoobies]: PlotBookModel::_plotIdx() failed.";
        exit(-1);
    }

    return pltIdx;
}

QModelIndex PlotBookModel::_ancestorIdx(const QModelIndex &startIdx,
                                    const QString& ancestorText,
                                    const QString &expectedStartIdxText) const
{
    QModelIndex idx;

    if ( !startIdx.isValid() ) {
        return idx;
    }

    if ( !expectedStartIdxText.isEmpty() ) {
        if ( !isIndex(startIdx, expectedStartIdxText) ) {
            QStandardItem* startItem = itemFromIndex(startIdx);
            QString startText = startItem->text();
            qDebug() << "snap [bad scoobies]: _ancestorIdx() received a "
                        "startIdx of " << startIdx << " with text "
                     << startText << ".  The expected start item text was "
                     << expectedStartIdxText << ".  The ancestor to find was "
                     << ancestorText << ".";
            exit(-1);
        }
    }

    idx = startIdx;
    while (idx.isValid() && !isIndex(idx, ancestorText)) {
        idx = idx.parent();
    }

    return idx;
}

QModelIndexList PlotBookModel::plotIdxs(const QModelIndex &pageIdx) const
{
    QModelIndex plotsIdx = getIndex(pageIdx,"Plots", "Page");
    return getIndexList(plotsIdx, "Plot", "Plots");
}

QModelIndexList PlotBookModel::curveIdxs(const QModelIndex &curvesIdx) const
{
    return getIndexList(curvesIdx, "Curve", "Curves");
}

QList<QColor> PlotBookModel::createCurveColors(int nCurves)
{
    QList<QColor> colors;

    QColor burntorange(177,79,0); // hook'em!

    if ( nCurves < 10 ) {
        // Handpicked colors for smaller number of curves
        QColor blue(48,85,200);
        QColor red(200,30,30);
        QColor magenta(130,15,120);
        QColor green(60,180,45);
        QColor yellow(222,222,10);
        QColor gray(145,170,192);
        QColor pink(255,192,255);
        QColor medblue(49,140,250);
        QList<QColor> handPickedColors;
        handPickedColors << blue << red << magenta
                         << green << burntorange << yellow
                         << gray << pink << medblue;
        for (int i = 0; i < nCurves; ++i ) {
            colors << handPickedColors.at(i);
        }
    } else {
        // This is for "rainbow banding" primarily for many monte carlo curves
        int nBands = 10;
        int hBeg = 10; int hEnd = 230;
        int q = div(nCurves-1,nBands).quot;
        for (int i = 0; i < nCurves; ++i ) {
            int h = qRound((double)(hEnd-hBeg)/(nBands-1.0))*div(i,q).quot+hBeg;
            int s = qRound(0.75*255);
            int v = qRound(0.87*255);
            colors << QColor::fromHsv(h,s,v);
        }
        colors.removeFirst();
        colors.prepend(burntorange);  // Longhorns #0!

    }

    return colors;
}

void PlotBookModel::_initModel()
{
    setColumnCount(2);
    QStandardItem *rootItem = invisibleRootItem();
    QStandardItem *citem;

    citem = addChild(rootItem, "SessionStartTime",-DBL_MAX);
    citem = addChild(rootItem, "SessionStopTime",DBL_MAX);
    citem = addChild(rootItem, "Pages","");
    citem = addChild(rootItem, "Tables","");
}

//
// Convenience method for obtaining QModelIndexes using search strings (tags)
//
// expectedStartIdxText should match startIdx's text.
// It's more for debugging, but helps ensure that one is in sync with the tree.
//
// StartIdx is normally the immediate parent of the item with searchItemText.
// The odd case is for Pages and Plots.  The search can go ancestorily (sp?).
// For instance:
//     getIndex(curveIdx, "Plot")
// will return the plotIdx that is the parent of curveIdx
//
QModelIndex PlotBookModel::getIndex(const QModelIndex &startIdx,
                                 const QString &searchItemText,
                                 const QString &expectedStartIdxText) const
{
    QModelIndex idx;

    // For Page, the search may go up the tree
    if ( searchItemText == "Page" ) {
        idx = _pageIdx(startIdx);
        return idx;
    }

    // For Plot, the search may go up the tree
    if ( searchItemText == "Plot" ) {
        idx = _plotIdx(startIdx);
        return idx;
    }

    // For Table, the search may go up the tree
    if ( searchItemText == "Table" ) {
        idx = _ancestorIdx(startIdx, "Table", expectedStartIdxText);
        if ( idx.isValid() ) {
            return idx;
        }
    }

    if ( !expectedStartIdxText.isEmpty() ) {
        if ( startIdx.isValid() ) {
            if ( !isIndex(startIdx, expectedStartIdxText) ) {
                QStandardItem* startItem = itemFromIndex(startIdx);
                QString startText = startItem->text();
                qDebug() << "snap [bad scoobies]:1: getIndex() received a "
                            "startIdx of " << startIdx << " with item text "
                         << startText << ".  The expected start item text was "
                         << expectedStartIdxText << ".";
                exit(-1);
            }
        } else {
            qDebug() << "snap [bad scoobies]:2: getIndex() received an "
                        "invalid startIdx. The startIdx "
                        "was expected to have this text "
                     << expectedStartIdxText << ".";
            exit(-1);
        }
    }

    if ( !startIdx.isValid() ) {
        if ( searchItemText == "SessionStartTime" ) {
            idx = index(0,0);
        } else if ( searchItemText == "SessionStopTime" ) {
            idx = index(1,0);
        } else if ( searchItemText == "Pages" ) {
            idx = index(2,0);
        } else if ( searchItemText == "Tables" ) {
            idx = index(3,0);
        } else if ( searchItemText == "DefaultPageTitles" ) {
            idx = index(4,0);
        } else {
            qDebug() << "snap [bad scoobies]:3: getIndex() received "
                        "root as a startIdx and had bad child item text of \""
                     << searchItemText
                     << "\".";
            exit(-1);
        }
    } else {
        QStringList cStrings;
        int rc = itemFromIndex(startIdx)->rowCount();
        bool isFound = false;
        for ( int i = 0; i < rc; ++i ) {
            QModelIndex cIdx = index(i,0,startIdx);
            QString cText = itemFromIndex(cIdx)->text();
            cStrings << cText;
            if ( cText == searchItemText ) {
                idx = cIdx;
                isFound = true;
                break;
            }
        }
        if ( !isFound ) {
            if ( !expectedStartIdxText.isEmpty() ) {
                qDebug()
                    << "snap [bad scoobies]:4: getIndex() received start item= "
                    << expectedStartIdxText
                    << ".  Unable to find a child with the item text "
                    << searchItemText
                    << " for that parent.  Child items found were: "
                    << cStrings;
            } else {
                qDebug()
                    << "snap [bad scoobies]:5: getIndex() received startIdx="
                    << startIdx
                    << ".  Unable to find a child with the item text "
                    << searchItemText << ".  Child items found were: "
                    << cStrings;
            }
            exit(-1);
        }
    }

    return idx;
}

QModelIndex PlotBookModel::getDataIndex(const QModelIndex &startIdx,
                                    const QString &searchItemText,
                                    const QString &expectedStartIdxText) const
{
    QModelIndex tagIdx = getIndex(startIdx,searchItemText,expectedStartIdxText);
    QModelIndex dataIdx = sibling(tagIdx.row(),1,tagIdx);
    return dataIdx;
}

QString PlotBookModel::getDataString(const QModelIndex &startIdx,
                                     const QString &searchItemText,
                                     const QString &expectedStartIdxText) const
{
    QModelIndex tagIdx = getIndex(startIdx,searchItemText,expectedStartIdxText);
    QModelIndex dataIdx = sibling(tagIdx.row(),1,tagIdx);
    return data(dataIdx).toString();
}

double PlotBookModel::getDataDouble(const QModelIndex &startIdx,
                                     const QString &searchItemText,
                                     const QString &expectedStartIdxText) const
{
    QModelIndex tagIdx = getIndex(startIdx,searchItemText,expectedStartIdxText);
    QModelIndex dataIdx = sibling(tagIdx.row(),1,tagIdx);
    bool ok;
    double d = data(dataIdx).toDouble(&ok);
    if ( !ok ) {
        qDebug() << "snap [bad scoobs]: PlotBookModel::getDataDouble()";
        exit(-1);
    }
    return d;
}

int PlotBookModel::getDataInt(const QModelIndex &startIdx,
                                 const QString &searchItemText,
                                 const QString &expectedStartIdxText) const
{
    QModelIndex dataIdx = getDataIndex(startIdx,searchItemText,
                                      expectedStartIdxText);
    bool ok;
    int i = data(dataIdx).toInt(&ok);
    if ( !ok ) {
        qDebug() << "snap [bad scoobs]: PlotBookModel::getDataInt()";
        exit(-1);
    }
    return i;
}

QModelIndexList PlotBookModel::getIndexList(const QModelIndex &startIdx,
                                  const QString &searchItemText,
                                  const QString &expectedStartIdxText) const
{
    QModelIndexList idxs;

    QString startItemText;
    if ( startIdx.isValid() ) {
        startItemText = itemFromIndex(startIdx)->text();
    }

    //
    // Get parent idx (pidx) for index list
    //
    QModelIndex pidx = startIdx;
    if ( searchItemText == "Page" ) {
        pidx = getIndex(QModelIndex(), "Pages");
    } else if ( searchItemText == "Plot" ) {
        // Made so startIdx doesn't have to be the parent PageIdx
        QModelIndex pageIdx = getIndex(startIdx, "Page", expectedStartIdxText);
        if ( !pageIdx.isValid() ) {
            qDebug() << "snap [bad scoobies]: getIndexList() received a "
                        "startIdx of \"" << startItemText << "\""
                    << startIdx << " with search item text " << searchItemText
                    << ".  Unable to find list for the item text.";
            exit(-1);
        }
        pidx = getIndex(pageIdx, "Plots", "Page");
    }

    int rc = rowCount(pidx);
    for ( int i = 0; i < rc; ++i ) {
        idxs.append(index(i,0,pidx));
    }

    return idxs;
}

bool PlotBookModel::isIndex(const QModelIndex &idx, const QString &itemText) const
{
    if ( !idx.isValid() ) return false;
    return ( itemFromIndex(idx)->text() == itemText );
}

bool PlotBookModel::isChildIndex(const QModelIndex &pidx,
                                 const QString& expectedParentItemText,
                                 const QString &childItemText) const
{
    bool isChild = false;

    if (!isIndex(pidx,expectedParentItemText)) return false;

    int rc = itemFromIndex(pidx)->rowCount();
    for ( int i = 0; i < rc; ++i ) {
        QModelIndex cIdx = index(i,0,pidx);
        QString cText = itemFromIndex(cIdx)->text();
        if ( cText == childItemText ) {
            isChild = true;
            break;
        }
    }

    return isChild;
}
