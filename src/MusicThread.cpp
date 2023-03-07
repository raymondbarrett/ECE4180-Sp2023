/// \file MusicThread.cpp
/// \date 2023-03-04
/// \author mshakula (matvey@gatech.edu)
///
/// \brief The MusicThread implementation.

// Save static code size and ram by disabling MP3 support.
#define DISABLE_MP3 1

#include "MusicThread.hpp"

#include <cstdint>
#include <cstring>

#include <mbed.h>
#include <rtos.h>

#if !defined(DISABLE_MP3) || !DISABLE_MP3
#include <minimp3_ex.h>
#endif

#include "hardware.hpp"
#include "util.hpp"

// Huge Audio buffer. 1 << 9 == 512 seems to be the limit, after which it
// becomes crunchy again. I suspect it may be due to memory architecture with
// 2kB ram sectors?
#define AUDIO_BUF_BANK_SIZE (1 << 9)
#define AUDIO_BUF_BANK_COUNT (2)
#define DATA_NEEDED_SIGNAL (0x1)

// ======================= Local Definitions =========================

namespace {

/// \brief Configure the DAC clock to be in-phase with the CPU Clock.
///
/// \return Clock speed.
int
configDACClock_()
{
  LPC_SC->PCLKSEL0 |= 0x1 << 22; // PCLK_DAC = CCLK
  return CCK_SPEED;
}

// switching audio buffer.
std::uint32_t audio_buf[AUDIO_BUF_BANK_COUNT][AUDIO_BUF_BANK_SIZE];

/// \brief The callback functor type for when the DMA encounters an error.
struct ErrorCallback_
{
  void operator()() { error("Error in DMA Callback\r\n"); }
};

/// \brief The callback functor type for when the music player DAC runs out of
/// samples.
struct DataCallback_
{
  osThreadId     tid;
  volatile int&  curr_bank;
  MODDMA_Config* bank_conf;

  DataCallback_(
    osThreadId     tid_,
    volatile int&  curr_bank_,
    MODDMA_Config* bank_conf_) :
      tid(tid_), curr_bank(curr_bank_), bank_conf(bank_conf_)
  {
  }

  void operator()()
  {
#define TICK_DIVIDER (8)
    static int i = 0;

    curr_bank = (curr_bank + 1) % AUDIO_BUF_BANK_COUNT;
    DMA.Disable((MODDMA::CHANNELS)DMA.getConfig()->channelNum());
    DMA.Prepare(&bank_conf[curr_bank]);
    if (DMA.irqType() == MODDMA::TcIrq)
      DMA.clearTcIrq();

    OnboardLEDs = (i = (i + 1) % (16 * TICK_DIVIDER)) / TICK_DIVIDER;
    osSignalSet(tid, DATA_NEEDED_SIGNAL);
#undef TICK_DIVIDER
  }
};

// clang-format off
/// \brief Supported file types.
enum FileType_
{
  FileType_Undefined = 0
, FileType_u8pcm    = 1
#if !defined(DISABLE_MP3) || !DISABLE_MP3
, FileType_mp3       = 2
#endif
};
// clang-format on

struct U8PCMFileInfo_
{
  FILE* file;

  void destroy() { std::fclose(file); }
};

#if !defined(DISABLE_MP3) || !DISABLE_MP3
struct MP3FileInfo_
{
  mp3dec_ex_t    dec;
  mp3d_sample_t* data_start; // location of next spot in buffer.
  mp3d_sample_t* data_end;

