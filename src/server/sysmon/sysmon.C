/**
 * System monitor utility that watches our system resources such as CPU, memory,
 * ... and shows them in a nice graphical way.
 */

#include <Wt/Chart/WCartesianChart.h>
#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WTable.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/WTimer.h>
#include <Wt/WWidget.h>

#include <list>
#include <map>
#include <cmath>
#include <memory>
#include <unordered_map>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <statgrab.h>

#include "../common/utils.h"
#include "../common/div.h"
#include "sysmon.h"

#define MAX_INSTANTS 60

using namespace boost;
using namespace Wt;
using namespace Wt::Chart;

struct SysMon::Impl : public Wt::WObject
{
public:
    enum class Cpu : unsigned char {
        User,
        Kernel,
        Idle,
        IoWait,
        Swap,
        Nice
    };

    enum class Memory : unsigned char {
        Total,
        Free,
        Used,
        Cache
    };

    enum class Swap : unsigned char {
        Total,
        Used,
        Free
    };

    enum class VirtualMemory : unsigned char {
        Total,
        Used,
        Free
    };

    typedef std::map<Cpu, double> CpuInstant;
    typedef std::map<Memory, double> MemoryInstant;
    typedef std::map<Swap, double> SwapInstant;
    typedef std::map<VirtualMemory, double> VirtualMemoryInstant;

    typedef std::unordered_map<sg_host_state, WString,
    Utility::Hasher<sg_host_state>> HostStateHashTable;

public:
    std::unique_ptr<WTimer> Timer;

    HostStateHashTable HostState;

    std::list<CpuInstant> CpuUsageCache;
    std::shared_ptr<WStandardItemModel> CpuUsageModel;

    std::list<MemoryInstant> MemoryUsageCache;
    std::shared_ptr<WStandardItemModel> MemoryUsageModel;

    std::list<SwapInstant> SwapUsageCache;
    std::shared_ptr<WStandardItemModel> SwapUsageModel;

    std::list<VirtualMemoryInstant> VirtualMemoryUsageCache;
    std::shared_ptr<WStandardItemModel> VirtualMemoryUsageModel;

    WContainerWidget *HostInfoHorizontalDiv;
    WContainerWidget *HostInfoVerticalDiv;
    WContainerWidget *DiskInfoDiv;
    WContainerWidget *NetworkInfoDiv;

public:
    Impl();
    ~Impl();

private:
    void RefreshResourceUsage();

public:
    void Initialize();
};

SysMon::SysMon() : pimpl(std::make_unique<SysMon::Impl>())
{
    clear();
    setId("SysMonPage");
    addWidget(this->Layout());

    pimpl->Initialize();
}

SysMon::~SysMon() = default;

void SysMon::Pause()
{
    if (pimpl->Timer == nullptr)
        return;

    pimpl->Timer->stop();
}

void SysMon::Resume()
{
    if (pimpl->Timer == nullptr)
        return;

    pimpl->Timer->start();
}

