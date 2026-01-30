#include <iostream>
#include <string>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

int main()
{
    // Console
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level( spdlog::level::trace );
    // Rolling file
    auto max_size = 1048576 * 5;
    auto max_files = 3;
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>( "logs/rotating.txt", max_size, max_files );
    file_sink->set_level( spdlog::level::trace );

    std::vector<spdlog::sink_ptr> sinks{ console_sink, file_sink };
    auto logger = std::make_shared<spdlog::logger>( "my_logger", sinks.begin(), sinks.end() );
    spdlog::set_default_logger( logger );

    logger->info( "はろーわーるど" );
    spdlog::info( "new logger log message {}", 0 );

    std::string message = std::format( "The answer is {}.", 42 );
    spdlog::info( message );
}
