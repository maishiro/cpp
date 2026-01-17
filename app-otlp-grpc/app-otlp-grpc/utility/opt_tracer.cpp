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
