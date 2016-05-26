#include "plotview.h"

//==================================================
//  Layout
//
//  14 parts in a single plot
//  6 rows and 5 columns
//
//    0    1    2            3               4
//  +---++---++---++----------------------++---+
//  |   ||   || C ||        Tics          || C |   0
//  |   ||   |+---++----------------------++---+
//  |   || Y |+---++----------------------++---+
//  | Y ||   ||   ||      Title           ||   |   1
//  |   || T ||   |+----------------------+|   |
//  | A || i ||   |+----------------------+|   |
//  | x || c || T ||                      || T |
//  | i ||   || i ||                      || i |
//  | s || L || c ||        Curves        || c |   2
//  |   || a || s ||                      || s |
//  | L || b ||   ||                      ||   |
//  | a || e ||   ||                      ||   |
//  | b || l ||   ||                      ||   |
//  | e || s |+---++----------------------++---+
//  | l ||   |+---++-----------------------+---+
//  |   ||   || C ||       Tics            | C |   3
//  +---++---++---++-----------------------+---+
//  +------------------------------------------+
//  |                 X Tic Labels             |   4
//  +------------------------------------------+
//  +------------------------------------------+
//  |                 X Axis Label             |   5
//  +------------------------------------------+
//
//
//==================================================
// Notes for Transform in rubberband MouseRelease
//
//      ^
//     e3  = (Wox,Woy,1) = (0,0,1)
//    /
//  Wo-e1>-------Ww----------------------------->
//   |\                ^                       /|
//   e2\               |b                     / |
//   v<------a-------->v                     /  |
//   |   \             wo--ww--------->     /   |
//   |    \            /|             |    /    |
//   |     \          / wh            |   /     |
//   |      \        /  v-------------+  /      |
//   |       \      /                   /       |
//   |       Mo----/----Mw------------->        |
//   Wh       |  mo----mw--->          |        |
//   |        |   |mh       |          |        |
//   |       Mh   v---------+          |        |
//   |        |                        |        |
//   |        v------------------------+        |
//   |       /                          \       |
//   |      /                            \      |
//   |     /                              \     |
//   |    /                                \    |
//   |   /                                  \   |
//   |  /                                    \  |
//   | /                                      \ |
//   v-----------------------------------------+
//
//   In a nutshell:
//
//        This is obvious:
//          mw = (Mw/Ww)*ww       (since Ww:Mw as ww:mw)
//          mh = (Mh/Wh)*wh
//
//        The main question is what is T to take wo to mo:
//
//             T?
//          wo -> mo?
//
//   [wo] = +- -+    [mo]  = +- -+
//       W  | a |        M   | ? |
//          | b |            | ? |
//          | 1 |            | 1 |
//          +- -+            +- -+
//
//   [Wo] = +- -+    [Mo]  = +-   -+
//       W  | 0 |        M   | Mox |
//          | 0 |            | Moy |
//          | 1 |            |  1  |
//          +- -+            +-   -+
//
//  Ww, Wh, Wo, Mo, Mw, Mh, a & b are known. What is k11,k12,k21,k22, s & t?
//
//  +-                -+ +- -+   +-                 -+   +- -+
//  | k11?   k12?   s? | | a |   | k11*a + k12*b + s |   | ? |
//  | k21?   k22?   t? | | b | = | k21*a + k22*b + t | = | ? |
//  |   0     0     1  | | 1 |   |        1          |   | 1 |
//  +-                -+ +- -+   +-                 -+   +- -+
//
//  Solution:
//
//  T =  [ T(e1) | T(e2) | T(e3) ]
//
//       +-                 -+
//       | Mw/Ww  0      Mox |
//    =  |   0    Mh/Wh  Moy |
//       |   0    0       1  |
//       +-                 -+
//
// Note:
//       +-                 -+ +- -+   +-   -+
//       | Mw/Ww  0      Mox | | 0 |   | Mox |
//       |   0    Mh/Wh  Moy | | 0 | = | Moy |  works
//       |   0    0       1  | | 1 |   |  1  |
//       +-                 -+ +- -+   +-   -+
//
//       +-                 -+ +-  -+   +-        -+
//       | Mw/Ww  0      Mox | | Ww |   | Mw + Mox |
//       |   0    Mh/Wh  Moy | | Wh | = | Mh + Moy |  works
//       |   0    0       1  | |  1 |   |    1     |
//       +-                 -+ +-  -+   +-        -+
//
// In general:
//       +-                 -+ +- -+   +-               -+
//       | Mw/Ww  0      Mox | | a |   | (Mw/Ww)*a + Mox |
//       |   0    Mh/Wh  Moy | | b | = | (Mh/Wh)*b + Moy |
//       |   0    0       1  | | 1 |   |          1      |
//       +-                 -+ +- -+   +-               -+
//
// So, k12 == k21 == 0
//     k11 = Mw/Ww, k22 = Mh/Wh
//     s = Mox, t = Moy
//


