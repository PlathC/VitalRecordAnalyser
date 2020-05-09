//
// Created by Platholl on 06/05/2020.
//

#ifndef CIVILREGISTRYANALYSER_DATASETBUILDER_HPP
#define CIVILREGISTRYANALYSER_DATASETBUILDER_HPP

#include <stack>
#include <vector>

#include <QDebug>
#include <QFileDialog>
#include <QKeyEvent>
#include <QListView>
#include <QMainWindow>
#include <QMessageBox>
#include <QTreeView>

#include "ImageUtil.hpp"

#include "DatasetBuilder/ImageSet.hpp"
#include "DatasetBuilder/ImageSegmenterDialog.hpp"

namespace Ui
{
    class DatasetBuilder;
}

namespace DatasetBuilder
{
    class DatasetBuilder : public QMainWindow
    {
    Q_OBJECT
    public:
        explicit DatasetBuilder(QWidget* parent = nullptr);
        void UpdateUi();
        void SelectInputFolders();
        void SelectOutputFolder();

        ~DatasetBuilder() override;

    private slots:
        void SkipCurrentImage();
        void Save();
        void keyPressEvent(QKeyEvent *event) override;

    private:
        void NextImage(bool save);

    private:
        Ui::DatasetBuilder *ui;
        std::vector<DatasetBuilder::ImageSet> m_sets;
        DatasetBuilder::DatasetImage* m_currentImg = nullptr;
        DatasetBuilder::ImageSet* m_currentSet = nullptr;

        bool m_outputFolderSelected = false;
        std::string m_outputFolder;
        fs::path m_transcriptionPath;
    };

}

#endif // CIVILREGISTRYANALYSER_DATASETBUILDER_HPP