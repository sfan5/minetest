#include <string>

#include "porting.h"
#include "wrapper.h"


namespace porting {
    void initializePathsiOS() {
        char buf[128];

        wrapper_paths(WRAPPER_LIBRARY_SUPPORT, buf, sizeof(buf));
        path_user = std::string(buf);
        path_share = std::string(buf);
        wrapper_paths(WRAPPER_LIBRARY_CACHE, buf, sizeof(buf));
        path_cache = std::string(buf);
    }

    void copyAssets() {
        wrapper_assets();
    }
}
