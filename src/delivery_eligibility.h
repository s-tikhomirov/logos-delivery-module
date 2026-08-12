#pragma once

#include <cstddef>
#include <string>

#include <nlohmann/json.hpp>

namespace delivery_eligibility {

constexpr int kStatusInternalError = -1;

int eligibilityCodeFromVerifyJson(const char* json, char* outDesc, size_t outDescLen);

int eligibilityProofHexFromPrepareJson(const char* json, char* outProofHex, size_t outBufLen);

bool pluginMethodsInclude(const char* methodsJson, const char* methodName);

// Normalize an lp_invoke / bind result into the JSON text expected by the
// parsers above. Accepts a JSON string, a bare verdict/prepare object, or a
// LogosResult-shaped {"value": ...} wrapper (historical Qt QVariantMap path).
std::string eligibilityJsonFromInvokeResult(const nlohmann::json& result);

} // namespace delivery_eligibility
