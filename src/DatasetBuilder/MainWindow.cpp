//
// Created by Platholl on 06/05/2020.
//

#include "DatasetBuilder/MainWindow.hpp"
#include "ui_MainWindow.h"

namespace DatasetBuilder
{
    MainWindow::MainWindow(QWidget *parent) :
            QMainWindow(parent),
            ui(new Ui::MainWindow)
    {
        ui->setupUi(this);
        QObject::connect(ui->m_acInputFolders,
                &QAction::triggered,
                [&](){ SelectInputFolders(); }
                );
    }

    void MainWindow::UpdateUi()
    {
        if(m_currentImg)
        {
            auto img = m_currentImg->Image();
            cv::Mat nImg;
            int width  = img.cols,
                height = img.rows;

            int target_width = ui->m_lblImg->width();
            cv::Mat square   = cv::Mat::zeros( target_width, target_width, img.type() );

            int max_dim = ( width >= height ) ? width : height;
            float scale = ( ( float ) target_width ) / max_dim;
            cv::Rect roi;
            if ( width >= height )
            {
                roi.width = target_width;
                roi.x = 0;
                roi.height = height * scale;
                roi.y = ( target_width - roi.height ) / 2;
            }
            else
            {
                roi.y = 0;
                roi.height = target_width;
                roi.width = width * scale;
                roi.x = ( target_width - roi.width ) / 2;
            }

            cv::resize( img, square( roi ), roi.size());

            ui->m_lblImg->setPixmap(
                    QPixmap::fromImage(
                            CvMatToQImage(square)
                            )
                    );
        }
        else
        {
            ui->m_lblImg->setText("No selected file");
            ui->m_pbSkip->setEnabled(false);
            ui->m_pbSave->setEnabled(false);
        }
    }

    void MainWindow::SelectInputFolders()
    {
        auto* dialog = new QFileDialog(this);

        // Change dialog behaviour to accept multi-folders selection
        dialog->setFileMode(QFileDialog::DirectoryOnly);
        dialog->setOption(QFileDialog::DontUseNativeDialog, true);

        auto* fileView = dialog->findChild<QListView*>("listView");
        if(fileView)
            fileView->setSelectionMode(QAbstractItemView::MultiSelection);

        auto* fileTreeView = dialog->findChild<QTreeView*>();
        if(fileTreeView)
            fileTreeView->setSelectionMode(QAbstractItemView::MultiSelection);

        if(dialog->exec())
        {
            QStringList files = dialog->selectedFiles();
            for(const auto& file : files)
                m_sets.push(DatasetBuilder::ImageSet(file.toStdString()));

            if(!m_currentImg)
            {
                m_currentSet = &m_sets.top();
                m_currentImg = &m_currentSet->CurrentImage();
            }

            UpdateUi();
        }
    }

    void MainWindow::SkipCurrentImage()
    {
        NextImage(false);
    }

    void MainWindow::Save()
    {
        m_currentImg->Text(ui->m_teImg->toPlainText().toStdString());
        NextImage(true);
    }

    void MainWindow::NextImage(bool save)
    {
        if(m_currentSet)
        {
            if(!m_currentSet->Skip(save))
            {
                m_sets.pop();
                if(!m_sets.empty())
                {
                    m_currentSet = &m_sets.top();
                    m_currentImg = &m_currentSet->CurrentImage();
                }
                else
                {
                    m_currentSet = nullptr;
                    m_currentImg = nullptr;
                }
            }
            else
            {
                m_currentImg = &m_currentSet->CurrentImage();
            }
            UpdateUi();
        }
    }

    MainWindow::~MainWindow()
    {
        delete ui;
    }
}