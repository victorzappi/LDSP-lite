#pragma once

#include <memory>
#include <mutex>

namespace ldsplite {

class OboeAudioEngine;

class LDSPlite {
 public:
  LDSPlite();

  ~LDSPlite();

  void start();

  void stop();

  bool isStarted() const;

 private:
  std::atomic<bool> _isStarted{false};
  std::mutex _mutex;
  std::unique_ptr<OboeAudioEngine> _audioEngine;
};
}  // namespace ldsplite
