//
// Created by Platholl on 16/05/2020.
//

#include <QApplication>

#include "Ui/CivilRegistryAnalyzer.hpp"

int main(int argc, char** argv)
{
    QApplication app{argc, argv};
    CivilRegistryAnalyzer::CivilRegistryAnalyzer window;
    window.show();

    return app.exec();
}