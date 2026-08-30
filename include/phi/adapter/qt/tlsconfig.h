#pragma once

// The fields an adapter offers when it can talk TLS, and what they mean.
//
// Here rather than in each adapter because the danger is not that two adapters
// write the same code twice - the code is small - but that they disagree about
// it. One calls the switch `tls`, the next `useSsl`, the third `secure`; one
// verifies the certificate by default and the next does not. An operator would
// then have to learn each adapter's opinion about the same question, and the
// one that quietly accepts any certificate would look exactly like the one that
// does not.
//
// So the vocabulary is decided once: these keys, these labels, these defaults.
// An adapter that speaks a protocol over TLS appends these fields and reads
// them back through the same header, and the answer to "is this connection
// actually verified" is the same everywhere.
//
// What is deliberately not here is a switch that trusts any certificate.
// Encryption without verification stops somebody reading the wire and does
// nothing about somebody standing in the middle of it - and unlike a browser,
// which at least shows a warning page, an adapter would connect silently while
// the interface said "TLS". A self-signed broker has a correct answer already:
// name the certificate in `tlsCaFile`. That is one more step for the operator
// and the difference between encrypted and secure.

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

namespace phicore::adapter {

/// The key of the switch, so an adapter can find it without spelling it out.
inline constexpr const char *kTlsFieldKey = "tls";
inline constexpr const char *kTlsCaFileFieldKey = "tlsCaFile";
inline constexpr const char *kTlsVerifyHostnameFieldKey = "tlsVerifyHostname";

/// The three fields, ready to append to a schema section's `fields` array.
///
/// The certificate and the hostname check are shown only when the switch is on,
/// through the schema's own visibility rules, so a form for a plain connection
/// does not carry two questions that cannot apply to it.
///
/// `parentActionId` belongs to adapters that build these into an action's form
/// rather than into the instance section; empty otherwise.
[[nodiscard]] QJsonArray tlsConfigFields(const QString &parentActionId = QString());

/// What an instance's meta says about TLS.
///
/// Read through this rather than out of the object directly, so that "the
/// switch is the string \"true\"" and "the switch is missing" mean the same
/// thing in every adapter. The defaults are the safe ones: off, and when on,
/// verified.
struct TlsSettings {
    /// Whether to speak TLS at all. Off unless the operator said otherwise: an
    /// adapter pointed at a plain endpoint has to keep working.
    bool enabled = false;
    /// A certificate or bundle to trust in addition to the system store, for an
    /// endpoint whose certificate no public authority signed. Empty means the
    /// system store alone.
    QString caFile;
    /// Whether the certificate has to match the address that was dialled. On
    /// unless the operator turned it off, which they need when they connect by
    /// IP to a certificate that names only a host.
    bool verifyHostname = true;
};

[[nodiscard]] TlsSettings tlsSettingsFrom(const QJsonObject &meta);

} // namespace phicore::adapter
