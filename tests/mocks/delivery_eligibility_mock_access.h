#pragma once

#include <stddef.h>

#include "../stubs/lib/liblogosdelivery.h"

EligibilityVerifierCb delivery_mock_lastVerifierCb();
void* delivery_mock_lastVerifierUserData();
EligibilityProviderCb delivery_mock_lastProviderCb();
void* delivery_mock_lastProviderUserData();
