#include <string>

#include "porting.h"
#include "config.h"
#include "wrapper.h"


namespace porting {
    void initializePathsiOS() {
        char buf[128];

        wrapper_paths(WRAPPER_DOCUMENTS, buf, sizeof(buf));
        path_user = std::string(buf);
		wrapper_paths(WRAPPER_LIBRARY_SUPPORT, buf, sizeof(buf));
		path_share = std::string(buf);
        wrapper_paths(WRAPPER_LIBRARY_CACHE, buf, sizeof(buf));
        path_cache = std::string(buf);
    }

    void copyAssets() {
        wrapper_assets();
    }

    float getDisplayDensity() {
        return 1.0;
        //return wrapper_scale();
    }

    v2u32 getDisplaySize() {
        static bool firstrun = true;
        static v2u32 retval;

        if(firstrun) {
            unsigned int values[2];
            wrapper_size(values);
            retval.X = values[0];
            retval.Y = values[1];
            firstrun = false;
        }

        return retval;
    }
}


extern int real_main(int argc, char *argv[]);

void irrlicht_main() {
	static const char *args[] = {
		PROJECT_NAME,
#ifndef NDEBUG
		"--verbose",
#else
		"",
#endif
	};
	real_main(2, (char**) args);
}