PlotView::PlotView(QWidget *parent) :
    BookIdxView(parent),
    _titleView(0),
    _xAxisLabel(0),
    _yAxisLabel(0),
    _tlCorner(0), _trCorner(0), _blCorner(0), _brCorner(0),
    _curvesView(0),
    _tTics(0), _bTics(0), _rTics(0), _lTics(0),
    _xTicLabels(0), _yTicLabels(0),
    _rubberBand(0)
{
    setFrameShape(QFrame::NoFrame);

    _grid = new QGridLayout;
    _grid->setContentsMargins(0, 0, 0, 0);
    _grid->setSpacing(0);

    _titleView  = new PlotTitleView(this);
    _xAxisLabel = new XAxisLabelView(this);
    _yAxisLabel = new YAxisLabelView(this);
    _tlCorner   = new CornerView(Qt::TopLeftCorner,this);
    _trCorner   = new CornerView(Qt::TopRightCorner,this);
    _blCorner   = new CornerView(Qt::BottomLeftCorner,this);
    _brCorner   = new CornerView(Qt::BottomRightCorner,this);
    _tTics      = new LinedRulerView(Qt::AlignTop,this);
    _bTics      = new LinedRulerView(Qt::AlignBottom,this);
    _rTics      = new LinedRulerView(Qt::AlignRight,this);
    _lTics      = new LinedRulerView(Qt::AlignLeft,this);
    _xTicLabels = new LabeledRulerView(Qt::AlignBottom,this);
    _yTicLabels = new LabeledRulerView(Qt::AlignLeft,this);
    _curvesView = new CurvesView(this);

    _grid->addWidget(_yAxisLabel,0,0,4,1);
    _grid->addWidget(_yTicLabels,0,1,4,1);
    _grid->addWidget(  _tlCorner,0,2,1,1);
    _grid->addWidget(     _tTics,0,3,1,1);
    _grid->addWidget(  _trCorner,0,4,1,1);
    _grid->addWidget(     _lTics,1,2,2,1);
    _grid->addWidget( _titleView,1,3,1,1);
    _grid->addWidget(     _rTics,1,4,2,1);
    _grid->addWidget(_curvesView,2,3,1,1);
    _grid->addWidget(  _blCorner,3,2,1,1);
    _grid->addWidget(     _bTics,3,3,1,1);
    _grid->addWidget(  _brCorner,3,4,1,1);
    _grid->addWidget(_xTicLabels,4,1,1,5);
    _grid->addWidget(_xAxisLabel,5,1,1,5);

    _grid->setRowStretch(0,1);
    _grid->setRowStretch(1,1);
    _grid->setRowStretch(2,100);
    _grid->setRowStretch(3,1);
    _grid->setRowStretch(4,1);
    _grid->setRowStretch(5,1);

    _grid->setColumnStretch(0,1);
    _grid->setColumnStretch(1,1);
    _grid->setColumnStretch(2,1);
    _grid->setColumnStretch(3,100);
    _grid->setColumnStretch(4,1);

    _childViews << _titleView
                << _xAxisLabel
                << _yAxisLabel
                << _curvesView
                << _tlCorner <<  _trCorner << _blCorner << _brCorner
                << _tTics <<  _bTics << _rTics <<  _lTics
                << _xTicLabels << _yTicLabels
                << _curvesView;


    foreach ( QWidget* widget, _childViews ) {
        widget->installEventFilter(this);
    }

    setLayout(_grid);
}

