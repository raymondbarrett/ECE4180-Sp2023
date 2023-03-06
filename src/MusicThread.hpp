/// \file MusicThread.hpp
/// \date 2023-03-04
/// \author mshakula (mshakula3)
///
/// \brief THe music playing thread broken out into a different file to
/// declutter main.

#ifndef MUSIC_PLAYER_THREAD_HPP
#define MUSIC_PLAYER_THREAD_HPP

#ifndef __cplusplus
#error "MusicThread.hpp is a cxx-only header."
#endif // __cplusplus

// ======================= Public Interface ==========================

namespace MusicThread {

/// \brief Thread function to play an audio file from a file.
///
/// \note As the sampling frequency of the file increases, the time drift of the
/// music player is delayed. It will play notes at their correct frequencies and
/// everything (calibrated to do so), however it will have significant slowdown
/// due to "crunchiness".
///
/// \param p a pointer to a `struct Params`.
void
main(const void* parameters);

/// \brief The parameters to pass into this thread's main function.
struct Params
{
  /// \brief The file name on an accessible mounted location.
  const char* file_name;

  /// \brief The seeking speed while playing the file.
  /// \note Faster seeking may introduce "crunchiness".
  double speed;
};

} // namespace MusicThread

// ===================== Detail Implementation =======================

#endif // MUSIC_PLAYER_THREAD_HPP