  void destroy() { mp3dec_ex_close(&dec); }
};
#endif

/// \brief Structure about a file. Very heavy.
struct FileInfo_
{
  const char* name;
  FileType_   type;
  int         rate;
  union {
    U8PCMFileInfo_ u8pcm;
#if !defined(DISABLE_MP3) || !DISABLE_MP3
    MP3FileInfo_ mp3;
#endif
  };
};

/// \brief Queries the file, and fills out file info structure.
FileType_
initFile_(const char* fname, FileInfo_& info)
{
  // Get name.
  info.name = fname;

  { // Get type.
    const char* dot = std::strrchr(fname, '.');
    if (!dot || dot == fname)
      info.type = FileType_u8pcm;
#if !defined(DISABLE_MP3) || !DISABLE_MP3
    else if (!std::strcmp(dot, ".mp3"))
      info.type = FileType_mp3;
#endif
    else
      info.type = FileType_u8pcm;
  }

  // Get rate and init decoding structs.
  switch (info.type) {

    case FileType_u8pcm: {
      info.u8pcm.file = std::fopen(info.name, "rb");
      if (!info.u8pcm.file)
        goto err;
      info.rate = 0;
    } break;

#if !defined(DISABLE_MP3) || !DISABLE_MP3
    case FileType_mp3: {
      if (mp3dec_ex_open(&info.mp3.dec, info.name, MP3D_SEEK_TO_SAMPLE))
        goto err;
      info.rate = info.mp3.dec.info.hz;
    } break;
#endif

    default:
      break;
  }

  return info.type;

err:
  info.type = FileType_Undefined;
  return info.type;
}

void
deinitFile_(FileInfo_& info)
{
  switch (info.type) {
    case FileType_u8pcm:
      info.u8pcm.destroy();
      break;

#if !defined(DISABLE_MP3) || !DISABLE_MP3
    case FileType_mp3:
      info.mp3.destroy();
      break;
#endif

    default:
      break;
  }
}

/// \brief Helper to read into audio buffer from file.
///
/// \return 0 on success, 1 on failure.
int
readBuffer_(FileInfo_& file_info, bool& more, std::uint32_t* buffer)
{
  switch (file_info.type) {
    case FileType_u8pcm: {
      FILE* const fp      = file_info.u8pcm.file;
      std::size_t read_ct = std::fread(buffer, 1, AUDIO_BUF_BANK_SIZE, fp);
      if (read_ct < AUDIO_BUF_BANK_SIZE) {
        if (std::ferror(fp)) {
          return 1;
        } else if (std::feof(fp)) {
          std::memset(
            buffer + read_ct,
            0,
            (AUDIO_BUF_BANK_SIZE - read_ct) * sizeof(*buffer));
          more = false;
        }
      }
      for (int i = read_ct - 1; i >= 0; --i)
        buffer[i] = reinterpret_cast<std::uint8_t*>(buffer)[i] << 8;
    } break;

#if !defined(DISABLE_MP3) || !DISABLE_MP3
    case FileType_mp3: {
      std::uint32_t*  b  = buffer;
      mp3d_sample_t*& ds = file_info.mp3.data_start;
      mp3d_sample_t*& de = file_info.mp3.data_end;

      // Leftover data in decode buffer.
      while (ds != de && b - buffer != AUDIO_BUF_BANK_SIZE)
        *b++ = static_cast<std::uint32_t>(*ds++) & 0xFFC0;

      if (b - buffer == AUDIO_BUF_BANK_SIZE)
        return 0;

      // Read all available data... reset.
      ds = de = file_info.mp3.dec.buffer;
      de +=
        mp3dec_ex_read(&file_info.mp3.dec, ds, MINIMP3_MAX_SAMPLES_PER_FRAME);
      if (de - ds != MINIMP3_MAX_SAMPLES_PER_FRAME) {
        if (file_info.mp3.dec.last_error) {
          return 1;
        } else { // EOF.
          std::memset(
            b + (de - ds),
            0,
            (MINIMP3_MAX_SAMPLES_PER_FRAME - (de - ds)) * sizeof(*b));
        }
      }

      while (ds != de && b - buffer != AUDIO_BUF_BANK_SIZE)
        *b++ = static_cast<std::uint32_t>(*ds++) & 0xFFC0;
    } break;
#endif

    default:
      return 1;
  }
  return 0;
}

} // namespace

