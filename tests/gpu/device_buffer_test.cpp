#include "qsim/gpu/device_buffer.hpp"

#include <complex>
#include <cstddef>
#include <stdexcept>
#include <utility>

#include "gtest/gtest.h"

TEST(DeviceBuffer, ConstructsCorrectly) {
  const std::size_t numAmps = std::size_t{1} << 8;

  qsim::gpu::DeviceBuffer d_buffer(numAmps);

  EXPECT_EQ(d_buffer.getSize(), numAmps);
  EXPECT_EQ(d_buffer.getSizeBytes(), numAmps * sizeof(std::complex<double>));
  EXPECT_NE(d_buffer.data(), nullptr);
}

TEST(DeviceBuffer, MoveOperation) {
  const std::size_t numAmps = std::size_t{1} << 8;
  const std::size_t sizeBytes = numAmps * sizeof(std::complex<double>);

  qsim::gpu::DeviceBuffer d_bufferOld(numAmps);

  std::complex<double>* ptr = d_bufferOld.data();

  qsim::gpu::DeviceBuffer d_bufferNew = std::move(d_bufferOld);

  EXPECT_EQ(d_bufferNew.data(), ptr);
  EXPECT_EQ(d_bufferNew.getSize(), numAmps);
  EXPECT_EQ(d_bufferNew.getSizeBytes(), sizeBytes);

  EXPECT_EQ(d_bufferOld.data(), nullptr);
  EXPECT_EQ(d_bufferOld.getSize(), std::size_t{0});
  EXPECT_EQ(d_bufferOld.getSizeBytes(), std::size_t{0});
}

TEST(DeviceBuffer, MoveAssignOperation) {
  const std::size_t numAmpsOld = std::size_t{1} << 8;

  const std::size_t numAmpsNew = std::size_t{1} << 12;

  qsim::gpu::DeviceBuffer d_bufferOld(numAmpsOld);
  qsim::gpu::DeviceBuffer d_bufferNew(numAmpsNew);

  std::complex<double>* ptr = d_bufferOld.data();
  const std::size_t size = d_bufferOld.getSize();
  const std::size_t sizeBytes = d_bufferOld.getSizeBytes();

  d_bufferNew = std::move(d_bufferOld);

  EXPECT_EQ(d_bufferNew.data(), ptr);
  EXPECT_EQ(d_bufferNew.getSize(), size);
  EXPECT_EQ(d_bufferNew.getSizeBytes(), sizeBytes);

  EXPECT_EQ(d_bufferOld.data(), nullptr);
  EXPECT_EQ(d_bufferOld.getSize(), std::size_t{0});
  EXPECT_EQ(d_bufferOld.getSizeBytes(), std::size_t{0});
}

TEST(DeviceBuffer, InvalidCreationSize) {
  const std::size_t invalidNumAmps =
      std::size_t{1} << 50;  // Too many Amps to fit on our T4 GPU

  EXPECT_THROW(qsim::gpu::DeviceBuffer d_buffer(invalidNumAmps),
               std::runtime_error);
}
