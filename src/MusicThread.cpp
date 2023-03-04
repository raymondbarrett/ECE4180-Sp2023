/// \file MusicThread.cpp
/// \date 2023-03-04
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief The MusicThread implementation.

#include "MusicThread.hpp"

#include <cstdint>

#include <mbed.h>
#include <rtos.h>

#include "hardware.hpp"

// Huge Audio buffer.
#define AUDIO_BUF_BANK_SIZE (2 << 10)
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
std::int32_t audio_buf[AUDIO_BUF_BANK_COUNT][AUDIO_BUF_BANK_SIZE];

/// \brief The callback functor type for when the DMA encounters an error.
struct ErrorCallback_
{
  void operator()() { error("Error in DMA Callback\n\r"); }
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
    curr_bank = (curr_bank + 1) % AUDIO_BUF_BANK_COUNT;
    DMA.Disable((MODDMA::CHANNELS)DMA.getConfig()->channelNum());
    DMA.Prepare(&bank_conf[curr_bank]);
    if (DMA.irqType() == MODDMA::TcIrq)
      DMA.clearTcIrq();
    osSignalSet(tid, DATA_NEEDED_SIGNAL);
  }
};

/// \brief Helper to read into audio buffer from file.
///
/// \return 0 on success, 1 on failure.
int
readBuffer_(FILE* fp, bool& more, std::int32_t* buffer)
{
#if defined(RAW_DMA) && RAW_DMA
  std::size_t read_ct =
    std::fread(buffer, sizeof(*buffer), AUDIO_BUF_BANK_SIZE, fp);
#else
  std::size_t read_ct = std::fread(buffer, 1, AUDIO_BUF_BANK_SIZE, fp);
#endif
  if (read_ct < AUDIO_BUF_BANK_SIZE) {
    if (std::ferror(fp)) {
      return 1;
    } else if (std::feof(fp)) {
      std::memset(
        buffer + read_ct, 0, (AUDIO_BUF_BANK_SIZE - read_ct) * sizeof(*buffer));
      more = false;
    }
  }
#if defined(RAW_DMA) && RAW_DMA
  for (int i = read_ct - 1; i >= 0; --i)
    buffer[i] =
      (reinterpret_cast<std::uint8_t*>(buffer)[i * 4 + 1] << 8) & 0xFFC0;
#else
  for (int i = read_ct - 1; i >= 0; --i)
    buffer[i] = reinterpret_cast<std::uint8_t*>(buffer)[i] << 8;
#endif
  return 0;
}

} // namespace

// ====================== Global Definitions =========================

namespace MusicThread {

void
main(const void* p)
{
  static const int kClockFreq = configDACClock_();

  const Params* const params = static_cast<const Params*>(p);

  const char* name = params->file_name;
  bool        more = true;
  FILE*       fp;

  volatile int   curr_bank = 0;
  MODDMA_Config  bank_conf[AUDIO_BUF_BANK_COUNT];
  DataCallback_  callback_d(osThreadGetId(), curr_bank, bank_conf);
  ErrorCallback_ callback_e;

  fp = std::fopen(name, "r");
  if (!fp) {
    error("[MusicThread::main] Cannot open file %s!\n\r", name);
    return;
  }

  // Fill initial two buffer banks.
  for (int i = 0; i < AUDIO_BUF_BANK_COUNT; ++i) {
    if (readBuffer_(fp, more, audio_buf[i])) {
      error("[MusicThread::main] Error reading file %s!\n\r", name);
      goto end;
    }
  }

  debug("[MusicThread::main] Loaded initial banks.\n\r");

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

  debug("[MusicThread::main] Configured initial banks.\n\r");

  // Start DMA to DAC.
  if (!DMA.Setup(&bank_conf[0])) {
    error("[MusicThread::main] Error in initial DMA Setup()!\n\r");
    goto end;
  }

  // Configure and start DAC.
  LPC_DAC->DACCNTVAL = static_cast<std::uint16_t>(kClockFreq / 2 / 48000);
  LPC_DAC->DACCTRL |= 0xC; // Start running DAC.

  debug("[MusicThread::main] DAC enabled.\n\r");

  DMA.Enable(&bank_conf[0]);

  debug("[MusicThread::main] DMA enabled.\n\r");

  // Start audio buffering loop.
  debug("[MusicThread::main] Starting audio buffering idle loop.\n\r");
  osSignalWait(DATA_NEEDED_SIGNAL, osWaitForever);
  while (more) {
    int next_bank =
      (curr_bank - 1 + AUDIO_BUF_BANK_COUNT) % AUDIO_BUF_BANK_COUNT;
    if (readBuffer_(fp, more, audio_buf[next_bank])) {
      error("[MusicThread::main] Error fetching more from file %s!\n\r", name);
      goto end2;
    }
    osSignalWait(DATA_NEEDED_SIGNAL, osWaitForever);
  }

end2:
  LPC_DAC->DACCTRL &= ~(0xC); // Stop running DAC.
  DMA.Disable((MODDMA::CHANNELS)DMA.getConfig()->channelNum());
end:
  std::fclose(fp);
}

} // namespace MusicThread
