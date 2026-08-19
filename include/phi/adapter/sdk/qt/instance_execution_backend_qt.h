#pragma once

#include <memory>

#include "phi/adapter/sdk/sidecar.h"

namespace phicore::adapter::sdk::qt {

/**
 * @brief Create a Qt event-loop based instance execution backend.
 *
 * Returned backend runs adapter instance callbacks on a dedicated `QThread`
 * and enqueues work with queued Qt invocations.
 */
std::unique_ptr<phicore::adapter::sdk::InstanceExecutionBackend> createInstanceExecutionBackend();

/**
 * @brief Create a Qt event-loop based factory execution backend.
 *
 * Same implementation as `createInstanceExecutionBackend()`; the separate name
 * exists so `AdapterFactory::createFactoryExecutionBackend()` overrides read
 * unambiguously. Return it from that hook to move blocking factory work (device
 * probes) off the host poll thread; objects with thread affinity must then be
 * created lazily inside a factory hook, not in the factory constructor.
 */
std::unique_ptr<phicore::adapter::sdk::InstanceExecutionBackend> createFactoryExecutionBackend();

} // namespace phicore::adapter::sdk::qt
