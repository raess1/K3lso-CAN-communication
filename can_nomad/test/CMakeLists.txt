cmake_minimum_required (VERSION 3.10)

include_directories("${PROJECT_SOURCE_DIR}/include")

set(CAN_TEST_SOURCES ${PROJECT_SOURCE_DIR}/test/can_test.cpp)
set(CAN_TEST_LIBS CAN)
add_executable(can_test ${CAN_TEST_SOURCES})
target_link_libraries(can_test ${CAN_TEST_LIBS} )

set(PCAN_TEST_SOURCES ${PROJECT_SOURCE_DIR}/test/pcan_test.cpp)
set(PCAN_TEST_LIBS CAN pcanfd)
add_executable(pcan_test ${PCAN_TEST_SOURCES})
target_link_libraries(pcan_test ${PCAN_TEST_LIBS} )

set(K3LSO_TEST_TX_SOURCES ${PROJECT_SOURCE_DIR}/test/k3lso_test_tx.cpp)
set(K3LSO_TEST_TX_LIBS CAN pcanfd)
add_executable(k3lso_test_tx ${K3LSO_TEST_TX_SOURCES})
target_link_libraries(k3lso_test_tx ${K3LSO_TEST_TX_LIBS} )

set(K3LSO_TEST_RX_SOURCES ${PROJECT_SOURCE_DIR}/test/k3lso_test_rx.cpp)
set(K3LSO_TEST_RX_LIBS CAN pcanfd)
add_executable(k3lso_test_rx ${K3LSO_TEST_RX_SOURCES})
target_link_libraries(k3lso_test_rx ${K3LSO_TEST_RX_LIBS} )

set(K3LSO_TORQUE_SOURCES ${PROJECT_SOURCE_DIR}/test/k3lso_torque.cpp)
set(K3LSO_TORQUE_LIBS CAN pcanfd)
add_executable(k3lso_torque ${K3LSO_TORQUE_SOURCES})
target_link_libraries(k3lso_torque ${K3LSO_TORQUE_LIBS} )

set(K3LSO_FREQ_SOURCES ${PROJECT_SOURCE_DIR}/test/k3lso_freq.cpp)
set(K3LSO_FREQ_LIBS CAN pcanfd)
add_executable(k3lso_freq ${K3LSO_FREQ_SOURCES})
target_link_libraries(k3lso_freq ${K3LSO_FREQ_LIBS} )