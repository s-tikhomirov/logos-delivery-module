#pragma once

// Dependency interface for Store eligibility (D45.20).
// Bound at runtime via modules().bind_store_eligibility(moduleName).
// Any module whose API is a superset of these methods can satisfy it
// (payment_streams_module is the current provider).

#include <string>

#include <logos_module_context.h>

class IStoreEligibility {
public:
    // Returns JSON; delivery_module parses eligibility verdict fields.
    std::string verifyEligibilityForStoreQuery(
        const std::string& proofBytes,
        const std::string& canonicalRequestBytes,
        const std::string& userPeerId);

    // Returns JSON; delivery_module extracts proof hex for the outbound query.
    std::string prepareEligibilityProofWithStreamProposalForStoreQuery(
        const std::string& canonicalRequestHex,
        const std::string& providerPeerId);
};
