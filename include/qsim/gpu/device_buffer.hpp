#pragma once
#include <complex>
#include <cstddef>

namespace qsim {
namespace gpu {

class DeviceBuffer {
 private:
  std::complex<double>* d_amps_;
  std::size_t size_;

 public:
  explicit DeviceBuffer(std::size_t size);

  // 5. Destructor
  ~DeviceBuffer();

  // 1. Disable copy constructor
  DeviceBuffer(const DeviceBuffer&) = delete;

  // 2. Disable copy assignment operator
  DeviceBuffer& operator=(const DeviceBuffer&) = delete;

  // 3. Move Constructor DeviceBuffer b1 = move(b2)

  DeviceBuffer(DeviceBuffer&& other) noexcept
      : d_amps_(other.d_amps_), size_(other.size_) {
    other.d_amps_ = nullptr;
    other.size_ = 0;
  }

  // 4. Move assignment b1 = move(b2)
  DeviceBuffer& operator=(DeviceBuffer&& other) noexcept;

  std::complex<double>* data() { return d_amps_; }
  std::size_t getSize() const { return size_; }
  std::size_t getSizeBytes() const {
    return size_ * sizeof(std::complex<double>);
  }
};

}  // namespace gpu
}  // namespace qsim
