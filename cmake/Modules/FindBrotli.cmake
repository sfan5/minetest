mark_as_advanced(BROTLIENC_LIBRARY BROTLIDEC_LIBRARY BROTLI_LIBRARIES BROTLI_INCLUDE_DIR)

find_path(BROTLI_INCLUDE_DIR NAMES enc/encode.h dec/decode.h PATH_SUFFIXES brotli)

find_library(BROTLIENC_LIBRARY NAMES brotlienc)
find_library(BROTLIDEC_LIBRARY NAMES brotlidec)
set(BROTLI_LIBRARIES ${BROTLIENC_LIBRARY} ${BROTLIDEC_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Brotli DEFAULT_MSG BROTLI_LIBRARIES BROTLI_INCLUDE_DIR)