std::unique_ptr<WWidget> SysMon::Layout()
{
    auto container = std::make_unique<Div>("SysMon", "container-fluid");

    /// Read the template, otherwise return
    std::string htmlData;
    if (!readFile("templates/system-monitor.wtml", htmlData)) {
        return container;
    }

    /// Fill the template
    WTemplate *tmpl = container->addNew<WTemplate>();
    tmpl->setTemplateText(WString::fromUTF8(htmlData), TextFormat::UnsafeXHTML);

    /// Host Info
    pimpl->HostInfoHorizontalDiv = tmpl->bindNew<Div>("host-info-horizontal", "HostInfoHorizontalDiv");
    pimpl->HostInfoVerticalDiv = tmpl->bindNew<Div>("host-info-vertical", "HostInfoVerticalDiv");

    /// CPU Model
    pimpl->CpuUsageModel = std::make_shared<WStandardItemModel>(60, 7);
    pimpl->CpuUsageModel->setHeaderData(0, tr("system-monitor-cpu-percents-user"));
    pimpl->CpuUsageModel->setHeaderData(1, tr("system-monitor-cpu-percents-kernel"));
    pimpl->CpuUsageModel->setHeaderData(2, tr("system-monitor-cpu-percents-idle"));
    pimpl->CpuUsageModel->setHeaderData(3, tr("system-monitor-cpu-percents-iowait"));
    pimpl->CpuUsageModel->setHeaderData(4, tr("system-monitor-cpu-percents-swap"));
    pimpl->CpuUsageModel->setHeaderData(5, tr("system-monitor-cpu-percents-nice"));
    pimpl->CpuUsageModel->setHeaderData(6, WString("-"));

    /// Memory Model
    pimpl->MemoryUsageModel = std::make_shared<WStandardItemModel>(60, 5);
    pimpl->MemoryUsageModel->setHeaderData(0, tr("system-monitor-memory-stats-total"));
    pimpl->MemoryUsageModel->setHeaderData(1, tr("system-monitor-memory-stats-free"));
    pimpl->MemoryUsageModel->setHeaderData(2, tr("system-monitor-memory-stats-used"));
    pimpl->MemoryUsageModel->setHeaderData(3, tr("system-monitor-memory-stats-cache"));
    pimpl->MemoryUsageModel->setHeaderData(4, WString("-"));

    /// Swap Model
    pimpl->SwapUsageModel = std::make_shared<WStandardItemModel>(60, 4);
    pimpl->SwapUsageModel->setHeaderData(0, tr("system-monitor-swap-stats-total"));
    pimpl->SwapUsageModel->setHeaderData(1, tr("system-monitor-swap-stats-used"));
    pimpl->SwapUsageModel->setHeaderData(2, tr("system-monitor-swap-stats-free"));
    pimpl->SwapUsageModel->setHeaderData(3, WString("-"));

    /// Virtual Memory Model
    pimpl->VirtualMemoryUsageModel = std::make_shared<WStandardItemModel>(60, 4);
    pimpl->VirtualMemoryUsageModel->setHeaderData(0, tr("system-monitor-virtual-memory-stats-total"));
    pimpl->VirtualMemoryUsageModel->setHeaderData(1, tr("system-monitor-virtual-memory-stats-used"));
    pimpl->VirtualMemoryUsageModel->setHeaderData(2, tr("system-monitor-virtual-memory-stats-free"));
    pimpl->VirtualMemoryUsageModel->setHeaderData(3, WString("-"));

    /// CPU Graph
    auto cpuUsageChart = tmpl->bindNew<WCartesianChart>("cpu-info");
    cpuUsageChart->setBackground(StandardColor::Gray);
    cpuUsageChart->setModel(pimpl->CpuUsageModel);
    cpuUsageChart->setXSeriesColumn(pimpl->CpuUsageModel->columnCount() - 1);
    cpuUsageChart->setLegendEnabled(true);
    cpuUsageChart->setLegendLocation(LegendLocation::Outside, Side::Right, AlignmentFlag::Middle);

    cpuUsageChart->setType(Chart::ChartType::Scatter);
    cpuUsageChart->axis(Axis::X).setScale(AxisScale::Linear);
    cpuUsageChart->axis(Axis::Y).setScale(AxisScale::Linear);

    cpuUsageChart->setPlotAreaPadding(120, Side::Right);
    cpuUsageChart->setPlotAreaPadding(30, Side::Top | Side::Bottom);

    for (int i = 0; i < pimpl->CpuUsageModel->columnCount() - 1; ++i) {
      auto s = std::make_unique<WDataSeries>(i, SeriesType::Curve);
      s->setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
      cpuUsageChart->addSeries(std::move(s));
    }

    cpuUsageChart->resize(600, 330);

    cpuUsageChart->setMargin(10, Side::Top | Side::Bottom);
    cpuUsageChart->setMargin(WLength::Auto, Side::Left | Side::Right);

    /// Memory Graph

    auto memoryUsageChart = tmpl->bindNew<WCartesianChart>("memory-info");
    memoryUsageChart->setBackground(StandardColor::Gray);
    memoryUsageChart->setModel(pimpl->MemoryUsageModel);
    memoryUsageChart->setXSeriesColumn(pimpl->MemoryUsageModel->columnCount() - 1);
    memoryUsageChart->setLegendEnabled(true);
    memoryUsageChart->setLegendLocation(LegendLocation::Outside, Side::Right, AlignmentFlag::Middle);

    memoryUsageChart->setType(Chart::ChartType::Scatter);
    memoryUsageChart->axis(Axis::X).setScale(AxisScale::Linear);
    memoryUsageChart->axis(Axis::Y).setScale(AxisScale::Linear);

    memoryUsageChart->setPlotAreaPadding(220, Side::Right);
    memoryUsageChart->setPlotAreaPadding(30, Side::Top | Side::Bottom);

    for (int i = 0; i < pimpl->MemoryUsageModel->columnCount() - 1; ++i) {
      auto s = std::make_unique<WDataSeries>(i, SeriesType::Curve);
      s->setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
      memoryUsageChart->addSeries(std::move(s));
    }

    memoryUsageChart->resize(600, 330);

    memoryUsageChart->setMargin(10, Side::Top | Side::Bottom);
    memoryUsageChart->setMargin(WLength::Auto, Side::Left | Side::Right);

    /// Swap Graph
    auto swapUsageChart = tmpl->bindNew<WCartesianChart>("swap-info");
    swapUsageChart->setBackground(StandardColor::Gray);
    swapUsageChart->setModel(pimpl->SwapUsageModel);
    swapUsageChart->setXSeriesColumn(pimpl->SwapUsageModel->columnCount() - 1);
    swapUsageChart->setLegendEnabled(true);
    swapUsageChart->setLegendLocation(LegendLocation::Outside, Side::Right, AlignmentFlag::Middle);

    swapUsageChart->setType(Chart::ChartType::Scatter);
    swapUsageChart->axis(Axis::X).setScale(AxisScale::Linear);
    swapUsageChart->axis(Axis::Y).setScale(AxisScale::Linear);

    swapUsageChart->setPlotAreaPadding(220, Side::Right);
    swapUsageChart->setPlotAreaPadding(30, Side::Top | Side::Bottom);

    for (int i = 0; i < pimpl->SwapUsageModel->columnCount() - 1; ++i) {
      auto s = std::make_unique<WDataSeries>(i, SeriesType::Curve);
      s->setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
      swapUsageChart->addSeries(std::move(s));
    }

    swapUsageChart->resize(600, 330);

    swapUsageChart->setMargin(10, Side::Top | Side::Bottom);
    swapUsageChart->setMargin(WLength::Auto, Side::Left | Side::Right);

    /// Virtual Memory Graph
    auto virtualMemoryUsageChart = tmpl->bindNew<WCartesianChart>("virtual-memory-info");

    virtualMemoryUsageChart->setBackground(StandardColor::Gray);
    virtualMemoryUsageChart->setModel(pimpl->VirtualMemoryUsageModel);
    virtualMemoryUsageChart->setXSeriesColumn(pimpl->VirtualMemoryUsageModel->columnCount() - 1);
    virtualMemoryUsageChart->setLegendEnabled(true);
    virtualMemoryUsageChart->setLegendLocation(LegendLocation::Outside, Side::Right, AlignmentFlag::Middle);

    virtualMemoryUsageChart->setType(Chart::ChartType::Scatter);
    virtualMemoryUsageChart->axis(Axis::X).setScale(AxisScale::Linear);
    virtualMemoryUsageChart->axis(Axis::Y).setScale(AxisScale::Linear);

    virtualMemoryUsageChart->setPlotAreaPadding(220, Side::Right);
    virtualMemoryUsageChart->setPlotAreaPadding(30, Side::Top | Side::Bottom);

    for (int i = 0; i < pimpl->VirtualMemoryUsageModel->columnCount() - 1; ++i) {
      auto s = std::make_unique<WDataSeries>(i, SeriesType::Curve);
      s->setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
      virtualMemoryUsageChart->addSeries(std::move(s));
    }

    virtualMemoryUsageChart->resize(600, 330);

    virtualMemoryUsageChart->setMargin(10, Side::Top | Side::Bottom);
    virtualMemoryUsageChart->setMargin(WLength::Auto, Side::Left | Side::Right);

    /// Disk Info
    pimpl->DiskInfoDiv = tmpl->bindNew<Div>("disk-info", "DiskInfoDiv");

    /// Net Info
    pimpl->NetworkInfoDiv = tmpl->bindNew<Div>("network-info", "NetworkInfoDiv");

    tmpl->bindWidget("host-info-title",
                     std::make_unique<WText>(WString("<h4>{1}</h4>").arg(tr("system-monitor-host-info"))));
    tmpl->bindWidget("cpu-info-title",
                     std::make_unique<WText>(WString("<h4>{1}</h4>").arg(tr("system-monitor-cpu-percents"))));
    tmpl->bindWidget("memory-info-title",
                     std::make_unique<WText>(WString("<h4>{1}</h4>").arg(tr("system-monitor-memory-stats"))));
    tmpl->bindWidget("swap-info-title",
                     std::make_unique<WText>(WString("<h4>{1}</h4>").arg(tr("system-monitor-swap-stats"))));
    tmpl->bindWidget("virtual-memory-info-title",
                     std::make_unique<WText>(WString("<h4>{1}</h4>").arg(tr("system-monitor-virtual-memory-stats"))));
    tmpl->bindWidget("disk-info-title",
                     std::make_unique<WText>(WString("<h4>{1}</h4>").arg(tr("system-monitor-disk-io-stats"))));
    tmpl->bindWidget("network-info-title",
                     std::make_unique<WText>(WString("<h4>{1}</h4>").arg(tr("system-monitor-network-io-stats"))));

    return container;
}

