set(SOURCES
	PoolURI.cpp PoolURI.h
	PoolClient.h
	PoolManager.h PoolManager.cpp
	testing/SimulateClient.h testing/SimulateClient.cpp
	stratum/EthStratumClient.h stratum/EthStratumClient.cpp
	getwork/EthGetworkClient.h getwork/EthGetworkClient.cpp
)

# Emulate HunterGate:
# * https://github.com/hunter-packages/gate
include("../common.cmake")

project(download-openssl)

hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)

add_library(poolprotocols ${SOURCES})
target_link_libraries(poolprotocols PRIVATE devcore coreminer-buildinfo ethash::ethash Boost::system jsoncpp_lib_static OpenSSL::SSL OpenSSL::Crypto)
target_include_directories(poolprotocols PRIVATE ..)
