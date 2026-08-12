#include <logos_test.h>

#include <QJsonArray>
#include <QString>

#include "delivery_eligibility.h"

LOGOS_TEST(eligibilityJsonFromInvokeResult_string_and_object) {
    using delivery_eligibility::eligibilityJsonFromInvokeResult;
    LOGOS_ASSERT_EQ(
        eligibilityJsonFromInvokeResult(nlohmann::json("{\"status\":\"ok\"}")),
        std::string("{\"status\":\"ok\"}"));
    const auto obj = nlohmann::json::parse(R"({"status":"ok","eligibility":"OK"})");
    LOGOS_ASSERT_CONTAINS(eligibilityJsonFromInvokeResult(obj), "eligibility");
    const auto wrapped = nlohmann::json::parse(
        R"({"success":true,"value":{"status":"ok","eligibility":"OK"},"error":null})");
    LOGOS_ASSERT_CONTAINS(eligibilityJsonFromInvokeResult(wrapped), "status");
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

LOGOS_TEST(eligibilityProofHexFromPrepareJson_flat_ps_shape) {
    char out[128] = {};
    const int code = delivery_eligibility::eligibilityProofHexFromPrepareJson(
        R"({"status":"ok","bytes_hex":"ef01","kind":"stream_proof"})", out, sizeof(out));
    LOGOS_ASSERT_EQ(code, 0);
    LOGOS_ASSERT_EQ(std::string(out), std::string("ef01"));
}

LOGOS_TEST(pluginMethodsInclude_finds_method) {
    LOGOS_ASSERT_TRUE(delivery_eligibility::pluginMethodsInclude(
        R"(["foo","verifyEligibilityForStoreQuery"])", "verifyEligibilityForStoreQuery"));
    LOGOS_ASSERT_FALSE(delivery_eligibility::pluginMethodsInclude(
        R"(["foo"])", "verifyEligibilityForStoreQuery"));
}

LOGOS_TEST(pluginMethodsInclude_finds_method_in_object_array) {
    LOGOS_ASSERT_TRUE(delivery_eligibility::pluginMethodsInclude(
        R"([{"name":"prepareEligibilityProofWithStreamProposalForStoreQuery"},{"name":"verifyEligibilityForStoreQuery"}])",
        "verifyEligibilityForStoreQuery"));
    LOGOS_ASSERT_FALSE(delivery_eligibility::pluginMethodsInclude(
        R"([{"name":"prepareEligibilityProofWithStreamProposalForStoreQuery"}])", "verifyEligibilityForStoreQuery"));
}