SysMon::Impl::Impl() : Timer(nullptr), HostState {
  {sg_unknown_configuration, tr("system-monitor-host-info-host-state-unknown-configuration")},
  {sg_physical_host, tr("system-monitor-host-info-host-state-physical-host")},
  {sg_virtual_machine, tr("system-monitor-host-info-host-state-virtual-machine")},
  {sg_paravirtual_machine, tr("system-monitor-host-info-host-state-paravirtual-machine")},
  {sg_hardware_virtualized, tr("system-monitor-host-info-host-state-hardware-virtualized")}}
{

}

SysMon::Impl::~Impl() = default;

void SysMon::Impl::RefreshResourceUsage()
{
    sg_cpu_percents *cpuPercents;
    sg_mem_stats *memStats;
    sg_swap_stats *swapStats;
    sg_host_info *hostInfo;
    sg_disk_io_stats *diskIoStats;
    sg_network_io_stats *networkIoStats;

    size_t cpuPercentsEntries = 0;
    size_t memStatsEntries = 0;
    size_t swapStatsEntries = 0;
    size_t hostInfoEntries = 0;
    size_t diskIoStatsEntries = 0;
    size_t networkIoStatsEntries = 0;

    /// Get the host info
    if ((hostInfo = sg_get_host_info(&hostInfoEntries)) != nullptr) {
        HostInfoHorizontalDiv->clear();
        HostInfoVerticalDiv->clear();

        auto horizontalHostInfoTable = HostInfoHorizontalDiv->addNew<WTable>();
        horizontalHostInfoTable->setStyleClass("table table-hover");
        horizontalHostInfoTable->setHeaderCount(1, Orientation::Horizontal);

        auto verticalHostInfoTable = HostInfoVerticalDiv->addNew<WTable>();
        verticalHostInfoTable->setStyleClass("table table-hover");
        verticalHostInfoTable->setHeaderCount(1, Orientation::Vertical);

        horizontalHostInfoTable->elementAt(0, 0)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-os-name")));
        horizontalHostInfoTable->elementAt(0, 1)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-os-release")));
        horizontalHostInfoTable->elementAt(0, 2)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-os-version")));
        horizontalHostInfoTable->elementAt(0, 3)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-platform")));
        horizontalHostInfoTable->elementAt(0, 4)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-hostname")));
        horizontalHostInfoTable->elementAt(0, 5)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-bitwidth")));
        horizontalHostInfoTable->elementAt(0, 6)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-host-state")));
        horizontalHostInfoTable->elementAt(0, 7)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-ncpus")));
        horizontalHostInfoTable->elementAt(0, 8)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-maxcpus")));
        horizontalHostInfoTable->elementAt(0, 9)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-uptime")));
        horizontalHostInfoTable->elementAt(0, 10)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-systime")));

        verticalHostInfoTable->elementAt(0, 0)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-os-name")));
        verticalHostInfoTable->elementAt(1, 0)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-os-release")));
        verticalHostInfoTable->elementAt(2, 0)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-os-version")));
        verticalHostInfoTable->elementAt(3, 0)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-platform")));
        verticalHostInfoTable->elementAt(4, 0)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-hostname")));
        verticalHostInfoTable->elementAt(5, 0)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-bitwidth")));
        verticalHostInfoTable->elementAt(6, 0)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-host-state")));
        verticalHostInfoTable->elementAt(7, 0)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-ncpus")));
        verticalHostInfoTable->elementAt(8, 0)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-maxcpus")));
        verticalHostInfoTable->elementAt(9, 0)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-uptime")));
        verticalHostInfoTable->elementAt(10, 0)->addWidget(std::make_unique<WText>(tr("system-monitor-host-info-systime")));

        std::string systime(dateTimeString(hostInfo->systime));

        horizontalHostInfoTable->elementAt(1, 0)->addWidget(std::make_unique<WText>(WString(hostInfo->os_name)));
        horizontalHostInfoTable->elementAt(1, 1)->addWidget(std::make_unique<WText>(WString(hostInfo->os_release)));
        horizontalHostInfoTable->elementAt(1, 2)->addWidget(std::make_unique<WText>(WString(hostInfo->os_version)));
        horizontalHostInfoTable->elementAt(1, 3)->addWidget(std::make_unique<WText>(WString(hostInfo->platform)));
        horizontalHostInfoTable->elementAt(1, 4)->addWidget(std::make_unique<WText>(WString(hostInfo->hostname)));
        horizontalHostInfoTable->elementAt(1, 5)->addWidget(
                    std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(hostInfo->bitwidth))));
        horizontalHostInfoTable->elementAt(1, 6)->addWidget(
                    std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(HostState[hostInfo->host_state]))));
        horizontalHostInfoTable->elementAt(1, 7)->addWidget(
                    std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(hostInfo->ncpus))));
        horizontalHostInfoTable->elementAt(1, 8)->addWidget(
                    std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(hostInfo->maxcpus))));
        horizontalHostInfoTable->elementAt(1, 9)->addWidget(
                    std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(secondsToHumanReadableTime(hostInfo->uptime)))));
        horizontalHostInfoTable->elementAt(1, 10)->addWidget(
                    std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(systime))));

        verticalHostInfoTable->elementAt(0, 1)->addWidget(std::make_unique<WText>(WString(hostInfo->os_name)));
        verticalHostInfoTable->elementAt(1, 1)->addWidget(std::make_unique<WText>(WString(hostInfo->os_release)));
        verticalHostInfoTable->elementAt(2, 1)->addWidget(std::make_unique<WText>(WString(hostInfo->os_version)));
        verticalHostInfoTable->elementAt(3, 1)->addWidget(std::make_unique<WText>(WString(hostInfo->platform)));
        verticalHostInfoTable->elementAt(4, 1)->addWidget(std::make_unique<WText>(WString(hostInfo->hostname)));
        verticalHostInfoTable->elementAt(5, 1)->addWidget(
                    std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(hostInfo->bitwidth))));
        verticalHostInfoTable->elementAt(6, 1)->addWidget(
                    std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(HostState[hostInfo->host_state]))));
        verticalHostInfoTable->elementAt(7, 1)->addWidget(
                    std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(hostInfo->ncpus))));
        verticalHostInfoTable->elementAt(8, 1)->addWidget(
                    std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(hostInfo->maxcpus))));
        verticalHostInfoTable->elementAt(9, 1)->addWidget
                (std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(secondsToHumanReadableTime(hostInfo->uptime)))));
        verticalHostInfoTable->elementAt(10, 1)->addWidget(
                    std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(systime))));
    }

    /// Shift and fill the CPU usage cache and model
    if ((cpuPercents = sg_get_cpu_percents(&cpuPercentsEntries)) != nullptr) {
        CpuInstant cpuInstant;
        cpuInstant[Cpu::User] = cpuPercents->user;
        cpuInstant[Cpu::Kernel] = cpuPercents->kernel;
        cpuInstant[Cpu::Idle] = cpuPercents->idle;
        cpuInstant[Cpu::IoWait] = cpuPercents->iowait;
        cpuInstant[Cpu::Swap] = cpuPercents->swap;
        cpuInstant[Cpu::Nice] = cpuPercents->nice;

        CpuUsageCache.push_back(cpuInstant);
        CpuUsageCache.pop_front();

        int i = 0;
        for (auto it = CpuUsageCache.begin(); it != CpuUsageCache.end(); ++it) {
          cpuInstant = *it;
          CpuUsageModel->setData(i, 0, cpuInstant[Cpu::User]);
          CpuUsageModel->setData(i, 1, cpuInstant[Cpu::Kernel]);
          CpuUsageModel->setData(i, 2, cpuInstant[Cpu::Idle]);
          CpuUsageModel->setData(i, 3, cpuInstant[Cpu::IoWait]);
          CpuUsageModel->setData(i, 4, cpuInstant[Cpu::Swap]);
          CpuUsageModel->setData(i, 5, cpuInstant[Cpu::Nice]);
          CpuUsageModel->setData(i, 6, i);
          ++i;
        }
    }

    /// Shift and fill the memory usage cache and model
    if ((memStats = sg_get_mem_stats(&memStatsEntries)) != nullptr) {
        MemoryInstant memoryInstant;
        memoryInstant[Memory::Total] = 100.0;
        memoryInstant[Memory::Free] = 100.0 * memStats->free / memStats->total;
        memoryInstant[Memory::Used] = 100.0 * memStats->used / memStats->total;
        memoryInstant[Memory::Cache] = 100.0 * memStats->cache / memStats->total;

        MemoryUsageCache.push_back(memoryInstant);
        MemoryUsageCache.pop_front();

        MemoryUsageModel->setHeaderData(
                    0, WString::fromUTF8("{1} {2}")
                    .arg(calculateSize(memStats->total))
                    .arg(tr("system-monitor-memory-stats-total")));
        MemoryUsageModel->setHeaderData(
                    1, WString::fromUTF8("{1} ({2}%) {3}")
                    .arg(calculateSize(memStats->free))
                    .arg((wformat(L"%.1f") % memoryInstant[Memory::Free]).str())
                .arg(tr("system-monitor-memory-stats-free"))
                );
        MemoryUsageModel->setHeaderData(
                    2, WString::fromUTF8("{1} ({2}%) {3}")
                    .arg((calculateSize(memStats->used)))
                    .arg((wformat(L"%.1f") % memoryInstant[Memory::Used]).str())
                .arg(tr("system-monitor-memory-stats-used"))
                );
        MemoryUsageModel->setHeaderData(
                    3, WString::fromUTF8("{1} ({2}%) {3}")
                    .arg(calculateSize(memStats->cache))
                    .arg((wformat(L"%.1f") % memoryInstant[Memory::Cache]).str())
                .arg(tr("system-monitor-memory-stats-cache"))
                );

        int i = 0;
        for (auto it = MemoryUsageCache.begin(); it != MemoryUsageCache.end(); ++it) {
            memoryInstant = *it;
            MemoryUsageModel->setData(i, 0, memoryInstant[Memory::Total]);
            MemoryUsageModel->setData(i, 1, memoryInstant[Memory::Free]);
            MemoryUsageModel->setData(i, 2, memoryInstant[Memory::Used]);
            MemoryUsageModel->setData(i, 3, memoryInstant[Memory::Cache]);
            MemoryUsageModel->setData(i, 4, i);
            ++i;
        }
    }

    /// Shift and fill the swap usage cache and model
    if ((swapStats = sg_get_swap_stats(&swapStatsEntries)) != nullptr) {
        SwapInstant swapInstant;
        swapInstant[Swap::Total] = 100.0;
        swapInstant[Swap::Used] = 100.0 * swapStats->used / swapStats->total;
        swapInstant[Swap::Free] = 100.0 * swapStats->free / swapStats->total;

        SwapUsageCache.push_back(swapInstant);
        SwapUsageCache.pop_front();

        SwapUsageModel->setHeaderData(
                    0, WString::fromUTF8("{1} {2}")
                    .arg(calculateSize(swapStats->total))
                    .arg(tr("system-monitor-swap-stats-total")));
        SwapUsageModel->setHeaderData(
                    1, WString::fromUTF8("{1} ({2}%) {3}")
                    .arg((calculateSize(swapStats->used)))
                    .arg((wformat(L"%.1f") % (!std::isnan(swapInstant[Swap::Used]) ? swapInstant[Swap::Used] : 0.0)).str())
                .arg(tr("system-monitor-swap-stats-used"))
                );
        SwapUsageModel->setHeaderData(
                    2, WString::fromUTF8("{1} ({2}%) {3}")
                    .arg(calculateSize(swapStats->free))
                    .arg((wformat(L"%.1f") % (!std::isnan(swapInstant[Swap::Free]) ? swapInstant[Swap::Free] : 0.0)).str())
                .arg(tr("system-monitor-swap-stats-free"))
                );

        int i = 0;
        for (auto it = SwapUsageCache.begin(); it != SwapUsageCache.end(); ++it) {
            swapInstant = *it;
            SwapUsageModel->setData(i, 0, swapInstant[Swap::Total]);
            SwapUsageModel->setData(i, 1, swapInstant[Swap::Used]);
            SwapUsageModel->setData(i, 2, swapInstant[Swap::Free]);
            SwapUsageModel->setData(i, 3, i);
            ++i;
        }
    }

    /// Shift and fill the virtual memory usage cache and model
    if (memStatsEntries > 0 && swapStatsEntries > 0) {
        unsigned long long total;
        unsigned long long used;
        unsigned long long free;

        if (!std::isnan(100.0 * swapStats->total / swapStats->total))
            total = memStats->total + swapStats->total;
        else
            total = memStats->total;

        if (!std::isnan(100.0 * swapStats->used / swapStats->total))
            used = memStats->used + swapStats->used;
        else
            used = memStats->used;

        if (!std::isnan(100.0 * swapStats->free / swapStats->total))
            free = memStats->free + swapStats->free;
        else
            free = memStats->free;

        VirtualMemoryInstant virtualMemoryInstant;
        virtualMemoryInstant[VirtualMemory::Total] = 100.0;
        virtualMemoryInstant[VirtualMemory::Used] = 100.0 * used / total;
        virtualMemoryInstant[VirtualMemory::Free] = 100.0 * free / total;

        VirtualMemoryUsageCache.push_back(virtualMemoryInstant);
        VirtualMemoryUsageCache.pop_front();

        VirtualMemoryUsageModel->setHeaderData(
                    0, WString::fromUTF8("{1} {2}")
                    .arg(calculateSize(total))
                    .arg(tr("system-monitor-virtual-memory-stats-total")));
        VirtualMemoryUsageModel->setHeaderData(
                    1, WString::fromUTF8("{1} ({2}%) {3}")
                    .arg((calculateSize(used)))
                    .arg((wformat(L"%.1f") % virtualMemoryInstant[VirtualMemory::Used]).str())
                .arg(tr("system-monitor-virtual-memory-stats-used"))
                );
        VirtualMemoryUsageModel->setHeaderData(
                    2, WString::fromUTF8("{1} ({2}%) {3}")
                    .arg(calculateSize(free))
                    .arg((wformat(L"%.1f") % virtualMemoryInstant[VirtualMemory::Free]).str())
                .arg(tr("system-monitor-virtual-memory-stats-free"))
                );

        int i = 0;
        for (auto it = VirtualMemoryUsageCache.begin(); it != VirtualMemoryUsageCache.end(); ++it) {
            virtualMemoryInstant = *it;
            VirtualMemoryUsageModel->setData(i, 0, virtualMemoryInstant[VirtualMemory::Total]);
            VirtualMemoryUsageModel->setData(i, 1, virtualMemoryInstant[VirtualMemory::Used]);
            VirtualMemoryUsageModel->setData(i, 2, virtualMemoryInstant[VirtualMemory::Free]);
            VirtualMemoryUsageModel->setData(i, 3, i);
            ++i;
        }
    }

    /// Get the disk info
    DiskInfoDiv->clear();

    size_t diskTotalRead = 0;
    size_t diskTotalWrite = 0;

    WTable *diskTable = DiskInfoDiv->addNew<WTable>();
    diskTable->setStyleClass("table table-hover");
    diskTable->setHeaderCount(1, Orientation::Horizontal);

    diskTable->elementAt(0, 0)->addWidget(std::make_unique<WText>(tr("system-monitor-disk-io-stats-disk-name")));
    diskTable->elementAt(0, 1)->addWidget(std::make_unique<WText>(tr("system-monitor-disk-io-stats-read-bytes")));
    diskTable->elementAt(0, 2)->addWidget(std::make_unique<WText>(tr("system-monitor-disk-io-stats-write-bytes")));
    diskTable->elementAt(0, 3)->addWidget(std::make_unique<WText>(tr("system-monitor-disk-io-stats-systime")));

    if ((diskIoStats = sg_get_disk_io_stats_diff(&diskIoStatsEntries)) != nullptr) {
        for (size_t i = 0; i < diskIoStatsEntries; ++i) {
            diskTable->elementAt(static_cast<int>(i) + 1, 0)->addWidget(
                        std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(diskIoStats->disk_name))));
            diskTable->elementAt(static_cast<int>(i) + 1, 1)->addWidget(
                        std::make_unique<WText>(WString(L"{1}").arg(calculateSize(diskIoStats->read_bytes))));
            diskTable->elementAt(static_cast<int>(i) + 1, 2)->addWidget(
                        std::make_unique<WText>(WString(L"{1}").arg(calculateSize(diskIoStats->write_bytes))));
            diskTable->elementAt(static_cast<int>(i) + 1, 3)->addWidget(
                        std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(static_cast<long>(diskIoStats->systime)))));

            diskTotalRead += diskIoStats->read_bytes;
            diskTotalWrite += diskIoStats->write_bytes;

            ++diskIoStats;
        }

        diskTable->elementAt(static_cast<int>(diskIoStatsEntries) + 1, 0)->addWidget(
                    std::make_unique<WText>(tr("system-monitor-disk-io-stats-total")));
        diskTable->elementAt(static_cast<int>(diskIoStatsEntries) + 1, 1)->addWidget(
                    std::make_unique<WText>(WString(L"{1}").arg(calculateSize(diskTotalRead))));
        diskTable->elementAt(static_cast<int>(diskIoStatsEntries) + 1, 2)->addWidget(
                    std::make_unique<WText>(WString(L"{1}").arg(calculateSize(diskTotalWrite))));
        diskTable->elementAt(static_cast<int>(diskIoStatsEntries) + 1, 3)->addWidget(std::make_unique<WText>(L"-"));
    }

    /// Get the netowrk info
    NetworkInfoDiv->clear();

    size_t networkTotalRx = 0;
    size_t networkTotalTx = 0;
    size_t networkTotalPacketsIn = 0;
    size_t networkTotalPacketsOut = 0;
    size_t networkTotalErrorsIn = 0;
    size_t networkTotalErrorsOut = 0;
    size_t networkTotalCollisions = 0;

    WTable *networkTable = NetworkInfoDiv->addNew<WTable>();
    networkTable->setStyleClass("table table-hover");
    networkTable->setHeaderCount(1, Orientation::Horizontal);

    networkTable->elementAt(0, 0)->addWidget(std::make_unique<WText>(tr("system-monitor-network-io-stats-interface_name")));
    networkTable->elementAt(0, 1)->addWidget(std::make_unique<WText>(tr("system-monitor-network-io-stats-tx")));
    networkTable->elementAt(0, 2)->addWidget(std::make_unique<WText>(tr("system-monitor-network-io-stats-rx")));
    networkTable->elementAt(0, 3)->addWidget(std::make_unique<WText>(tr("system-monitor-network-io-stats-ipackets")));
    networkTable->elementAt(0, 4)->addWidget(std::make_unique<WText>(tr("system-monitor-network-io-stats-opackets")));
    networkTable->elementAt(0, 5)->addWidget(std::make_unique<WText>(tr("system-monitor-network-io-stats-ierrors")));
    networkTable->elementAt(0, 6)->addWidget(std::make_unique<WText>(tr("system-monitor-network-io-stats-oerrors")));
    networkTable->elementAt(0, 7)->addWidget(std::make_unique<WText>(tr("system-monitor-network-io-stats-collisions")));
    networkTable->elementAt(0, 8)->addWidget(std::make_unique<WText>(tr("system-monitor-network-io-stats-systime")));

    if ((networkIoStats = sg_get_network_io_stats_diff(&networkIoStatsEntries)) != nullptr) {
        for (size_t i = 0; i < networkIoStatsEntries; ++i) {
            networkTable->elementAt(static_cast<int>(i) + 1, 0)->addWidget(
                        std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(networkIoStats->interface_name))));
            networkTable->elementAt(static_cast<int>(i) + 1, 1)->addWidget(
                        std::make_unique<WText>(WString(L"{1}").arg(calculateSize(networkIoStats->tx))));
            networkTable->elementAt(static_cast<int>(i) + 1, 2)->addWidget(
                        std::make_unique<WText>(WString(L"{1}").arg(calculateSize(networkIoStats->rx))));
            networkTable->elementAt(static_cast<int>(i) + 1, 3)->addWidget(
                        std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(networkIoStats->ipackets))));
            networkTable->elementAt(static_cast<int>(i) + 1, 4)->addWidget(
                        std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(networkIoStats->opackets))));
            networkTable->elementAt(static_cast<int>(i) + 1, 5)->addWidget(
                        std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(networkIoStats->ierrors))));
            networkTable->elementAt(static_cast<int>(i) + 1, 6)->addWidget(
                        std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(networkIoStats->oerrors))));
            networkTable->elementAt(static_cast<int>(i) + 1, 7)->addWidget(
                        std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(networkIoStats->collisions))));
            networkTable->elementAt(static_cast<int>(i) + 1, 8)->addWidget(
                        std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(static_cast<long>(networkIoStats->systime)))));

            networkTotalTx += networkIoStats->tx;
            networkTotalRx += networkIoStats->rx;
            networkTotalPacketsIn += networkIoStats->ipackets;
            networkTotalPacketsOut += networkIoStats->opackets;
            networkTotalErrorsIn += networkIoStats->ierrors;
            networkTotalErrorsOut += networkIoStats->oerrors;
            networkTotalCollisions += networkIoStats->collisions;

            NetworkInfoDiv->addNew<WBreak>();

            ++networkIoStats;
        }

        networkTable->elementAt(static_cast<int>(networkIoStatsEntries) + 1, 0)->addWidget(
                    std::make_unique<WText>(tr("system-monitor-network-io-stats-total")));
        networkTable->elementAt(static_cast<int>(networkIoStatsEntries) + 1, 1)->addWidget(
                    std::make_unique<WText>(WString(L"{1}").arg(calculateSize(networkTotalTx))));
        networkTable->elementAt(static_cast<int>(networkIoStatsEntries) + 1, 2)->addWidget(
                    std::make_unique<WText>(WString(L"{1}").arg(calculateSize(networkTotalRx))));
        networkTable->elementAt(static_cast<int>(networkIoStatsEntries) + 1, 3)->addWidget(
                    std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(networkTotalPacketsIn))));
        networkTable->elementAt(static_cast<int>(networkIoStatsEntries) + 1, 4)->addWidget(
                    std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(networkTotalPacketsOut))));
        networkTable->elementAt(static_cast<int>(networkIoStatsEntries) + 1, 5)->addWidget(
                    std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(networkTotalErrorsIn))));
        networkTable->elementAt(static_cast<int>(networkIoStatsEntries) + 1, 6)->addWidget(
                    std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(networkTotalErrorsOut))));
        networkTable->elementAt(static_cast<int>(networkIoStatsEntries) + 1, 7)->addWidget(
                    std::make_unique<WText>(WString::fromUTF8(lexical_cast<std::string>(networkTotalCollisions))));
        networkTable->elementAt(static_cast<int>(networkIoStatsEntries) + 1, 8)->addWidget(std::make_unique<WText>(L"-"));

        NetworkInfoDiv->addNew<WBreak>();
    }
}

