/**
 * System monitor utility that watches our system resources such as CPU, memory,
 * ... and shows them in a nice graphical way.
 */

#ifndef SYSMON
#define SYSMON

#include <Wt/WContainerWidget.h>

#include <memory>

namespace Wt {
  class WWidget;
}

class SysMon : public Wt::WContainerWidget
{
private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;

public:
    explicit SysMon();
    virtual ~SysMon();

public:
    void Pause();
    void Resume();

private:
    std::unique_ptr<Wt::WWidget> Layout();
};

#endif /* SYSMON */
