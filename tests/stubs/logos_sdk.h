#pragma once

#include <logos_api.h>

struct LogosModules {
    explicit LogosModules(LogosAPI* api) : api(api) {}
    LogosAPI* api;
};