void SysMon::Impl::Initialize()
{
    /// Fill the cpu model
    CpuInstant cpuInstant;
    cpuInstant[Cpu::User] = 0.0;
    cpuInstant[Cpu::Kernel] = 0.0;
    cpuInstant[Cpu::Idle] = 0.0;
    cpuInstant[Cpu::IoWait] = 0.0;
    cpuInstant[Cpu::Swap] = 0.0;
    cpuInstant[Cpu::Nice] = 0.0;

    for (size_t i = 0; i < MAX_INSTANTS; ++i)
        CpuUsageCache.push_back(cpuInstant);

    /// Fill the memory model
    MemoryInstant memoryInstant;
    memoryInstant[Memory::Total] = 0.0;
    memoryInstant[Memory::Free] = 0.0;
    memoryInstant[Memory::Used] = 0.0;
    memoryInstant[Memory::Cache] = 0.0;

    for (size_t i = 0; i < MAX_INSTANTS; ++i)
        MemoryUsageCache.push_back(memoryInstant);

    /// Fill the swap model
    SwapInstant swapInstant;
    swapInstant[Swap::Total] = 0.0;
    swapInstant[Swap::Used] = 0.0;
    swapInstant[Swap::Free] = 0.0;

    for (size_t i = 0; i < MAX_INSTANTS; ++i)
        SwapUsageCache.push_back(swapInstant);

    /// Fill the virtual memory model
    VirtualMemoryInstant virtualMemoryInstant;
    virtualMemoryInstant[VirtualMemory::Total] = 0.0;
    virtualMemoryInstant[VirtualMemory::Used] = 0.0;
    virtualMemoryInstant[VirtualMemory::Free] = 0.0;

    for (size_t i = 0; i < MAX_INSTANTS; ++i)
        VirtualMemoryUsageCache.push_back(virtualMemoryInstant);

    /// Initialize and start the timer
    Timer = std::make_unique<WTimer>();
    Timer->setInterval(std::chrono::milliseconds(1000));
    Timer->timeout().connect(this, &SysMon::Impl::RefreshResourceUsage);
}
