/**
 * System monitor utility that watches our system resources such as CPU, memory,
 * ... and shows them in a nice graphical way.
 */

#ifndef SYSMON
#define SYSMON

#include <Wt/WWidget.h>

using namespace Wt;

#include <memory>

namespace Wt {
  class WWidget;
}

class SysMon {
private:
  struct Impl;
  std::unique_ptr<Impl> pimpl;

public:
  std::unique_ptr<WWidget> layout();

  void pause();
  void resume();

  explicit SysMon();
  virtual ~SysMon();
};

#endif /* SYSMON */
