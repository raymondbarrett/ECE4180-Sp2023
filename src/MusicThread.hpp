/// \file MusicThread.hpp
/// \date 2023-03-04
/// \author mshakula (mshakula3)
///
/// \brief THe music playing thread broken out into a different file to
/// declutter main.

#ifndef MUSIC_THREAD_HPP
#define MUSIC_THREAD_HPP

#ifndef __cplusplus
#error "MusicThread.hpp is a cxx-only header."
#endif // __cplusplus

#include "ThreadCommon.hpp"

// ======================= Public Interface ==========================

/// \brief Class encapsulating an audio playing function.
///
/// \note As the sampling frequency of the file increases, the time drift of the
/// music player is delayed. It will play notes at their correct frequencies and
/// everything (calibrated to do so), however it will have significant slowdown
/// due to "crunchiness".
class MusicThread : public ThreadHelper<MusicThread>
{
 public:
  /// \brief The constructor.
  ///
  /// \param file_name The file name on an accessible mounted location.
  ///
  /// \param initial_speed The initial speed at which to play the music file.
  /// Faster seeking may introduce "crunchiness". Keep it within [0, 2] to be
  /// safe usually for 24kHz pcm.
  MusicThread(const char* file_name, double initial_speed) :
      file_name_(file_name), speed_(initial_speed)
  {
  }

  /// \brief Run the actual thread.
  void operator()();

 private:
  const char* file_name_;
  double      speed_;
};

// ===================== Detail Implementation =======================

#endif // MUSIC_THREAD_HPP
