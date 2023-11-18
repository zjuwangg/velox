/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#include <stdint.h>
#include <string.h>

#include <folly/executors/CPUThreadPoolExecutor.h>
#include "velox/common/compression/Compression.h"

namespace facebook::velox::common {
/// Specifies the config for spilling.
struct SpillConfig {
  SpillConfig(
      const std::string& _filePath,
      uint64_t _maxFileSize,
      uint64_t _writeBufferSize,
      uint64_t _minSpillRunSize,
      folly::Executor* _executor,
      int32_t _minSpillableReservationPct,
      int32_t _spillableReservationGrowthPct,
      uint8_t _startPartitionBit,
      uint8_t _joinPartitionBits,
      int32_t _maxSpillLevel,
      uint64_t _writerFlushThresholdSize,
      int32_t _testSpillPct,
      const std::string& _compressionKind,
      const std::unordered_map<std::string, std::string>& _writeFileOptions =
          {});

  /// Returns the hash join spilling level with given 'startBitOffset'.
  ///
  /// NOTE: we advance (or right shift) the partition bit offset when goes to
  /// the next level of recursive spilling.
  int32_t joinSpillLevel(uint8_t startBitOffset) const;

  /// Checks if the given 'startBitOffset' has exceeded the max hash join
  /// spill limit.
  bool exceedJoinSpillLevelLimit(uint8_t startBitOffset) const;

  /// Filesystem path for spill files.
  std::string filePath;

  /// The max spill file size. If it is zero, there is no limit on the spill
  /// file size.
  uint64_t maxFileSize;

  /// Specifies the size to buffer the serialized spill data before write to
  /// storage system for io efficiency.
  uint64_t writeBufferSize;

  /// The min spill run size (bytes) limit used to select partitions for
  /// spilling. The spiller tries to spill a previously spilled partitions if
  /// its data size exceeds this limit, otherwise it spills the partition with
  /// most data. If the limit is zero, then the spiller always spill a
  /// previously spilled partition if it has any data. This is to avoid spill
  /// from a partition with a small amount of data which might result in
  /// generating too many small spilled files.
  uint64_t minSpillRunSize;

  /// Executor for spilling. If nullptr spilling writes on the Driver's thread.
  folly::Executor* executor; // Not owned.

  /// The minimal spillable memory reservation in percentage of the current
  /// memory usage.
  int32_t minSpillableReservationPct;

  /// The spillable memory reservation growth in percentage of the current
  /// memory usage.
  int32_t spillableReservationGrowthPct;

  /// Used to calculate spill partition number.
  uint8_t startPartitionBit;

  /// Used to calculate the spill hash partition number for hash join with
  /// 'startPartitionBit'.
  uint8_t joinPartitionBits;

  /// The max allowed spilling level with zero being the initial spilling
  /// level. This only applies for hash build spilling which needs recursive
  /// spilling when the build table is too big. If it is set to -1, then there
  /// is no limit and then some extreme large query might run out of spilling
  /// partition bits at the end.
  int32_t maxSpillLevel;

  /// Minimum memory footprint size required to reclaim memory from a file
  /// writer by flushing its buffered data to disk.
  uint64_t writerFlushThresholdSize;

  /// Percentage of input batches to be spilled for testing. 0 means no
  /// spilling for test.
  int32_t testSpillPct;

  /// CompressionKind when spilling, CompressionKind_NONE means no compression.
  common::CompressionKind compressionKind;

  /// Custom options passed to velox::FileSystem to create spill WriteFile.
  std::unordered_map<std::string, std::string> writeFileOptions;
};
} // namespace facebook::velox::common
