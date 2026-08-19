#pragma once

#include <memory>

#include "phi/adapter/v1/value.h"

class QObject;

namespace phicore::adapter::sdk {
class SidecarHost;
}

namespace phicore::adapter::sdk::qt {

/**
 * @brief Drives a SidecarHost from the Qt event loop.
 *
 * Watches the host's poll descriptor with a `QSocketNotifier` and calls
 * `pollOnce(0)` whenever it becomes readable. Because the host's wake
 * descriptor is part of the same poll set, this covers inbound frames as well
 * as outbound work queued from worker threads.
 *
 * This replaces the two hand-written patterns adapters used before:
 * a blocking `pollOnce(250ms)` loop that starved the Qt event loop, and a
 * short-interval `QTimer` that woke up dozens of times per second. With the
 * notifier there is no polling interval at all: idle costs nothing and
 * latency is bounded by the event loop, not by a timer tick.
 *
 * Usage:
 * @code
 * QCoreApplication app(argc, argv);
 * phi::SidecarHost host(socketPath, factory);
 * phi::qt::SidecarDriver driver(host);
 * if (!driver.start(&error))
 *     return 1;
 * return app.exec(); // driver.stop() runs in the destructor
 * @endcode
 */
class SidecarDriver
{
public:
    /**
     * @param host Host to drive; must outlive the driver.
     * @param parent Optional parent for the internal notifier object.
     */
    explicit SidecarDriver(phicore::adapter::sdk::SidecarHost &host, QObject *parent = nullptr);
    ~SidecarDriver();

    SidecarDriver(const SidecarDriver &) = delete;
    SidecarDriver &operator=(const SidecarDriver &) = delete;

    /**
     * @brief Start the host and attach it to the Qt event loop.
     * @return `true` on success; `false` leaves the host stopped.
     */
    bool start(phicore::adapter::v1::Utf8String *error = nullptr);

    /// Detach from the event loop and stop the host.
    void stop();

    /// Whether the driver is currently attached.
    bool isRunning() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace phicore::adapter::sdk::qt
