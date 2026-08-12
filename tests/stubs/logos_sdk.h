#pragma once

#include <string>

#include <QMetaType>
#include <QString>
#include <QVariant>
#include <QVariantList>

#include <logos_api.h>
#include <logos_api_client.h>
#if __has_include(<cpp/logos_call_error.h>)
#include <cpp/logos_call_error.h>
#else
#include <logos_call_error.h>
#endif

// Test stub mirroring codegen from interface_dependencies store_eligibility.
// Production builds use generated_code/logos_sdk.h instead.

class StoreEligibility {
public:
    StoreEligibility(std::string moduleName, LogosAPI* api)
        : m_moduleName(std::move(moduleName))
        , m_api(api)
    {
    }

    std::string verifyEligibilityForStoreQuery(
        const std::string& proofBytes,
        const std::string& canonicalRequestBytes,
        const std::string& userPeerId,
        logos::CallError* err = nullptr)
    {
        return invoke(
            "verifyEligibilityForStoreQuery",
            {QString::fromStdString(proofBytes), QString::fromStdString(canonicalRequestBytes),
             QString::fromStdString(userPeerId)},
            err);
    }

    std::string prepareEligibilityProofWithStreamProposalForStoreQuery(
        const std::string& canonicalRequestHex,
        const std::string& providerPeerId,
        logos::CallError* err = nullptr)
    {
        return invoke(
            "prepareEligibilityProofWithStreamProposalForStoreQuery",
            {QString::fromStdString(canonicalRequestHex), QString::fromStdString(providerPeerId)},
            err);
    }

private:
    std::string invoke(
        const char* method,
        const QVariantList& args,
        logos::CallError* err)
    {
        if (m_api == nullptr) {
            if (err != nullptr) {
                err->code = "object_unavailable";
                err->message = "LogosAPI not available in test stub";
            }
            return {};
        }
        LogosAPIClient* client = m_api->getClient(QString::fromStdString(m_moduleName));
        if (client == nullptr) {
            if (err != nullptr) {
                err->code = "object_unavailable";
                err->message = "module not connected: " + m_moduleName;
            }
            return {};
        }
        const QVariant result = client->invokeRemoteMethod(
            QString::fromStdString(m_moduleName), QString::fromUtf8(method), args);
        if (!result.isValid()) {
            if (err != nullptr) {
                err->code = "object_unavailable";
                err->message = "invokeRemoteMethod failed";
            }
            return {};
        }
        if (err != nullptr) {
            err->clear();
        }
        if (result.typeId() == QMetaType::QString) {
            return result.toString().toStdString();
        }
        return result.toString().toStdString();
    }

    std::string m_moduleName;
    LogosAPI* m_api = nullptr;
};

struct LogosModules {
    LogosModules() = default;

    // Test harness only: production LogosModules has no raw api field.
    void setTestApi(LogosAPI* api) { m_testApi = api; }

    StoreEligibility bind_store_eligibility(const std::string& moduleName) const
    {
        return StoreEligibility(moduleName, m_testApi);
    }

private:
    LogosAPI* m_testApi = nullptr;
};
