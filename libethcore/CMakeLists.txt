set(SOURCES
	EthashAux.h EthashAux.cpp
	Farm.cpp Farm.h
	Miner.h Miner.cpp
)

include_directories(BEFORE ..)

add_library(ethcore ${SOURCES})
target_link_libraries(ethcore PUBLIC devcore ethash::ethash)
target_link_libraries(ethcore PUBLIC ethash-cpu)
