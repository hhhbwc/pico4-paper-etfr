#pragma once
#include <cstdint>
#include <vector>
namespace paper_bridge {
struct PupilObservation { float x=0, y=0, radius=0, confidence=0; bool valid=false; };
class PupilDetector {
 public:
  PupilObservation Detect(const uint8_t* gray, int width, int height) const;
};
}
