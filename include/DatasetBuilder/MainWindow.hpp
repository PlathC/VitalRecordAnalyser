//
// Created by Platholl on 06/05/2020.
//

#ifndef CIVILREGISTRYANALYSER_MAINWINDOW_HPP
#define CIVILREGISTRYANALYSER_MAINWINDOW_HPP

#include <stack>

#include <QDebug>
#include <QFileDialog>
#include <QListView>
#include <QMainWindow>
#include <QTreeView>

#include "DatasetBuilder/ImageSet.hpp"

namespace Ui
{
    class MainWindow;
}

namespace DatasetBuilder
{
    class MainWindow : public QMainWindow
    {
    Q_OBJECT
    public:
        explicit MainWindow(QWidget* parent = nullptr);
        void UpdateUi();
        void SelectInputFolders();

        inline QImage CvMatToQImage( const cv::Mat &inMat );

        ~MainWindow() override;
    private slots:
        void SkipCurrentImage();
        void Save();
    private:
        void NextImage(bool save);

    private:
        Ui::MainWindow *ui;
        std::stack<DatasetBuilder::ImageSet> m_sets;
        DatasetBuilder::DatasetImage* m_currentImg = nullptr;
        DatasetBuilder::ImageSet* m_currentSet = nullptr;
    };

    inline QImage MainWindow::CvMatToQImage( const cv::Mat &inMat )
    {
        switch ( inMat.type() )
        {
            // 8-bit, 4 channel
            case CV_8UC4:
            {
                QImage image( inMat.data,
                              inMat.cols, inMat.rows,
                              static_cast<int>(inMat.step),
                              QImage::Format_ARGB32 );

                return image;
            }

                // 8-bit, 3 channel
            case CV_8UC3:
            {
                QImage image( inMat.data,
                              inMat.cols, inMat.rows,
                              static_cast<int>(inMat.step),
                              QImage::Format_RGB888 );

                return image.rgbSwapped();
            }

                // 8-bit, 1 channel
            case CV_8UC1:
            {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
                QImage image( inMat.data,
                              inMat.cols, inMat.rows,
                              static_cast<int>(inMat.step),
                              QImage::Format_Grayscale8 );
#else
                static QVector<QRgb>  sColorTable;

            // only create our color table the first time
            if ( sColorTable.isEmpty() )
            {
               sColorTable.resize( 256 );

               for ( int i = 0; i < 256; ++i )
               {
                  sColorTable[i] = qRgb( i, i, i );
               }
            }

            QImage image( inMat.data,
                          inMat.cols, inMat.rows,
                          static_cast<int>(inMat.step),
                          QImage::Format_Indexed8 );

            image.setColorTable( sColorTable );
#endif

                return image;
            }

            default:
                qWarning() << "ASM::cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
                break;
        }

        return QImage();
    }
}

#endif // CIVILREGISTRYANALYSER_MAINWINDOW_HPP