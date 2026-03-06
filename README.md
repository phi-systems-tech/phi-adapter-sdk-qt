# phi-adapter-sdk-qt

Qt helper package for `phi-adapter-sdk`.

## Contents

- `phi::adapter-sdk-qt` shared library helper for Qt event-loop based adapter instance execution.
- Optional runtime dependency on Qt6 Core.

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

## License

Apache License 2.0 (same as `phi-adapter-sdk`).
