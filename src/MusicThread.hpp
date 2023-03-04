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

#define TH_MUSIC_PLAYER_SSIZE (2048)

// ======================= Public Interface ==========================

namespace MusicThread {

/// \brief Thread function to play an audio file from a file.
///
/// \note Can only play 32-bit little endian PCM files.
///
/// \param p a pointer to a `struct Params`.
void
main(const void* parameters);

/// \brief The parameters to pass into this thread's main function.
struct Params
{
  /// \brief The file name on an accessible mounted location.
  const char* file_name;
};

} // namespace MusicThread

// ===================== Detail Implementation =======================

#endif // MUSIC_PLAYER_THREAD_HPP
