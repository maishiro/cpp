#include "opt_tracer.h"
#include <opentelemetry/sdk/trace/provider.h>
#include <opentelemetry/sdk/trace/simple_processor_factory.h>
#include <opentelemetry/sdk/trace/tracer_provider_factory.h>
//#include <opentelemetry/exporters/zipkin/zipkin_exporter_factory.h>
//#include <opentelemetry/exporters/zipkin/zipkin_exporter_options.h>
#include <opentelemetry/exporters/otlp/otlp_grpc_exporter_factory.h>
#include <opentelemetry/trace/provider.h>

namespace trace = opentelemetry::trace;
namespace trace_sdk = opentelemetry::sdk::trace;
//namespace zipkin = opentelemetry::exporter::zipkin;
namespace otlp = opentelemetry::exporter::otlp;
namespace resource = opentelemetry::sdk::resource;
namespace trace = opentelemetry::trace;
namespace nostd = opentelemetry::nostd;

//zipkin::ZipkinExporterOptions opts;
otlp::OtlpGrpcExporterOptions opts;


void InitTracer()
{
    // Collector (Zipkin)  URL 例: http://localhost:9411/api/v2/spans
    //opts.endpoint = "http://localhost:9411/api/v2/spans";
    // OTLP Collector (gRPC)  URL 例: http://localhost:4317  Aspire のデフォルト OTLP gRPC
    opts.endpoint = "http://localhost:4317";
    opts.use_ssl_credentials = false;
    opts.metadata = { {"x-otlp-api-key", ""} };
    // 環境変数 OTEL_EXPORTER_OTLP_ENDPOINT を取得
    char* libvar = nullptr;
    size_t requiredSize = 0;
    getenv_s( &requiredSize, NULL, 0, "OTEL_EXPORTER_OTLP_ENDPOINT" );
    if( requiredSize != 0 )
    {
        libvar = new char[requiredSize];
		if( libvar != nullptr )
        {
            getenv_s( &requiredSize, libvar, requiredSize, "OTEL_EXPORTER_OTLP_ENDPOINT" );
            const std::string otel_endpoint = std::string( libvar );
            opts.endpoint = otel_endpoint;
            if( otel_endpoint.find( "https://" ) != std::string::npos )
            {
                opts.use_ssl_credentials = true;
            }
            else if( otel_endpoint.find( "http://" ) != std::string::npos )
            {
                opts.use_ssl_credentials = false;
            }
            delete[] libvar;
			libvar = nullptr;
        }
    }
    // 環境変数 OTEL_EXPORTER_OTLP_HEADERS を取得
    getenv_s( &requiredSize, NULL, 0, "OTEL_EXPORTER_OTLP_HEADERS" );
    if( requiredSize != 0 )
    {
        libvar = new char[requiredSize];
        if( libvar != nullptr )
        {
            getenv_s( &requiredSize, libvar, requiredSize, "OTEL_EXPORTER_OTLP_HEADERS" );
            const std::string otel_headers = std::string( libvar );
            if( !otel_headers.empty() )
            {
                size_t pos = 0;
                size_t delim_pos = otel_headers.find( '=' );
                if( delim_pos != std::string::npos )
                {
                    std::string key = otel_headers.substr( pos, delim_pos - pos );
                    std::string value = otel_headers.substr( delim_pos + 1 );
                    opts.metadata = { { key, value } };
                }
            }
            delete[] libvar;
            libvar = nullptr;
        }
    }

    // Create zipkin exporter instance
    resource::ResourceAttributes attributes = { {"service.name", "app-otlp-grpc"} };
    auto resource = resource::Resource::Create( attributes );
    //auto exporter = zipkin::ZipkinExporterFactory::Create( opts );
    auto exporter = otlp::OtlpGrpcExporterFactory::Create( opts );
    auto processor = trace_sdk::SimpleSpanProcessorFactory::Create( std::move( exporter ) );
    std::shared_ptr<opentelemetry::trace::TracerProvider> provider =
        trace_sdk::TracerProviderFactory::Create( std::move( processor ), resource );
    // Set the global trace provider
    trace_sdk::Provider::SetTracerProvider( provider );
}

void CleanupTracer()
{
    std::shared_ptr<opentelemetry::trace::TracerProvider> none;
    trace_sdk::Provider::SetTracerProvider( none );
}

nostd::shared_ptr<trace::Tracer> GetTracer()
{
    auto provider = trace::Provider::GetTracerProvider();
    return provider->GetTracer( "foo_library" );
}
