include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

add_executable(tests
    test_event_queue.cpp
    test_latency_model.cpp
    test_matching.cpp
    test_portfolio.cpp
    test_simulator.cpp
    test_feed_generator.cpp
)
target_link_libraries(tests PRIVATE backtester Catch2::Catch2WithMain)
add_test(NAME BacktesterTests COMMAND tests)