QSize PlotView::minimumSizeHint() const
{
    QSize s(50,50);
    return s;
}

QSize PlotView::sizeHint() const
{
    QSize s(200,200);
    return s;
}

void PlotView::rowsInserted(const QModelIndex &pidx, int start, int end)
{
    Q_UNUSED(pidx);
    Q_UNUSED(start);
    Q_UNUSED(end);
}

bool PlotView::eventFilter(QObject *obj, QEvent *event)
{
    bool isHandled = true;

    QWidget* widget = 0;
    foreach ( QWidget* w, _childViews ) {
        if ( obj == w ) {
            widget = w;
            break;
        }
    }
    if ( !widget ) {
        return QObject::eventFilter(obj, event);
    }

    if (event->type() ==  QEvent::MouseButtonPress ) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if ( mouseEvent->button() == Qt::MidButton ){
            _rubberBandOrigin = widget->mapTo(this,mouseEvent->pos());
            if ( !_rubberBand ) {
                _rubberBand = new QRubberBand(QRubberBand::Rectangle,this);
            }
            _rubberBand->setGeometry(QRect(_rubberBandOrigin, QSize()));
            _rubberBand->show();
        } else if ( mouseEvent->button() == Qt::RightButton ){
            //zoomToFit();
        }
    } else if ( event->type() == QEvent::MouseMove ) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if ( mouseEvent->buttons() == Qt::MidButton && _rubberBand ){
            QPoint pt = widget->mapTo(this,mouseEvent->pos());
            _rubberBand->setGeometry(QRect(_rubberBandOrigin,pt).normalized());
        }
    } else if ( event->type() == QEvent::MouseButtonRelease ) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if ( mouseEvent->button() == Qt::MidButton && _rubberBand ){
            QRect geom = _rubberBand->geometry();
            if ( geom.width() > 20 && geom.height() > 20 ) {
                // If rubberband too small,
                // normally by accidental click, don't zoom
                int Ww = viewport()->width();
                int Wh = viewport()->height();
                if ( Ww != 0 && Wh != 0 ) {
                    QRectF M = _mathRect();
                    double Mw = M.width();
                    double Mh = M.height();
                    double Mox = M.topLeft().x();
                    double Moy = M.topLeft().y();
                    QTransform T(Mw/Ww, 0.0,  // See comment above for details
                                 0.0, Mh/Wh,
                                 Mox, Moy);
                    QPointF wo((double)geom.topLeft().x(),
                               (double)geom.topLeft().y());
                    QPointF wbr((double)geom.bottomRight().x(),
                                (double)geom.bottomRight().y());
                    QPointF mo  = T.map(wo);
                    QPointF mbr = T.map(wbr);
                    QRectF mrect(mo,mbr);
                    _setPlotMathRect(mrect);
                    _curvesView->viewport()->update();
                    foreach ( QAbstractItemView* childView, _childViews ) {
                        childView->viewport()->update();
                    }
                }
            }
            _rubberBand->hide();
        }
    } else {
        // standard event processing
        isHandled = QObject::eventFilter(obj, event);
    }

    return isHandled;
}

void PlotView::_update()
{
    if ( !_curvesView ) {
        qDebug() << "snap [bad scoobs]: _curvesView should be set in "
                    "PlotView.";
        exit(-1);
    }
    foreach (QAbstractItemView* view, _childViews ) {
        if ( view ) {
            view->setModel(model());
            view->setRootIndex(_myIdx);
        }
        BookIdxView* bview = dynamic_cast<BookIdxView*>(view);
        if ( bview ) {
            bview->setCurvesView(_curvesView);
        }
    }
    this->setCurvesView(_curvesView);
    if ( model() ) {
        disconnect(model(),SIGNAL(rowsInserted(QModelIndex,int,int)),
                   this,SLOT(rowsInserted(QModelIndex,int,int)));
    }
}
