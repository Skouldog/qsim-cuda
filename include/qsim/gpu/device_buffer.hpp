#pragma once
#include <complex>
#include <cstddef>
#include <utility>

namespace qsim {
namespace gpu {

class DeviceBuffer {
 private:
  std::complex<double>* d_amps_;
  std::size_t sizeBytes_;

 public:
  explicit DeviceBuffer(std::size_t sizeBytes);

  // 5. Destructor
  ~DeviceBuffer();

  // 1. Disable copy constructor
  DeviceBuffer(const DeviceBuffer&) = delete;

  // 2. Disable copy assignment operator
  DeviceBuffer& operator=(const DeviceBuffer&) = delete;

  // 3. Move Constructor DeviceBuffer b1 = move(b2)

  DeviceBuffer(DeviceBuffer&& other) noexcept
      : d_amps_(other.d_amps_), sizeBytes_(other.sizeBytes_) {
    other.d_amps_ = nullptr;
    other.sizeBytes_ = 0;
  }

  // 4. Move assignment b1 = move(b2)
  DeviceBuffer& operator=(DeviceBuffer&& other) noexcept;

  std::complex<double>* data() { return d_amps_; }
  std::size_t getSizeBytes() const { return sizeBytes_; }
};

}  // namespace gpu
}  // namespace qsim
