#pragma once

#include <string>

#include <QMetaType>
#include <QString>
#include <QVariant>
#include <QVariantList>

#include <nlohmann/json.hpp>

#include <logos_api.h>
#include <logos_api_client.h>
#if __has_include(<cpp/logos_call_error.h>)
#include <cpp/logos_call_error.h>
#else
#include <logos_call_error.h>
#endif

// Test stub mirroring codegen from interface_dependencies store_eligibility.
// Production builds use generated_code/logos_sdk.h instead.
// EligibilityWireJson maps to LIDL any → LogosMap (nlohmann::json) in codegen.

using LogosMap = nlohmann::json;

class StoreEligibility {
public:
    StoreEligibility(std::string moduleName, LogosAPI* api)
        : m_moduleName(std::move(moduleName))
        , m_api(api)
    {
    }

    LogosMap verifyEligibilityForStoreQuery(
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

    LogosMap prepareEligibilityProofWithStreamProposalForStoreQuery(
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
    LogosMap invoke(
        const char* method,
        const QVariantList& args,
        logos::CallError* err)
    {
        if (m_api == nullptr) {
            if (err != nullptr) {
                err->code = "object_unavailable";
                err->message = "LogosAPI not available in test stub";
            }
            return nullptr;
        }
        LogosAPIClient* client = m_api->getClient(QString::fromStdString(m_moduleName));
        if (client == nullptr) {
            if (err != nullptr) {
                err->code = "object_unavailable";
                err->message = "module not connected: " + m_moduleName;
            }
            return nullptr;
        }
        const QVariant result = client->invokeRemoteMethod(
            QString::fromStdString(m_moduleName), QString::fromUtf8(method), args);
        if (!result.isValid()) {
            if (err != nullptr) {
                err->code = "object_unavailable";
                err->message = "invokeRemoteMethod failed";
            }
            return nullptr;
        }
        if (err != nullptr) {
            err->clear();
        }
        if (result.typeId() == QMetaType::QString) {
            const std::string text = result.toString().toStdString();
            auto parsed = nlohmann::json::parse(text, nullptr, false);
            if (!parsed.is_discarded()) {
                return parsed;
            }
            return text;
        }
        return result.toString().toStdString();
    }

    std::string m_moduleName;
    LogosAPI* m_api = nullptr;
};

struct LogosModules {
    LogosModules() = default;

    // Matches Std/Qt generated LogosModules (universal builds use LOGOS_API_STYLE=std).
    LogosAPI* api = nullptr;

    // Test harness: production sets api via LogosModules(LogosAPI*).
    void setTestApi(LogosAPI* testApi) { api = testApi; }

    StoreEligibility bind_store_eligibility(const std::string& moduleName) const
    {
        return StoreEligibility(moduleName, api);
    }
};
