#include "delivery_eligibility.h"

#include <cstring>
#include <string>

#include <nlohmann/json.hpp>

namespace delivery_eligibility {
namespace {

int mapEligibilityStringToCode(const std::string& eligibility)
{
    if (eligibility == "OK") {
        return 0;
    }
    if (eligibility == "PARAMS_REJECTED") {
        return 1;
    }
    if (eligibility == "PROOF_INVALID") {
        return 2;
    }
    if (eligibility == "STREAM_NOT_ACTIVE") {
        return 3;
    }
    return kStatusInternalError;
}

void copyTruncated(const std::string& src, char* outDesc, size_t outDescLen)
{
    if (outDescLen == 0 || outDesc == nullptr) {
        return;
    }
    const size_t maxCopy = outDescLen - 1;
    const size_t n = src.size() < maxCopy ? src.size() : maxCopy;
    if (n > 0) {
        std::memcpy(outDesc, src.data(), n);
    }
    outDesc[n] = '\0';
}

} // namespace

std::string eligibilityJsonFromInvokeResult(const nlohmann::json& result)
{
    if (result.is_string()) {
        return result.get<std::string>();
    }
    if (!result.is_object()) {
        return {};
    }
    if (result.contains("status")) {
        return result.dump();
    }
    if (result.contains("value")) {
        const nlohmann::json& value = result.at("value");
        if (value.is_string()) {
            return value.get<std::string>();
        }
        if (value.is_object()) {
            return value.dump();
        }
    }
    if (result.contains("result")) {
        const nlohmann::json& nested = result.at("result");
        if (nested.is_string()) {
            return nested.get<std::string>();
        }
        if (nested.is_object()) {
            return nested.dump();
        }
    }
    return result.dump();
}

int eligibilityCodeFromVerifyJson(const char* json, char* outDesc, size_t outDescLen)
{
    if (outDesc != nullptr && outDescLen > 0) {
        outDesc[0] = '\0';
    }
    if (json == nullptr) {
        return kStatusInternalError;
    }

    nlohmann::json obj;
    try {
        obj = nlohmann::json::parse(json);
    } catch (const nlohmann::json::parse_error&) {
        return kStatusInternalError;
    }

    if (!obj.is_object()) {
        return kStatusInternalError;
    }

    const std::string status = obj.value("status", "");
    if (status == "ok") {
        const std::string eligibility = obj.value("eligibility", "OK");
        if (eligibility != "OK") {
            return mapEligibilityStringToCode(eligibility);
        }
        return 0;
    }

    if (status != "error") {
        return kStatusInternalError;
    }

    if (obj.contains("eligibility") && obj["eligibility"].is_string()) {
        const std::string eligibility = obj["eligibility"].get<std::string>();
        const int code = mapEligibilityStringToCode(eligibility);
        if (code == kStatusInternalError) {
            return kStatusInternalError;
        }
        if (obj.contains("message") && obj["message"].is_string()) {
            copyTruncated(obj["message"].get<std::string>(), outDesc, outDescLen);
        }
        return code;
    }

    if (obj.contains("message") && obj["message"].is_string()) {
        copyTruncated(obj["message"].get<std::string>(), outDesc, outDescLen);
    }
    return kStatusInternalError;
}

int eligibilityProofHexFromPrepareJson(const char* json, char* outProofHex, size_t outBufLen)
{
    if (outProofHex == nullptr || outBufLen == 0) {
        return kStatusInternalError;
    }
    outProofHex[0] = '\0';
    if (json == nullptr) {
        return kStatusInternalError;
    }

    nlohmann::json obj;
    try {
        obj = nlohmann::json::parse(json);
    } catch (const nlohmann::json::parse_error&) {
        return kStatusInternalError;
    }

    if (!obj.is_object() || obj.value("status", "") != "ok") {
        return kStatusInternalError;
    }

    nlohmann::json result = obj.value("result", nlohmann::json::object());
    if (!result.is_object()) {
        result = obj;
    }

    std::string bytesHex;
    if (result.contains("bytes_hex") && result["bytes_hex"].is_string()) {
        bytesHex = result["bytes_hex"].get<std::string>();
    } else if (result.contains("bytesHex") && result["bytesHex"].is_string()) {
        bytesHex = result["bytesHex"].get<std::string>();
    } else if (obj.contains("bytes_hex") && obj["bytes_hex"].is_string()) {
        bytesHex = obj["bytes_hex"].get<std::string>();
    } else if (obj.contains("bytesHex") && obj["bytesHex"].is_string()) {
        bytesHex = obj["bytesHex"].get<std::string>();
    } else {
        return kStatusInternalError;
    }

    copyTruncated(bytesHex, outProofHex, outBufLen);
    return 0;
}

bool pluginMethodsInclude(const char* methodsJson, const char* methodName)
{
    if (methodsJson == nullptr || methodName == nullptr) {
        return false;
    }

    nlohmann::json arr;
    try {
        arr = nlohmann::json::parse(methodsJson);
    } catch (const nlohmann::json::parse_error&) {
        return false;
    }

    if (!arr.is_array()) {
        return false;
    }

    for (const auto& entry : arr) {
        if (entry.is_string() && entry.get<std::string>() == methodName) {
            return true;
        }
        if (entry.is_object() && entry.contains("name") && entry["name"].is_string()
            && entry["name"].get<std::string>() == methodName) {
            return true;
        }
    }
    return false;
}

} // namespace delivery_eligibility
