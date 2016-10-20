#import <Foundation/Foundation.h>
#include "wrapper.h"

void wrapper_NSLog(const char *message)
{
    NSLog(@"%s", message);
}
