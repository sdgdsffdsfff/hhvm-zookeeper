
FIND_PATH(LIBZOOKEEPER_LIBRARY_INCLUDE_DIR zookeeper/zookeeper.h)

find_library(LIBZOOKEEPER_LIBRARY NAMES zookeeper_mt)

include_directories(${LIBZOOKEEPER_INCLUDE_DIR})

HHVM_EXTENSION(zookeeper ext_zookeeper.cpp)
HHVM_SYSTEMLIB(zookeeper ext_zookeeper.php)

target_link_libraries(zookeeper  ${LIBZOOKEEPER_LIBRARY})