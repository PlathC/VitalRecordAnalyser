//
// Created by Platholl on 10/08/2020.
//

#include "Ui/PyOutputRedirector.hpp"

namespace CivilRegistryAnalyser
{
    PyOutputRedirector::PyOutputRedirector()
    {
        auto sysm = py::module::import("sys");
        m_stdout = sysm.attr("stdout");
        m_stderr = sysm.attr("stderr");

        auto stringio = py::module::import("io").attr("StringIO");
        m_stdoutBuffer = stringio();  // Other filelike object can be used here as well, such as objects created by pybind11
        m_stderrBuffer = stringio();

        sysm.attr("stdout") = m_stdoutBuffer;
        sysm.attr("stderr") = m_stderrBuffer;

        threadHandle = new QThread(this);
        QTimer* m_timer = new QTimer();
        QObject::connect(m_timer, &QTimer::timeout, [&]()
        {
            auto outContent = StdoutContent();
            if(!outContent.empty())
            {
                emit newStdOutContent(QString::fromStdString(outContent));
            }

            auto errContent = StdErrContent();
            if(!errContent.empty())
            {
                emit newStdErrContent(QString::fromStdString(errContent));
            }
        });

        m_timer->setInterval(100);
        m_timer->setSingleShot(false);

        QObject::connect(threadHandle, &QThread::started, [m_timer](){
            m_timer->start();
        });

        m_timer->moveToThread(threadHandle);
        threadHandle->start();
    }

    std::string PyOutputRedirector::StdoutContent()
    {
        pybind11::gil_scoped_acquire acquire;

        m_stdoutBuffer.attr("seek")(0);
        auto result = py::str(m_stdoutBuffer.attr("read")());
        m_stdoutBuffer.attr("truncate")(0);
        return result;
    }

    std::string PyOutputRedirector::StdErrContent()
    {
        pybind11::gil_scoped_acquire acquire;

        m_stderrBuffer.attr("seek")(0);
        auto result = py::str(m_stderrBuffer.attr("read")());
        m_stderrBuffer.attr("truncate")(0);
        return result;
    }

    PyOutputRedirector::~PyOutputRedirector()
    {
        auto sysm = py::module::import("sys");
        sysm.attr("stdout") = m_stdout;
        sysm.attr("stderr") = m_stderr;
        threadHandle->exit(0);
    }
}