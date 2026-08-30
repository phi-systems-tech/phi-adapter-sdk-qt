#include "phi/adapter/qt/tlsconfig.h"

#include "phi/adapter/v1/enum_names.h"

namespace phicore::adapter {

namespace {

/// A contract scalar as the JSON value it stands for.
QJsonValue jsonOf(const v1::ScalarValue &value)
{
    if (const bool *flag = std::get_if<bool>(&value))
        return QJsonValue(*flag);
    if (const std::int64_t *number = std::get_if<std::int64_t>(&value))
        return QJsonValue(static_cast<double>(*number));
    if (const double *number = std::get_if<double>(&value))
        return QJsonValue(*number);
    if (const v1::Utf8String *text = std::get_if<v1::Utf8String>(&value))
        return QJsonValue(QString::fromStdString(*text));
    return QJsonValue();
}

/// A JSON value as the contract scalar it stands for, so that the reading of it
/// happens once, in the contract, and not once per adapter.
v1::ScalarValue scalarOf(const QJsonValue &value)
{
    if (value.isBool())
        return v1::ScalarValue(value.toBool());
    if (value.isDouble())
        return v1::ScalarValue(value.toDouble());
    if (value.isString())
        return v1::ScalarValue(value.toString().toStdString());
    return v1::ScalarValue();
}

} // namespace

QJsonArray tlsConfigFields(const QString &parentActionId)
{
    QJsonArray out;
    for (const v1::AdapterConfigField &field :
         v1::tlsConfigFields(parentActionId.toStdString())) {
        QJsonObject object;
        object.insert(QStringLiteral("key"), QString::fromStdString(field.key));
        object.insert(QStringLiteral("type"),
                      QString::fromStdString(
                          v1::enum_names::enumNameFor("AdapterConfigFieldType", static_cast<int>(field.type))));
        object.insert(QStringLiteral("label"), QString::fromStdString(field.label));
        object.insert(QStringLiteral("description"), QString::fromStdString(field.description));
        object.insert(QStringLiteral("default"), jsonOf(field.defaultValue));
        if (!field.parentActionId.empty()) {
            object.insert(QStringLiteral("parentActionId"),
                          QString::fromStdString(field.parentActionId));
        }
        if (!field.visibility.fieldKey.empty()) {
            QJsonObject visibility;
            visibility.insert(QStringLiteral("fieldKey"),
                              QString::fromStdString(field.visibility.fieldKey));
            visibility.insert(QStringLiteral("value"), jsonOf(field.visibility.value));
            visibility.insert(QStringLiteral("op"),
                              QString::fromStdString(
                                  v1::enum_names::enumNameFor("AdapterConfigVisibilityOp",
                                                  static_cast<int>(field.visibility.op)))
                                  .toLower());
            object.insert(QStringLiteral("visibility"), visibility);
        }
        out.append(object);
    }
    return out;
}

TlsSettings tlsSettingsFrom(const QJsonObject &meta)
{
    return v1::tlsSettingsFrom(scalarOf(meta.value(QString::fromLatin1(kTlsFieldKey))),
                               scalarOf(meta.value(QString::fromLatin1(kTlsCaFileFieldKey))),
                               scalarOf(meta.value(QString::fromLatin1(kTlsVerifyHostnameFieldKey))));
}

} // namespace phicore::adapter
