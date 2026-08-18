#pragma once

#include <string>
#include <vector>

#include <QByteArray>
#include <QString>
#include <QStringList>

#include <phi/adapter/v1/enum_names.h>

#include "types.h"

namespace phicore::adapter {

namespace detail {

inline std::string toUtf8(const QString &value)
{
    const QByteArray bytes = value.toUtf8();
    return std::string(bytes.constData(), static_cast<std::size_t>(bytes.size()));
}

inline QString fromUtf8(const std::string &value)
{
    return QString::fromUtf8(value.data(), static_cast<qsizetype>(value.size()));
}

inline QStringList fromUtf8List(const std::vector<std::string> &values)
{
    QStringList list;
    list.reserve(static_cast<qsizetype>(values.size()));
    for (const std::string &entry : values)
        list.push_back(fromUtf8(entry));
    return list;
}

} // namespace detail

inline QString cmdStatusName(CmdStatus status)
{
    return detail::fromUtf8(
        v1::enum_names::cmdStatusName(static_cast<v1::CmdStatus>(static_cast<int>(status))));
}

inline QString actionResultTypeName(ActionResultType resultType)
{
    return detail::fromUtf8(
        v1::enum_names::actionResultTypeName(
            static_cast<v1::ActionResultType>(static_cast<int>(resultType))));
}

inline QString enumNameFor(const QString &enumTypeName, int value, bool fallbackNumber = true)
{
    return detail::fromUtf8(
        v1::enum_names::enumNameFor(detail::toUtf8(enumTypeName), value, fallbackNumber));
}

inline QString enumNameFor(const char *enumTypeName, int value, bool fallbackNumber = true)
{
    if (!enumTypeName)
        return fallbackNumber ? QString::number(value) : QString();

    return detail::fromUtf8(v1::enum_names::enumNameFor(enumTypeName, value, fallbackNumber));
}

inline QStringList flagNamesFor(const QString &enumTypeName, int mask)
{
    return detail::fromUtf8List(v1::enum_names::flagNamesFor(detail::toUtf8(enumTypeName), mask));
}

inline QStringList flagNamesFor(const char *enumTypeName, int mask)
{
    if (!enumTypeName)
        return {};

    return detail::fromUtf8List(v1::enum_names::flagNamesFor(enumTypeName, mask));
}

inline bool parseEnumValueByName(const QString &enumTypeName, const QString &name, int *outValue)
{
    return v1::enum_names::parseEnumValueByName(
        detail::toUtf8(enumTypeName),
        detail::toUtf8(name),
        outValue);
}

inline bool parseEnumValueByName(const char *enumTypeName, const QString &name, int *outValue)
{
    if (!enumTypeName)
        return false;

    return v1::enum_names::parseEnumValueByName(enumTypeName, detail::toUtf8(name), outValue);
}

} // namespace phicore::adapter
