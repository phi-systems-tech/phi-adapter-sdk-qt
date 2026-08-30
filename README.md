# phi-adapter-sdk-qt

Qt helper package for `phi-adapter-sdk`.

## Contents

- `phi::adapter-sdk-qt` shared library helper for Qt event-loop based adapter instance execution.
- `phi::adapter-contract-qt` header-only Qt-facing wrapper over the canonical v1 adapter
  contract (`phi/adapter/qt/*.h`). This is the **single shared copy** consumed by `phi-core`
  and Qt-based adapters; per-repo copies of these headers are not allowed. Contract semantics
  stay pinned to `phi::adapter-contract` (`phi/adapter/v1/*`) via `static_assert` guards.
- `phi::qt::SidecarDriver` (`phi/adapter/sdk/qt/sidecar_driver_qt.h`): drives a
  `SidecarHost` from the Qt event loop by watching its poll descriptor with a
  `QSocketNotifier`. No polling interval, no idle wakeups, and the Qt event loop
  is never blocked by a sidecar poll.
- Optional runtime dependency on Qt6 Core.
- CMake project and Debian packaging for `phi-adapter-sdk-qt` / `phi-adapter-sdk-qt-dev`.

## Debian Packaging Layout

- `phi-adapter-sdk-qt` stays `Architecture: any` because it ships
  `libphi_adapter_sdk_qt.so`
- `phi-adapter-sdk-qt-dev` can be `Architecture: all` only if it contains only
  headers and arch-neutral CMake metadata

Recommended install layout:

- runtime library: `usr/lib/<multiarch-triplet>/libphi_adapter_sdk_qt.so*`
- headers: `usr/include/phi/adapter/sdk/qt/...`
- CMake package config for the `-dev` package:
  `usr/lib/cmake/phi-adapter-sdk-qt`

For this project, prefer `usr/lib/cmake/...` over `usr/share/...` for CMake
package config files. It is the less surprising Linux/CMake layout and works
well with `find_package(... CONFIG)`.

## Purpose

Provide a small helper backend so adapter instances can run their SDK callbacks inside a Qt
event loop (`QThread` + queued execution) instead of the default worker thread.

## Usage

- Include `phi/adapter/sdk/qt/instance_execution_backend_qt.h`.
- In your factory override `createInstanceExecutionBackend(...)` and return `phicore::adapter::sdk::qt::createInstanceExecutionBackend()`.
- Use a `QCoreApplication` in `main()`, because the backend relies on Qt internals and
  command/event processing must coexist with core polling.

```cpp
#include "phi/adapter/sdk/qt/instance_execution_backend_qt.h"

std::unique_ptr<phicore::adapter::sdk::InstanceExecutionBackend>
MyFactory::createInstanceExecutionBackend(const phicore::adapter::v1::ExternalId &)
{
    return phicore::adapter::sdk::qt::createInstanceExecutionBackend();
}
```

## What matters for correct results

- Every `Cmd*` request requires exactly one correlated `sendResult(CmdResponse, ...)`.
- Every `Action*` request requires exactly one correlated `sendResult(ActionResponse, ...)`.
- Use `response.id = request.cmdId` and set `response.tsMs` before sending.
- Fill `status` explicitly (`Success`, `InvalidArgument`, `NotSupported`, etc.).
- For failures, set `response.error` (and optional error params/context when useful).
- Do **not** write result frames directly; completion must always go through `sendResult(...)`.
- Return from handlers after scheduling an async task is fine as long as you send once later.
- Avoid long/blocking work in handler methods (`onChannelInvoke`, `onAdapterActionInvoke`, ...).
  Offload with Qt timers/queues, workers, or async I/O.
- Keep one outstanding callback per request ID; never send multiple responses for same `cmdId`.
- `sendResult(...)` returns `false` on enqueue error; log/report that error.

## Threading and lifecycle rules

- The Qt backend runs adapter instance callbacks on one dedicated `QThread`.
- Create long-lived `QObject` helpers from `start()` or another backend callback so their affinity
  is the backend thread.
- Parent `QTimer`, `QTcpSocket`, reconnect timers and protocol/session managers to a backend-owned
  runtime object.
- Stop timers, sockets and reconnect loops in `stop()` before the runtime object is destroyed.
- Do not move backend-owned `QObject` instances to the host thread and do not delete them from the
  host thread.
- Avoid `std::thread`/sleep loops for polling inside Qt adapters. Prefer `QTimer` and async Qt I/O.
- On shutdown, the SDK waits for adapter `hostStop()` before stopping the Qt backend, then destroys
  the backend target in its affinity thread. Adapter code must still release its own QObject tree
  during `stop()`.

### Recommended status mapping

- Missing required field => `CmdStatus::InvalidArgument`
- Unknown action/channel => `CmdStatus::NotSupported`
- Transport/protocol failure => `CmdStatus::Failure`
- Internal coding bug/exception => `CmdStatus::InternalError`
- Normal completion => `CmdStatus::Success`

## Example

See `examples/qthread_usage_example.cpp` for:
- a factory that uses `phicore::adapter::sdk::qt::createInstanceExecutionBackend()`
- `onChannelInvoke(...)` that schedules work via `QTimer::singleShot(...)`
- immediate and async completion paths that both call `sendResult(...)`
- explicit error/failure responses for unsupported inputs
- event-loop integration via `phi::qt::SidecarDriver` (`app.exec()`), so you need neither a manual
  `while` + sleep loop nor a poll timer.

Note: `SidecarHost::pollOnce(...)` still does the frame dispatching; the driver simply calls it
when the host's poll descriptor becomes readable, which covers inbound frames and outbound work
queued from worker threads.

## License

Apache License 2.0 (same as `phi-adapter-sdk`).

## TLS fields

An adapter that speaks its protocol over TLS gets the fields from
`phi/adapter/qt/tlsconfig.h` rather than writing its own. Not because the code
is long — it is three fields — but because adapters would otherwise disagree
about them: one calling the switch `tls` and the next `useSsl`, one verifying
the certificate by default and the next not. An operator would then have to
learn each adapter's opinion about the same question, and the one that quietly
accepts any certificate would look exactly like the one that does not.

`tlsConfigFields()` returns them ready to append to a schema section, with the
certificate and the hostname check shown only when the switch is on.
`tlsSettingsFrom()` reads them back, so `"false"` and `false` and a missing key
mean the same thing everywhere. The defaults are the safe ones: off unless the
operator asked, and verified when on.

There is deliberately no switch that trusts any certificate. Encryption without
verification stops somebody reading the wire and does nothing about somebody
standing in the middle of it — and unlike a browser, which at least shows a
warning page, an adapter would connect silently while the interface said "TLS".
An endpoint with a self-signed certificate has a correct answer already: name
the certificate in `tlsCaFile`.
