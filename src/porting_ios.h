#ifndef __PORTING__IOS_H__
#define __PORTING__IOS_H__

#ifndef __IOS__
#error This file should only be included on iOS
#endif

namespace porting {
    void initializePathsiOS();
    void copyAssets();
}

#endif
