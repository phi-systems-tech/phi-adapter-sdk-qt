// The TLS fields every adapter offers, and what they mean.
//
// The point of these living in the SDK is not that the code is long - it is
// three fields - but that adapters would otherwise disagree about them: one
// calling the switch `tls` and the next `useSsl`, one verifying by default and
// the next not. So what is pinned here is the vocabulary and the defaults, and
// especially the two answers that must never drift: off unless asked, and
// verified when on.

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

void testTheVocabularyIsTheOneEverybodyUses()
{
    const QJsonArray fields = tlsConfigFields();
    PHI_CHECK(fields.size() == 3);

    const QJsonObject tls = fieldNamed(fields, QStringLiteral("tls"));
    PHI_CHECK(tls.value(QStringLiteral("type")).toString() == QStringLiteral("Boolean"));
    // Off. An adapter pointed at a plain endpoint has to keep working, and an
    // upgrade that silently starts requiring TLS would take every one of them
    // off the air.
    PHI_CHECK(tls.value(QStringLiteral("default")).toBool() == false);

    const QJsonObject ca = fieldNamed(fields, QStringLiteral("tlsCaFile"));
    PHI_CHECK(ca.value(QStringLiteral("type")).toString() == QStringLiteral("String"));

    const QJsonObject verify = fieldNamed(fields, QStringLiteral("tlsVerifyHostname"));
    PHI_CHECK(verify.value(QStringLiteral("type")).toString() == QStringLiteral("Boolean"));
    // On. This is the whole reason the fields are shared: an adapter that
    // shipped this off would encrypt and authenticate nothing, and would look
    // from the outside exactly like one that does both.
    PHI_CHECK(verify.value(QStringLiteral("default")).toBool() == true);
}

void testTheTwoDetailsAreOnlyAskedWhenTlsIsOn()
{
    const QJsonArray fields = tlsConfigFields();
    for (const QString &key : {QStringLiteral("tlsCaFile"),
                               QStringLiteral("tlsVerifyHostname")}) {
        const QJsonObject visibility =
            fieldNamed(fields, key).value(QStringLiteral("visibility")).toObject();
        PHI_CHECK(visibility.value(QStringLiteral("fieldKey")).toString()
                  == QStringLiteral("tls"));
        PHI_CHECK(visibility.value(QStringLiteral("value")).toBool());
        PHI_CHECK(visibility.value(QStringLiteral("op")).toString() == QStringLiteral("equals"));
    }
    // The switch itself is always shown, or nothing could turn the others on.
    PHI_CHECK(!fieldNamed(fields, QStringLiteral("tls"))
                   .contains(QStringLiteral("visibility")));
}

void testAnActionsFormCarriesTheFieldsToo()
{
    const QJsonArray fields = tlsConfigFields(QStringLiteral("probe"));
    for (const QJsonValue &value : fields) {
        PHI_CHECK(value.toObject().value(QStringLiteral("parentActionId")).toString()
                  == QStringLiteral("probe"));
    }
    // And an instance section's fields belong to no action.
    for (const QJsonValue &value : tlsConfigFields())
        PHI_CHECK(!value.toObject().contains(QStringLiteral("parentActionId")));
}

void testNothingSaidMeansOffAndVerified()
{
    // An instance made before the adapter offered these fields, which is every
    // instance that exists today.
    const TlsSettings settings = tlsSettingsFrom({});
    PHI_CHECK(!settings.enabled);
    PHI_CHECK(settings.caFile.isEmpty());
    PHI_CHECK(settings.verifyHostname);
}

void testTheAnswerIsReadTheSameWhateverShapeItComesBackIn()
{
    // A stored setting has been through JSON, a form and a database, and comes
    // back as whatever those left it as. Deciding this once is most of the
    // point: `"false"` is a non-empty string and reads as true to anybody who
    // writes the obvious thing.
    PHI_CHECK(tlsSettingsFrom({{QStringLiteral("tls"), true}}).enabled);
    PHI_CHECK(tlsSettingsFrom({{QStringLiteral("tls"), QStringLiteral("true")}}).enabled);
    PHI_CHECK(tlsSettingsFrom({{QStringLiteral("tls"), QStringLiteral("on")}}).enabled);
    PHI_CHECK(tlsSettingsFrom({{QStringLiteral("tls"), 1}}).enabled);
    PHI_CHECK(!tlsSettingsFrom({{QStringLiteral("tls"), QStringLiteral("false")}}).enabled);
    PHI_CHECK(!tlsSettingsFrom({{QStringLiteral("tls"), QStringLiteral("0")}}).enabled);
    PHI_CHECK(!tlsSettingsFrom({{QStringLiteral("tls"), 0}}).enabled);

    // And a value nobody can make sense of falls to the safe answer, which is
    // not the same one for both: off for the switch, on for the check.
    PHI_CHECK(!tlsSettingsFrom({{QStringLiteral("tls"), QStringLiteral("perhaps")}}).enabled);
    PHI_CHECK(tlsSettingsFrom({{QStringLiteral("tlsVerifyHostname"),
                                QStringLiteral("perhaps")}}).verifyHostname);
    PHI_CHECK(!tlsSettingsFrom({{QStringLiteral("tlsVerifyHostname"), false}}).verifyHostname);
    PHI_CHECK(!tlsSettingsFrom({{QStringLiteral("tlsVerifyHostname"),
                                 QStringLiteral("no")}}).verifyHostname);
}

void testTheCertificateIsReadWithoutItsSurroundingSpace()
{
    const TlsSettings settings = tlsSettingsFrom(
        {{QStringLiteral("tls"), true},
         {QStringLiteral("tlsCaFile"), QStringLiteral("  /etc/ssl/broker.crt  ")}});
    PHI_CHECK(settings.caFile == QStringLiteral("/etc/ssl/broker.crt"));
}

} // namespace

int main()
{
    testTheVocabularyIsTheOneEverybodyUses();
    testTheTwoDetailsAreOnlyAskedWhenTlsIsOn();
    testAnActionsFormCarriesTheFieldsToo();
    testNothingSaidMeansOffAndVerified();
    testTheAnswerIsReadTheSameWhateverShapeItComesBackIn();
    testTheCertificateIsReadWithoutItsSurroundingSpace();
    return phi::testing::report("tlsconfig_tests");
}
