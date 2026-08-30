#include "phi/adapter/qt/tlsconfig.h"

namespace phicore::adapter {

namespace {

QJsonObject onlyWhenTlsIsOn()
{
    QJsonObject visibility;
    visibility.insert(QStringLiteral("fieldKey"), QString::fromLatin1(kTlsFieldKey));
    visibility.insert(QStringLiteral("value"), true);
    visibility.insert(QStringLiteral("op"), QStringLiteral("equals"));
    return visibility;
}

QJsonObject field(const char *key, const char *type, const QString &label,
                  const QString &description, const QJsonValue &defaultValue,
                  const QString &parentActionId, const QJsonObject &visibility = {})
{
    QJsonObject out;
    out.insert(QStringLiteral("key"), QString::fromLatin1(key));
    out.insert(QStringLiteral("type"), QString::fromLatin1(type));
    out.insert(QStringLiteral("label"), label);
    out.insert(QStringLiteral("description"), description);
    out.insert(QStringLiteral("default"), defaultValue);
    if (!parentActionId.isEmpty())
        out.insert(QStringLiteral("parentActionId"), parentActionId);
    if (!visibility.isEmpty())
        out.insert(QStringLiteral("visibility"), visibility);
    return out;
}

/// Whether a value that may be a bool, a number or a string means yes.
///
/// A stored setting has been through JSON, a form and a database, and comes
/// back as whatever those left it as. Deciding here rather than in each adapter
/// is most of the point of this file: `"false"` is a non-empty string and reads
/// as true to anybody who writes the obvious thing.
bool truthOf(const QJsonValue &value, bool fallback)
{
    if (value.isBool())
        return value.toBool();
    if (value.isDouble())
        return value.toDouble() != 0.0;
    if (value.isString()) {
        const QString text = value.toString().trimmed().toLower();
        if (text == QLatin1String("true") || text == QLatin1String("1")
            || text == QLatin1String("yes") || text == QLatin1String("on")) {
            return true;
        }
        if (text == QLatin1String("false") || text == QLatin1String("0")
            || text == QLatin1String("no") || text == QLatin1String("off")) {
            return false;
        }
        return fallback;
    }
    return fallback;
}

} // namespace

QJsonArray tlsConfigFields(const QString &parentActionId)
{
    QJsonArray fields;
    fields.append(field(kTlsFieldKey, "Boolean", QStringLiteral("TLS"),
                        QStringLiteral("Encrypt the connection. Off unless the endpoint offers"
                                       " it - a plain endpoint has to keep working."),
                        QJsonValue(false), parentActionId));
    fields.append(field(kTlsCaFileFieldKey, "String",
                        QStringLiteral("CA certificate"),
                        QStringLiteral("Path to a certificate or bundle to trust in addition to"
                                       " the system store. Needed when the endpoint's"
                                       " certificate was not signed by a public authority,"
                                       " which is the usual case on a local network."),
                        QJsonValue(QString()), parentActionId, onlyWhenTlsIsOn()));
    fields.append(field(kTlsVerifyHostnameFieldKey, "Boolean",
                        QStringLiteral("Check the certificate's hostname"),
                        QStringLiteral("Require the certificate to name the address that was"
                                       " dialled. Turn this off only when connecting by IP to a"
                                       " certificate that names a host - the certificate is"
                                       " still checked against the trusted authorities."),
                        QJsonValue(true), parentActionId, onlyWhenTlsIsOn()));
    return fields;
}

TlsSettings tlsSettingsFrom(const QJsonObject &meta)
{
    TlsSettings settings;
    settings.enabled = truthOf(meta.value(QString::fromLatin1(kTlsFieldKey)), false);
    settings.caFile = meta.value(QString::fromLatin1(kTlsCaFileFieldKey)).toString().trimmed();
    settings.verifyHostname =
        truthOf(meta.value(QString::fromLatin1(kTlsVerifyHostnameFieldKey)), true);
    return settings;
}

} // namespace phicore::adapter
