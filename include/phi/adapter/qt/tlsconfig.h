#pragma once

// The TLS fields, in the shapes a Qt adapter already holds.
//
// The vocabulary itself is not here. It is in the contract -
// `phi/adapter/v1/tlsconfig.h` - because what a field is called and what its
// default is are not properties of an event loop, and an adapter written
// without Qt has to reach the same answer. This file is the conversion and
// nothing else: the same three fields as `QJsonArray`, because Qt adapters
// build their schema as JSON, and the same reading of an instance's `meta`,
// because they hold it as `QJsonObject`.
//
// Both are one call deep. If either grew a decision of its own, that decision
// would be the Qt adapters' opinion about a question the contract already
// answered, which is the thing this whole arrangement exists to prevent.

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

#include "phi/adapter/v1/tlsconfig.h"

namespace phicore::adapter {

using v1::kTlsCaFileFieldKey;
using v1::kTlsFieldKey;
using v1::kTlsVerifyHostnameFieldKey;
using v1::TlsSettings;

/// The contract's fields, as the JSON a Qt adapter appends to its schema.
[[nodiscard]] QJsonArray tlsConfigFields(const QString &parentActionId = QString());

/// What an instance's meta says about TLS, decided by the contract.
[[nodiscard]] TlsSettings tlsSettingsFrom(const QJsonObject &meta);

} // namespace phicore::adapter
