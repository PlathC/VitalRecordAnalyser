//
// Created by Platholl on 06/05/2020.
//

#ifndef CIVILREGISTRYANALYSER_MAINWINDOW_HPP
#define CIVILREGISTRYANALYSER_MAINWINDOW_HPP

#include <stack>

#include <QMainWindow>

#include "DatasetBuilder/ImageSet.hpp"

namespace Ui {
    class MainWindow;
}

namespace DatasetBuilder {
    class MainWindow : public QMainWindow {
    Q_OBJECT
    public:
        explicit MainWindow(QWidget* parent = nullptr);

        ~MainWindow() override;
    private slots:
        void UpdateUi();
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
}

#endif // CIVILREGISTRYANALYSER_MAINWINDOW_HPP