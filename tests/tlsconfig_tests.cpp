// The conversion from the contract's fields to the JSON a Qt adapter emits.
//
// The vocabulary is not tested here - it is tested where it lives, in the
// contract. What is worth holding here is that nothing is lost or invented on
// the way through: the keys, the defaults and the visibility rule that come out
// as JSON are the ones the contract decided, and a value read back out of a
// QJsonObject reaches the same answer the contract would.

#include <phi/adapter/testing/check.h>

#include "phi/adapter/qt/tlsconfig.h"

#include <QJsonArray>
#include <QJsonObject>

using namespace phicore::adapter;

namespace {

QJsonObject fieldNamed(const QJsonArray &fields, const QString &key)
{
    for (const QJsonValue &value : fields) {
        const QJsonObject field = value.toObject();
        if (field.value(QStringLiteral("key")).toString() == key)
            return field;
    }
    return {};
}

void testTheContractsFieldsArriveIntactAsJson()
{
    const QJsonArray fields = tlsConfigFields();
    PHI_CHECK(fields.size() == static_cast<int>(v1::tlsConfigFields().size()));

    const QJsonObject tls = fieldNamed(fields, QStringLiteral("tls"));
    PHI_CHECK(tls.value(QStringLiteral("type")).toString() == QStringLiteral("Boolean"));
    PHI_CHECK(tls.value(QStringLiteral("default")).toBool() == false);
    PHI_CHECK(!tls.value(QStringLiteral("label")).toString().isEmpty());
    PHI_CHECK(!tls.value(QStringLiteral("description")).toString().isEmpty());

    const QJsonObject ca = fieldNamed(fields, QStringLiteral("tlsCaFile"));
    PHI_CHECK(ca.value(QStringLiteral("type")).toString() == QStringLiteral("String"));

    const QJsonObject verify = fieldNamed(fields, QStringLiteral("tlsVerifyHostname"));
    PHI_CHECK(verify.value(QStringLiteral("type")).toString() == QStringLiteral("Boolean"));
    PHI_CHECK(verify.value(QStringLiteral("default")).toBool() == true);
}

void testTheVisibilityRuleSurvivesTheConversion()
{
    const QJsonArray fields = tlsConfigFields();
    for (const QString &key : {QStringLiteral("tlsCaFile"),
                               QStringLiteral("tlsVerifyHostname")}) {
        const QJsonObject visibility =
            fieldNamed(fields, key).value(QStringLiteral("visibility")).toObject();
        PHI_CHECK(visibility.value(QStringLiteral("fieldKey")).toString()
                  == QStringLiteral("tls"));
        PHI_CHECK(visibility.value(QStringLiteral("value")).toBool());
        // Lower case, which is the spelling core parses back.
        PHI_CHECK(visibility.value(QStringLiteral("op")).toString() == QStringLiteral("equals"));
    }
    PHI_CHECK(!fieldNamed(fields, QStringLiteral("tls"))
                   .contains(QStringLiteral("visibility")));
}

void testAnActionsFormCarriesTheFieldsToo()
{
    for (const QJsonValue &value : tlsConfigFields(QStringLiteral("probe"))) {
        PHI_CHECK(value.toObject().value(QStringLiteral("parentActionId")).toString()
                  == QStringLiteral("probe"));
    }
    for (const QJsonValue &value : tlsConfigFields())
        PHI_CHECK(!value.toObject().contains(QStringLiteral("parentActionId")));
}

void testReadingMetaReachesTheContractsAnswer()
{
    // An instance made before the adapter offered these fields, which is every
    // instance that exists today.
    const TlsSettings none = tlsSettingsFrom({});
    PHI_CHECK(!none.enabled);
    PHI_CHECK(none.caFile.empty());
    PHI_CHECK(none.verifyHostname);

    // And the shapes a stored setting comes back in, decided in the contract
    // rather than here - the string "false" is not empty and would read as true
    // to anybody who wrote the obvious thing.
    PHI_CHECK(tlsSettingsFrom({{QStringLiteral("tls"), QStringLiteral("true")}}).enabled);
    PHI_CHECK(!tlsSettingsFrom({{QStringLiteral("tls"), QStringLiteral("false")}}).enabled);
    PHI_CHECK(tlsSettingsFrom({{QStringLiteral("tls"), 1}}).enabled);
    PHI_CHECK(!tlsSettingsFrom({{QStringLiteral("tlsVerifyHostname"), false}}).verifyHostname);
    PHI_CHECK(tlsSettingsFrom({{QStringLiteral("tlsVerifyHostname"),
                                QStringLiteral("perhaps")}}).verifyHostname);

    const TlsSettings withCa = tlsSettingsFrom(
        {{QStringLiteral("tls"), true},
         {QStringLiteral("tlsCaFile"), QStringLiteral("  /etc/ssl/broker.crt  ")}});
    PHI_CHECK(withCa.caFile == "/etc/ssl/broker.crt");
}

} // namespace

int main()
{
    testTheContractsFieldsArriveIntactAsJson();
    testTheVisibilityRuleSurvivesTheConversion();
    testAnActionsFormCarriesTheFieldsToo();
    testReadingMetaReachesTheContractsAnswer();
    return phi::testing::report("tlsconfig_tests");
}
