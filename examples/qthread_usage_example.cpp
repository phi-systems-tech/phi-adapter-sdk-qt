#include <chrono>
#include <atomic>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>

#include <QCoreApplication>
#include <QEventLoop>
#include <QTimer>

#include "phi/adapter/sdk/qt/instance_execution_backend_qt.h"
#include "phi/adapter/sdk/sidecar.h"

namespace {

std::atomic_bool g_running{true};
namespace phi = phicore::adapter::sdk;

void handleSignal(int)
{
    g_running.store(false);
}

std::int64_t nowMs()
{
    using clock = std::chrono::system_clock;
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               clock::now().time_since_epoch())
        .count();
}

class QThreadAdapterInstance final : public phi::AdapterInstance
{
protected:
    bool start() override
    {
        std::cerr << "[qt-demo] start extId=" << externalId() << std::endl;
        return true;
    }

    void onConnected() override
    {
        std::cerr << "[qt-demo] core connected extId=" << externalId() << std::endl;
    }

    void onDisconnected() override
    {
        std::cerr << "[qt-demo] core disconnected extId=" << externalId() << std::endl;
    }

    void onChannelInvoke(const phi::ChannelInvokeRequest &request) override
    {
        if (request.channelExternalId != "power") {
            submitCmdFailure(request.cmdId,
                             phi::CmdStatus::NotSupported,
                             "unsupported channel: " + request.channelExternalId);
            return;
        }

        if (!request.hasScalarValue) {
            submitCmdFailure(request.cmdId,
                             phi::CmdStatus::InvalidArgument,
                             "channel.invoke requires scalar value");
            return;
        }

        // Simulate asynchronous I/O or slow device call on the Qt instance thread.
        const auto cmdId = request.cmdId;
        const auto value = request.value;
        QTimer::singleShot(150, this, [this, cmdId, value = value]() mutable {
            phi::CmdResponse response;
            response.id = cmdId;
            response.status = phi::CmdStatus::Success;
            response.finalValue = std::move(value);
            response.tsMs = nowMs();

            if (!sendResult(response, &m_lastResultError))
                std::cerr << "[qt-demo] failed to send channel result: " << m_lastResultError << std::endl;
        });
    }

    void onAdapterActionInvoke(const phi::AdapterActionInvokeRequest &request) override
    {
        if (request.actionId != "ping") {
            submitActionFailure(request.cmdId,
                                phi::CmdStatus::NotSupported,
                                "unsupported action: " + request.actionId);
            return;
        }

        phi::ActionResponse response;
        response.id = request.cmdId;
        response.status = phi::CmdStatus::Success;
        response.resultType = phi::ActionResultType::String;
        response.resultValue = phi::ScalarValue("pong");
        response.tsMs = nowMs();

        if (!sendResult(response, &m_lastResultError))
            std::cerr << "[qt-demo] failed to send action result: " << m_lastResultError << std::endl;
    }

private:
    void submitCmdFailure(phi::CmdId cmdId, phi::CmdStatus status, const std::string &error)
    {
        phi::CmdResponse response;
        response.id = cmdId;
        response.status = status;
        response.error = error;
        response.tsMs = nowMs();

        if (!sendResult(response, &m_lastResultError))
            std::cerr << "[qt-demo] failed to send command error: " << m_lastResultError << std::endl;
    }

    void submitActionFailure(phi::CmdId cmdId, phi::CmdStatus status, const std::string &error)
    {
        phi::ActionResponse response;
        response.id = cmdId;
        response.status = status;
        response.error = error;
        response.tsMs = nowMs();

        if (!sendResult(response, &m_lastResultError))
            std::cerr << "[qt-demo] failed to send action error: " << m_lastResultError << std::endl;
    }

    phi::Utf8String m_lastResultError;
};

class QThreadAdapterFactory final : public phi::AdapterFactory
{
protected:
    phi::Utf8String pluginType() const override
    {
        return "qt-demo";
    }

    phi::Utf8String displayName() const override
    {
        return "Qt Demo Adapter";
    }

    phi::Utf8String description() const override
    {
        return "Small example showing Qt-backed instance execution";
    }

    phi::Utf8String apiVersion() const override
    {
        return "1.0.0";
    }

    phi::Utf8String iconSvg() const override
    {
        return "<svg width=\"32\" height=\"32\" xmlns=\"http://www.w3.org/2000/svg\"></svg>";
    }

    std::unique_ptr<phi::InstanceExecutionBackend> createInstanceExecutionBackend(
        const phi::ExternalId &externalId) override
    {
        (void)externalId;
        return phi::qt::createInstanceExecutionBackend();
    }

    std::unique_ptr<phi::AdapterInstance> createInstance(const phi::ExternalId &externalId) override
    {
        std::cerr << "[qt-demo] create instance extId=" << externalId << std::endl;
        return std::make_unique<QThreadAdapterInstance>();
    }

    int timeoutMs() const override
    {
        return 5000;
    }
};

} // namespace

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    const char *envSocketPath = std::getenv("PHI_ADAPTER_SOCKET_PATH");
    const phi::Utf8String socketPath = (argc > 1)
                                          ? argv[1]
                                          : (envSocketPath ? envSocketPath
                                                           : phi::Utf8String("/tmp/phi-adapter-demo.sock"));

    QThreadAdapterFactory factory;
    phi::SidecarHost host(socketPath, factory);

    phi::Utf8String error;
    if (!host.start(&error)) {
        std::cerr << "failed to start host: " << error << std::endl;
        return 1;
    }

    while (g_running.load()) {
        if (!host.pollOnce(std::chrono::milliseconds(250), &error)) {
            std::cerr << "host poll failed: " << error << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
    }

    host.stop();
    return 0;
}
