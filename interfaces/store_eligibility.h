#pragma once

// Dependency interface for Store eligibility (D45.20).
// Bound at runtime via modules().bind_store_eligibility(moduleName).
// Any module whose API is a superset of these methods can satisfy it
// (payment_streams_module is the current provider).
//
// Return type is an opaque LIDL `any` marker (not std::string): the Lp
// string decoder drops JSON-object returns, and payment_streams may surface
// the verdict/prepare payload as either a JSON string or an object on the
// wire (historical Qt QVariantMap path).

#include <string>

#include <logos_module_context.h>

struct EligibilityWireJson;

class IStoreEligibility {
public:
    // Returns JSON (string or object); delivery_module normalizes and parses.
    EligibilityWireJson verifyEligibilityForStoreQuery(
        const std::string& proofBytes,
        const std::string& canonicalRequestBytes,
        const std::string& userPeerId);

    // Returns JSON (string or object); delivery_module extracts proof hex.
    EligibilityWireJson prepareEligibilityProofWithStreamProposalForStoreQuery(
        const std::string& canonicalRequestHex,
        const std::string& providerPeerId);
};
