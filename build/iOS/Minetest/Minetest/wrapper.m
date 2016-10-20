#import <Foundation/Foundation.h>
#import <SSZipArchive/SSZipArchive.h>
#include "wrapper.h"

void wrapper_NSLog(const char *message)
{
    NSLog(@"%s", message);
}

void wrapper_paths(int type, char *dest, size_t destlen)
{
    NSArray *paths;

    if (type == WRAPPER_DOCUMENTS)
        paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    else if (type == WRAPPER_LIBRARY_SUPPORT || type == WRAPPER_LIBRARY_CACHE)
        paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
    else
        return;

    NSString *path = paths.firstObject;
    const char *path_c = path.UTF8String;

    if (type == WRAPPER_DOCUMENTS)
        snprintf(dest, destlen, "%s", path_c);
    else if (type == WRAPPER_LIBRARY_SUPPORT)
        snprintf(dest, destlen, "%s/Application Support", path_c);
    else // type == WRAPPER_LIBRARY_CACHE
        snprintf(dest, destlen, "%s/Caches", path_c);
}

void wrapper_assets()
{
    char buf[256];
    wrapper_paths(WRAPPER_LIBRARY_SUPPORT, buf, sizeof(buf));
    NSString *destpath = [NSString stringWithUTF8String:buf];
    NSString *zippath = [[NSBundle mainBundle] pathForResource:@"assets" ofType:@"zip"];

    NSLog(@"Assets found in %@", zippath);
    NSLog(@"Extracting to %@", destpath);
    [SSZipArchive unzipFileAtPath:zippath toDestination:destpath];
}
