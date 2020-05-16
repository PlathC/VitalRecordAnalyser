//
// Created by Platholl on 16/05/2020.
//

#ifndef CIVILREGISTRYANALYSER_CIVILREGISTRYANALYZER_HPP
#define CIVILREGISTRYANALYSER_CIVILREGISTRYANALYZER_HPP

#include <QMainWindow>

namespace Ui
{
    class CivilRegistryAnalyzer;
}

namespace CivilRegistryAnalyzer
{
    class CivilRegistryAnalyzer : public QMainWindow
    {
    Q_OBJECT
    public:
        explicit CivilRegistryAnalyzer(QWidget* parent = nullptr);

        ~CivilRegistryAnalyzer() override;

    private:
        Ui::CivilRegistryAnalyzer* ui;
    };
}



#endif //CIVILREGISTRYANALYSER_CIVILREGISTRYANALYZER_HPP
