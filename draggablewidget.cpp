#include "draggablewidget.h"
#include <QDebug>
#include <QDrag>
#include <QMimeData>
#include <QDragEnterEvent>

PlotWidget::PlotWidget(PlotDataMap *mapped_plot_data, QWidget *parent): QwtPlot(parent)
{
    _plot_data = mapped_plot_data;
    setAcceptDrops(true);
}

void PlotWidget::addCurve(const QString &name)
{
    qDebug() << "attach " << name;
    QwtPlotCurve *curve = new QwtPlotCurve(name);
    PlotData& data = _plot_data->at(name);

    curve->setData( &data  );
    curve->attach( this );

    QColor color;

    int index = 0;
    // todo: store a map with this index
    for(PlotDataMap::iterator it = _plot_data->begin(); it != _plot_data->end(); it++)
    {
        if( it->first.compare( name) == 0) break;
        index++;
    }

    switch( index%9 )
    {
    case 0:  color = QColor(Qt::black) ;break;
    case 1:  color = QColor(Qt::blue);break;
    case 2:  color =  QColor(Qt::red); break;
    case 3:  color =  QColor(Qt::darkGreen); break;
    case 4:  color =  QColor(Qt::magenta); break;
    case 5:  color =  QColor(Qt::darkCyan); break;
    case 6:  color =  QColor(Qt::gray); break;
    case 7: color =  QColor(Qt::darkBlue); break;
    case 8: color =  QColor(Qt::darkYellow); break;
    }

    curve->setPen( color, 1.0 );
    curve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

    this->replot();
}

void PlotWidget::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    QStringList mimeFormats = mimeData->formats();
    foreach(QString format, mimeFormats)
    {
        QByteArray encoded = mimeData->data( format );
        QDataStream stream(&encoded, QIODevice::ReadOnly);

        if( format.contains( "qabstractitemmodeldatalist") )
        {
            event->acceptProposedAction();

            /* while (!stream.atEnd()) {
                int row, col;
                QMap<int,  QVariant> roleDataMap;
                stream >> row >> col >> roleDataMap;
            }*/
        }
        if( format.contains( "plot_area") )
        {
            QString source_name;
            stream >> source_name;

            if(QString::compare( windowTitle(),source_name ) != 0 ){
                event->acceptProposedAction();
            }
        }
    }
}

void PlotWidget::dragMoveEvent(QDragMoveEvent *event)
{

}



void PlotWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    QStringList mimeFormats = mimeData->formats();

    foreach(QString format, mimeFormats)
    {
        QByteArray encoded = mimeData->data( format );
        QDataStream stream(&encoded, QIODevice::ReadOnly);

        if( format.contains( "qabstractitemmodeldatalist") )
        {
            while (!stream.atEnd())
            {
                int row, col;
                QMap<int,  QVariant> roleDataMap;

                stream >> row >> col >> roleDataMap;
                QString itemName = roleDataMap[0].toString();
                addCurve( itemName );
            }
        }
        if( format.contains( "plot_area") )
        {
            QString source_name;
            stream >> source_name;
            emit swapWidgets( source_name, windowTitle() );
        }
    }
}

void PlotWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        QByteArray data;
        QDataStream dataStream(&data, QIODevice::WriteOnly);

        dataStream << this->windowTitle();

        mimeData->setData("plot_area", data );

        drag->setMimeData(mimeData);

        drag->exec();
    }
}
