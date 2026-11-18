#pragma once
#include <complex>
#include <cstddef>

namespace qsim {
namespace gpu {

class DeviceBuffer {
 private:
  std::complex<double>* d_amps_;
  std::size_t sizeBytes_;

 public:
  explicit DeviceBuffer(std::size_t sizeBytes);

  ~DeviceBuffer();

  // 1. Disable copy constructor
  DeviceBuffer(const DeviceBuffer&) = delete;

  // 2. Disable copy assignment operator
  DeviceBuffer& operator=(const DeviceBuffer&) = delete;

  std::complex<double>* data() { return d_amps_; }
  std::size_t getSizeBytes() const { return sizeBytes_; }
};

}  // namespace gpu
}  // namespace qsim
