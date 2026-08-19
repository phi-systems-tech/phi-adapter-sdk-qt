#include "phi/adapter/sdk/qt/sidecar_driver_qt.h"

#include "phi/adapter/sdk/sidecar.h"

#include <QObject>
#include <QSocketNotifier>

#include <chrono>
#include <iostream>

namespace phicore::adapter::sdk::qt {

namespace {

// The notifier reports readiness; the actual wait already happened in the Qt
// event loop, so polling must not block.
constexpr auto kNonBlockingPoll = std::chrono::milliseconds(0);

} // namespace

struct SidecarDriver::Impl {
    Impl(phicore::adapter::sdk::SidecarHost &hostRef, QObject *parentObject)
        : host(hostRef)
        , parent(parentObject)
    {
    }

    phicore::adapter::sdk::SidecarHost &host;
    QObject *parent = nullptr;
    QSocketNotifier *notifier = nullptr;
    bool running = false;
};

SidecarDriver::SidecarDriver(phicore::adapter::sdk::SidecarHost &host, QObject *parent)
    : m_impl(std::make_unique<Impl>(host, parent))
{
}

SidecarDriver::~SidecarDriver()
{
    stop();
}

bool SidecarDriver::start(phicore::adapter::v1::Utf8String *error)
{
    if (m_impl->running)
        return true;

    if (!m_impl->host.start(error))
        return false;

    const int pollFd = m_impl->host.pollDescriptor();
    if (pollFd < 0) {
        if (error)
            *error = "sidecar host has no poll descriptor";
        m_impl->host.stop();
        return false;
    }

    m_impl->notifier = new QSocketNotifier(pollFd, QSocketNotifier::Read, m_impl->parent);
    QObject::connect(m_impl->notifier, &QSocketNotifier::activated, m_impl->notifier, [this]() {
        phicore::adapter::v1::Utf8String pollError;
        if (!m_impl->host.pollOnce(kNonBlockingPoll, &pollError)) {
            // Host/runtime failure: structured logging runs over the very path
            // that is failing here, so this stays on stderr.
            std::cerr << "[sidecar][pollFailure][host] " << pollError << std::endl;
        }
    });

    // Queued outbound work may already be pending, and the descriptor only
    // signals edges the event loop has not consumed yet.
    phicore::adapter::v1::Utf8String initialError;
    m_impl->host.pollOnce(kNonBlockingPoll, &initialError);

    m_impl->running = true;
    return true;
}

void SidecarDriver::stop()
{
    if (!m_impl->running)
        return;

    // Detach before the host closes the descriptor the notifier watches.
    if (m_impl->notifier) {
        m_impl->notifier->setEnabled(false);
        m_impl->notifier->deleteLater();
        m_impl->notifier = nullptr;
    }

    m_impl->host.stop();
    m_impl->running = false;
}

bool SidecarDriver::isRunning() const
{
    return m_impl->running;
}

} // namespace phicore::adapter::sdk::qt
