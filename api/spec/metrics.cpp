/**
 * MTS Router — Prometheus Metrics Collector Implementation
 */

#include "metrics.h"
#include <sstream>
#include <algorithm>
#include <iomanip>

namespace mts {
namespace metrics {

// ============================================================
// Counter Implementation
// ============================================================
Counter::Counter(std::string name)
    : name_(std::move(name)), value_(0) {}

void Counter::Increment(uint64_t amount) {
    value_.fetch_add(amount, std::memory_order_relaxed);
}

uint64_t Counter::Get() {
    return value_.load(std::memory_order_relaxed);
}

std::string Counter::GetName() const {
    return name_;
}

std::string Counter::ToPrometheusString() const {
    std::ostringstream oss;
    oss << name_ << " " << value_.load(std::memory_order_relaxed);
    return oss.str();
}

// ============================================================
// Gauge Implementation
// ============================================================
Gauge::Gauge(std::string name)
    : name_(std::move(name)), value_(0.0) {}

void Gauge::Set(double value) {
    std::lock_guard<std::mutex> lock(mutex_);
    value_ = value;
}

void Gauge::Increment(double amount) {
    std::lock_guard<std::mutex> lock(mutex_);
    value_ += amount;
}

void Gauge::Decrement(double amount) {
    std::lock_guard<std::mutex> lock(mutex_);
    value_ -= amount;
}

double Gauge::Get() {
    std::lock_guard<std::mutex> lock(mutex_);
    return value_;
}

std::string Gauge::GetName() const {
    return name_;
}

std::string Gauge::ToPrometheusString() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ostringstream oss;
    oss << name_ << " " << std::fixed << std::setprecision(1) << value_;
    return oss.str();
}

// ============================================================
// Histogram Implementation
// ============================================================
Histogram::Histogram(std::string name)
    : name_(std::move(name)), sum_(0.0), count_(0) {}

void Histogram::Observe(double value) {
    std::lock_guard<std::mutex> lock(mutex_);
    sum_ += value;
    count_++;
}

double Histogram::GetSum() {
    std::lock_guard<std::mutex> lock(mutex_);
    return sum_;
}

uint64_t Histogram::GetCount() {
    std::lock_guard<std::mutex> lock(mutex_);
    return count_;
}

double Histogram::GetMean() {
    std::lock_guard<std::mutex> lock(mutex_);
    return count_ > 0 ? sum_ / count_ : 0.0;
}

std::string Histogram::GetName() const {
    return name_;
}

std::string Histogram::ToPrometheusString() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ostringstream oss;
    oss << name_ << "_sum " << sum_ << "\n"
        << name_ << "_count " << count_ << "\n"
        << name_ << "_mean " << std::fixed << std::setprecision(2)
        << (count_ > 0 ? sum_ / count_ : 0.0);
    return oss.str();
}

// ============================================================
// MetricsRegistry Implementation
// ============================================================
MetricsRegistry& MetricsRegistry::GetInstance() {
    static MetricsRegistry instance;
    return instance;
}

std::shared_ptr<Counter> MetricsRegistry::CreateCounter(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto counter = std::make_shared<Counter>(name);
    counters_[name] = counter;
    return counter;
}

std::shared_ptr<Gauge> MetricsRegistry::CreateGauge(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto gauge = std::make_shared<Gauge>(name);
    gauges_[name] = gauge;
    return gauge;
}

std::shared_ptr<Histogram> MetricsRegistry::CreateHistogram(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto histogram = std::make_shared<Histogram>(name);
    histograms_[name] = histogram;
    return histogram;
}

std::string MetricsRegistry::GetAllPrometheusMetrics() const {
    std::ostringstream oss;
    std::lock_guard<std::mutex> lock(mutex_);

    for (const auto& [name, counter] : counters_) {
        oss << counter->ToPrometheusString() << "\n";
    }
    for (const auto& [name, gauge] : gauges_) {
        oss << gauge->ToPrometheusString() << "\n";
    }
    for (const auto& [name, histogram] : histograms_) {
        oss << histogram->ToPrometheusString() << "\n";
    }

    return oss.str();
}

void MetricsRegistry::ExportToHttpServer(int port) {
    // TODO: Implement HTTP server for /metrics endpoint
    // This would use a lightweight HTTP library or gRPC health check
    // For now, just log the metrics
}

} // namespace metrics
} // namespace mts
