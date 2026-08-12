#include <logos_test.h>

#include <memory>

#include "delivery_module_plugin.h"
#include "logos_sdk.h"
#include "mocks/delivery_eligibility_mock_access.h"
#include "mocks/delivery_module_events_stub.h"

struct TestLogosModulesHolder {
    std::unique_ptr<LogosModules> modules;

    void reset(LogosAPI* api)
    {
        modules = std::make_unique<LogosModules>();
        modules->setTestApi(api);
    }
};

static void wireLogosApi(LogosTestContext& t, DeliveryModuleImpl* impl, TestLogosModulesHolder& holder)
{
    holder.reset(t.api());
    impl->_logosCoreSetLogosModulesPtr_(holder.modules.get());
    impl->_logosCoreSetContext_("test-module-path", "test-instance", "/tmp/delivery-module-test");
}

static DeliveryModuleImpl* createInitializedImplWithApi(LogosTestContext& t, TestLogosModulesHolder& holder)
{
    t.mockCFunction("logosdelivery_create_node").returns(1);
    auto* impl = new DeliveryModuleImpl();
    wireLogosApi(t, impl, holder);
    LOGOS_ASSERT_TRUE(impl->createNode(R"({"logLevel":"INFO"})").success);
    return impl;
}

LOGOS_TEST(setEligibilityVerifier_fails_without_createNode) {
    auto t = LogosTestContext("delivery_module");
    DeliveryModuleImpl impl;
    TestLogosModulesHolder holder;
    wireLogosApi(t, &impl, holder);
    LOGOS_ASSERT_FALSE(impl.setEligibilityVerifier("payment_streams_module").success);
}

LOGOS_TEST(setEligibilityVerifier_registers_ffi_hook) {
    auto t = LogosTestContext("delivery_module");
    TestLogosModulesHolder holder;
    auto* impl = createInitializedImplWithApi(t, holder);

    LOGOS_ASSERT_TRUE(impl->setEligibilityVerifier("ps_module").success);
    LOGOS_ASSERT(t.cFunctionCalled("logosdelivery_set_eligibility_verifier"));
    LOGOS_ASSERT(delivery_mock_lastVerifierCb() != nullptr);

    delete impl;
}

LOGOS_TEST(verifier_trampoline_null_proof_invokes_module) {
    auto t = LogosTestContext("delivery_module");
    TestLogosModulesHolder holder;
    auto* impl = createInitializedImplWithApi(t, holder);

    LOGOS_ASSERT_TRUE(impl->setEligibilityVerifier("ps_module").success);

    t.mockModule("ps_module", "verifyEligibilityForStoreQuery")
        .returns(R"({"status":"error","eligibility":"PARAMS_REJECTED","message":"empty"})");

    char desc[64] = {};
    const int code = delivery_mock_lastVerifierCb()(
        nullptr, "canonical", "peerA", desc, sizeof(desc), delivery_mock_lastVerifierUserData());
    LOGOS_ASSERT_EQ(code, 1);
    LOGOS_ASSERT_EQ(std::string(desc), std::string("empty"));
    LOGOS_ASSERT(t.moduleCalledWith("ps_module", "verifyEligibilityForStoreQuery",
                                    {QString(), QStringLiteral("canonical"), QStringLiteral("peerA")}));

    delete impl;
}

LOGOS_TEST(storeQueryWithEligibility_emits_completion_event) {
    auto t = LogosTestContext("delivery_module");
    delivery_test_events::resetStoreQueryEvents();
    TestLogosModulesHolder holder;
    auto* impl = createInitializedImplWithApi(t, holder);

    t.mockCFunction("logosdelivery_store_query").returns(R"({"requestId":"r1","statusCode":200})");
    LOGOS_ASSERT_TRUE(impl->storeQueryWithEligibility(R"({"requestId":"r1","includeData":true,"paginationForward":true})",
                                       "/ip4/127.0.0.1/tcp/60000/p2p/abc")
                        .success);
    LOGOS_ASSERT_TRUE(delivery_test_events::g_lastStoreQueryCompleted.fired);
    LOGOS_ASSERT_TRUE(delivery_test_events::g_lastStoreQueryCompleted.success);
    LOGOS_ASSERT_CONTAINS(delivery_test_events::g_lastStoreQueryCompleted.responseJson, "requestId");

    delete impl;
}

LOGOS_TEST(setEligibilityProvider_registers_ffi_hook) {
    auto t = LogosTestContext("delivery_module");
    TestLogosModulesHolder holder;
    auto* impl = createInitializedImplWithApi(t, holder);

    LOGOS_ASSERT_TRUE(impl->setEligibilityProvider("ps_module").success);
    LOGOS_ASSERT(t.cFunctionCalled("logosdelivery_set_eligibility_provider"));
    LOGOS_ASSERT(delivery_mock_lastProviderCb() != nullptr);

    delete impl;
}

LOGOS_TEST(clear_eligibility_verifier_calls_ffi_null) {
    auto t = LogosTestContext("delivery_module");
    TestLogosModulesHolder holder;
    auto* impl = createInitializedImplWithApi(t, holder);

    LOGOS_ASSERT_TRUE(impl->setEligibilityVerifier("ps_module").success);
    LOGOS_ASSERT_TRUE(impl->setEligibilityVerifier("").success);
    LOGOS_ASSERT_EQ(delivery_mock_lastVerifierCb(), nullptr);

    delete impl;
}