// ====================== Global Definitions =========================

void
MusicThread::operator()()
{
  static const int kClockFreq = configDACClock_();

  static FileInfo_ file_info; // allocate decoding structs in static memory.
  bool             more = true;

  volatile int   curr_bank = 0;
  MODDMA_Config  bank_conf[AUDIO_BUF_BANK_COUNT];
  DataCallback_  callback_d(osThreadGetId(), curr_bank, bank_conf);
  ErrorCallback_ callback_e;

  // Open file and get its data.
  if (!initFile_(this->file_name_, file_info)) {
    error("[MusicThread::main] Cannot open file %s!\r\n", file_info.name);
    return;
  }

  // Fill initial two buffer banks.
  for (int i = 0; i < AUDIO_BUF_BANK_COUNT; ++i) {
    if (readBuffer_(file_info, more, audio_buf[i])) {
      error("[MusicThread::main] Error reading file %s!\r\n", file_info.name);
      goto end;
    }
  }

  debug("[MusicThread::main] Loaded initial banks.\r\n");

  // Configure Banks
  for (int i = 0; i < AUDIO_BUF_BANK_COUNT; ++i) {
    (&bank_conf[i])
      ->srcMemAddr(reinterpret_cast<std::uint32_t>(&audio_buf[i]))
      ->dstMemAddr(MODDMA::DAC)
      ->transferSize(sizeof(audio_buf[i]) / sizeof(audio_buf[i][0]))
      ->transferType(MODDMA::m2p)
      ->dstConn(MODDMA::DAC)
      ->attach_tc(&callback_d, &DataCallback_::operator())
      ->attach_err(&callback_e, &ErrorCallback_::operator());
  }
  bank_conf[0].channelNum(MODDMA::Channel_0);
  bank_conf[1].channelNum(MODDMA::Channel_1);

  debug("[MusicThread::main] Configured initial banks.\r\n");

  // Start DMA to DAC.
  if (!DMA.Setup(&bank_conf[0])) {
    error("[MusicThread::main] Error in initial DMA Setup()!\r\n");
    goto end;
  }

  // Configure and start DAC. Assume 24kHz for PCM (seems to work well @ this
  // speed).
  LPC_DAC->DACCNTVAL = static_cast<std::uint16_t>(
    kClockFreq / this->speed_ / 2 / (file_info.rate ? file_info.rate : 24000));
  LPC_DAC->DACCTRL |= 0xC; // Start running DAC.

  debug("[MusicThread::main] DAC enabled.\r\n");

  DMA.Enable(&bank_conf[0]);

  debug("[MusicThread::main] DMA enabled.\r\n");

  // Start audio buffering loop.
  debug("[MusicThread::main] Starting audio buffering idle loop.\r\n");
  osSignalWait(DATA_NEEDED_SIGNAL, osWaitForever);
  while (more) {
    // debug("[MusicThread::main] Fetching more from file...");
    int next_bank =
      (curr_bank - 1 + AUDIO_BUF_BANK_COUNT) % AUDIO_BUF_BANK_COUNT;
    if (readBuffer_(file_info, more, audio_buf[next_bank])) {
      error(
        "[MusicThread::main] Error fetching more from file %s!\r\n",
        file_info.name);
      goto end2;
    }
    // debug(" done.\r\n");
    osSignalWait(DATA_NEEDED_SIGNAL, osWaitForever);
  }

end2:
  LPC_DAC->DACCTRL &= ~(0xC); // Stop running DAC.
  DMA.Disable((MODDMA::CHANNELS)DMA.getConfig()->channelNum());
end:
  deinitFile_(file_info);
}

// Include minimp3 implementation
#if !defined(DISABLE_MP3) || !DISABLE_MP3
#define MINIMP3_ONLY_MP3
#define MINIMP3_NONSTANDARD_BUT_LOGICAL
#define MINIMP3_IMPLEMENTATION
#include <minimp3_ex.h>
#endif
