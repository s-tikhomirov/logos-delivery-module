#pragma once

#include <cstddef>

namespace delivery_eligibility {

constexpr int kStatusInternalError = -1;

int eligibilityCodeFromVerifyJson(const char* json, char* outDesc, size_t outDescLen);

int eligibilityProofHexFromPrepareJson(const char* json, char* outProofHex, size_t outBufLen);

bool pluginMethodsInclude(const char* methodsJson, const char* methodName);

} // namespace delivery_eligibility
