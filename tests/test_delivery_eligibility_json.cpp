#include <logos_test.h>

#include <QJsonArray>
#include <QString>

#include "delivery_eligibility.h"

LOGOS_TEST(eligibilityCodeFromVerifyJson_ok) {
    char desc[64] = {"x"};
    const int code = delivery_eligibility::eligibilityCodeFromVerifyJson(
        R"({"status":"ok","eligibility":"OK"})", desc, sizeof(desc));
    LOGOS_ASSERT_EQ(code, 0);
    LOGOS_ASSERT_EQ(desc[0], '\0');
}

LOGOS_TEST(eligibilityCodeFromVerifyJson_verdict) {
    char desc[64] = {};
    const int code = delivery_eligibility::eligibilityCodeFromVerifyJson(
        R"({"status":"error","eligibility":"PROOF_INVALID","message":"bad sig"})",
        desc,
        sizeof(desc));
    LOGOS_ASSERT_EQ(code, 2);
    LOGOS_ASSERT_EQ(std::string(desc), std::string("bad sig"));
}

LOGOS_TEST(eligibilityProofHexFromPrepareJson_ok) {
    char out[128] = {};
    const int code = delivery_eligibility::eligibilityProofHexFromPrepareJson(
        R"({"status":"ok","result":{"bytes_hex":"abcd"}})", out, sizeof(out));
    LOGOS_ASSERT_EQ(code, 0);
    LOGOS_ASSERT_EQ(std::string(out), std::string("abcd"));
}

LOGOS_TEST(pluginMethodsInclude_finds_method) {
    LOGOS_ASSERT_TRUE(delivery_eligibility::pluginMethodsInclude(
        R"(["foo","verifyEligibilityForStoreQuery"])", "verifyEligibilityForStoreQuery"));
    LOGOS_ASSERT_FALSE(delivery_eligibility::pluginMethodsInclude(
        R"(["foo"])", "verifyEligibilityForStoreQuery"));
}
