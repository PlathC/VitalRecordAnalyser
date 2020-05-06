//
// Created by Platholl on 06/05/2020.
//

#include "DatasetBuilder/MainWindow.hpp"
#include "ui_MainWindow.h"

namespace DatasetBuilder {
    MainWindow::MainWindow(QWidget *parent) :
            QMainWindow(parent),
            ui(new Ui::MainWindow)
    {
        ui->setupUi(this);
    }

    void MainWindow::UpdateUi()
    {
        if(m_currentImg)
        {
            auto img = m_currentImg->Image();
            ui->m_lblImg->setPixmap(
                    QPixmap::fromImage(
                            QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888)
                            )
                    );
        }
        else
        {
            //ui->m_lblImg->setPixmap()
            ui->m_pbSkip->setEnabled(false);
            ui->m_pbSave->setEnabled(false);
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
                if(!m_sets.empty())
                {
                    m_currentSet = &m_sets.top();
                    m_sets.pop();
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