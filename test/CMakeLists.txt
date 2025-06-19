
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${TEST_DIR}")

add_executable(testCore testCore.cpp) 
target_link_libraries(testCore PUBLIC HH::core)

# add_executable(testSerial testSerial.cpp)
# target_link_libraries(testSerial PUBLIC HH::core)

# add_executable(json_obj json_obj.cpp)
# target_link_libraries(json_obj PUBLIC reqs)

# add_executable(testPositionSensor testPositionSensor.cpp)
# target_link_libraries(testPositionSensor PUBLIC commIO::CANOpen reqs)

# add_executable(testCanOpenIO testCanOpenIO.cpp)
# target_link_libraries(testCanOpenIO PRIVATE commIO::CANOpen reqs)

# add_executable(testEPOS testEPOS.cpp)
# target_link_libraries(testEPOS PUBLIC commIO::CANOpen deviceIO::EPOS reqs)

