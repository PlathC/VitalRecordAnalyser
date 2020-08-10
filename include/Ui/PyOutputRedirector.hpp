//
// Created by Platholl on 10/08/2020.
//

#ifndef CIVILREGISTRYANALYSER_PYOUTPUTREDIRECTOR_HPP
#define CIVILREGISTRYANALYSER_PYOUTPUTREDIRECTOR_HPP

#include <QObject>
#include <QTimer>
#include <QThread>

#undef slots
#include <pybind11/embed.h>
#include <pybind11/iostream.h>
#define slots Q_SLOTS

namespace py = pybind11;

namespace CivilRegistryAnalyser
{
    // based on https://stackoverflow.com/questions/58758429/pybind11-redirect-python-sys-stdout-to-c-from-print
    class PyOutputRedirector : public QObject
    {
        Q_OBJECT

    public:
        PyOutputRedirector();

        std::string StdoutContent();
        std::string StdErrContent();

        ~PyOutputRedirector() override;

    signals:
        void newStdOutContent(QString content);
        void newStdErrContent(QString content);

    private:
        py::object m_stdout;
        py::object m_stderr;
        py::object m_stdoutBuffer;
        py::object m_stderrBuffer;
        QThread* threadHandle;
    };    
}

#endif //CIVILREGISTRYANALYSER_PYOUTPUTREDIRECTOR_HPP
