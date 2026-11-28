#pragma once
#include <complex>
#include <cstddef>

namespace qsim {
namespace gpu {

class DeviceBuffer {
 public:
  explicit DeviceBuffer(std::size_t numAmps);

  ~DeviceBuffer();

  DeviceBuffer(const DeviceBuffer&) = delete;

  DeviceBuffer& operator=(const DeviceBuffer&) = delete;

  DeviceBuffer(DeviceBuffer&& other) noexcept;

  DeviceBuffer& operator=(DeviceBuffer&& other) noexcept;

  std::complex<double>* data() { return d_amps_; }
  std::size_t getSize() const { return size_; }
  std::size_t getSizeBytes() const {
    return size_ * sizeof(std::complex<double>);
  }

 private:
  std::complex<double>* d_amps_;
  std::size_t size_;
};

}  // namespace gpu
}  // namespace qsim
