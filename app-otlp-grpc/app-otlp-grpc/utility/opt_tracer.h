#pragma once

#include <opentelemetry/trace/provider.h>
#include <opentelemetry/nostd/shared_ptr.h>

namespace trace = opentelemetry::trace;
namespace nostd = opentelemetry::nostd;

void InitTracer();
void CleanupTracer();
nostd::shared_ptr<trace::Tracer> GetTracer();
