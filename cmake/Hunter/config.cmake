hunter_config(CURL VERSION ${HUNTER_CURL_VERSION} CMAKE_ARGS HTTP_ONLY=ON CMAKE_USE_OPENSSL=OFF CMAKE_USE_LIBSSH2=OFF CURL_CA_PATH=none)
hunter_config(
    Boost
    VERSION 1.79.0_new_url
    SHA1 28b4c71b7d9b8e323d40748f14e5c6d390e19720
    URL https://boostorg.jfrog.io/artifactory/main/release/1.79.0/source/boost_1_79_0.tar.gz
    CMAKE_ARGS USE_CONFIG_FROM_BOOST=ON Boost_USE_STATIC_LIBS=OFF Boost_USE_STATIC_RUNTIME=OFF
)
