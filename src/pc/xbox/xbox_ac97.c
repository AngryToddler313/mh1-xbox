/* AC97 output for the Xbox: our replacement for nxdk's lib/hal/audio.c (same API,
 * hal/audio.h; nxdk's SDL2 audio driver calls it). Defining XAudioInit/Play/Pause/
 * ProvideSamples here keeps the linker from pulling nxdk's audio.obj out of its library.
 *
 * Why (docs/xbox.md "Audio after a quit and relaunch"): nxdk's driver calls back (SDL's
 * "a buffer is free") only when the PCM-out and the S/PDIF-out DMA have both finished a
 * buffer, and after a "last valid buffer" interrupt it ignores completions until the next
 * interrupt. If the S/PDIF channel does not run (xemu does not play it; on the real Xbox
 * after a quit + relaunch without a power cycle it was left in an unknown state) or the
 * PCM DMA ran dry once, no callback ever comes: the SDL audio thread waits forever and the
 * game is silent ("0 frames played"). Here:
 * - the analog PCM-out channel alone drives the callbacks, one per finished buffer
 *   (counted, so two completions before the DPC runs are two callbacks);
 * - S/PDIF gets the same buffers but is never waited on;
 * - a channel that halted (ran dry) is restarted when new samples come;
 * - init stops both DMA channels and the interrupt from a previous init (SDL's close
 *   calls XAudioInit again), cold-resets the AC-link, resets both bus masters and clears
 *   their status; xbox_ac97_stop() stops the DMA before a quit to the dashboard. */
#include <stdbool.h>
#include <string.h>
#include <hal/audio.h>
#include <xboxkrnl/xboxkrnl.h>

#define AUDIO_IRQ 6
/* nForce ACI bus master registers (byte offsets from 0xFEC00000) */
#define PO_BDBAR 0x110          /* PCM out: descriptor list base */
#define PO_LVI   0x115          /* last valid index */
#define PO_SR    0x116          /* status: 0 DCH halted, 2 LVBCI, 3 BCIS, 4 FIFOE */
#define PO_CR    0x11B          /* control: 0 run, 1 reset, 2 LVBIE, 3 IOCE, 4 FEIE */
#define SP_BDBAR 0x170          /* S/PDIF out, the same layout */
#define SP_LVI   0x175
#define SP_SR    0x176
#define SP_CR    0x17B
#define GLOB_CNT 0x12C          /* bit 1: AC-link cold reset (0 = in reset) */
#define GLOB_STA 0x130          /* bit 8: primary codec ready */
#define CR_RUN   0x1D           /* run + interrupts on completion, last buffer, FIFO error */
#define CR_PAUSE 0x1C

AC97_DEVICE ac97Device;
static KINTERRUPT irq_obj;
static KDPC dpc_obj;
static bool irq_connected, playing, dpc_ready;
static volatile LONG done_pending;      /* finished PCM-out buffers not yet called back */

static volatile unsigned char *mmio8(void) { return (volatile unsigned char *)0xFEC00000; }
static volatile unsigned int *mmio32(int off) { return (volatile unsigned int *)(0xFEC00000 + off); }

static void __stdcall dpc(PKDPC d, PVOID ctx, PVOID a1, PVOID a2)
{
    (void)d; (void)ctx; (void)a1; (void)a2;
    while (done_pending > 0) {
        InterlockedDecrement(&done_pending);
        if (ac97Device.callback)
            ac97Device.callback(&ac97Device, ac97Device.callbackData);
    }
}

static BOOLEAN __stdcall isr(PKINTERRUPT intr, PVOID ctx)
{
    volatile unsigned char *pb = mmio8();
    unsigned char a = pb[PO_SR] & 0x1C, s = pb[SP_SR] & 0x1C;
    (void)intr; (void)ctx;
    if (!a && !s)
        return FALSE;
    if (a)
        pb[PO_SR] = a;                  /* write-1-to-clear */
    if (s)
        pb[SP_SR] = s;
    if (a & 0x08) {                     /* a PCM-out buffer finished */
        InterlockedIncrement(&done_pending);
        KeInsertQueueDpc(&dpc_obj, NULL, NULL);
    }
    return TRUE;
}

static void bm_reset(int cr, int sr)
{
    volatile unsigned char *pb = mmio8();
    int n = 0;
    pb[cr] = 0;                         /* stop (and no interrupts) */
    pb[cr] = 0x02;                      /* reset the bus master */
    while ((pb[cr] & 0x02) && ++n < 1000000)
        ;
    pb[sr] = 0x1E;                      /* clear any status left */
}

/* stop both DMA channels (before a quit to the dashboard: nothing left running into the
 * next program), interrupts off */
void xbox_ac97_stop(void)
{
    volatile unsigned char *pb = mmio8();
    pb[PO_CR] = 0;
    pb[SP_CR] = 0;
    pb[PO_SR] = 0x1E;
    pb[SP_SR] = 0x1E;
    if (irq_connected) {
        KeDisconnectInterrupt(&irq_obj);
        irq_connected = false;
    }
    playing = false;
}

void XAudioInit(int sampleSizeInBits, int numChannels, XAudioCallback callback, void *data)
{
    AC97_DEVICE *d = &ac97Device;
    LARGE_INTEGER t;
    KIRQL irql;
    ULONG vector;
    int n = 0;

    xbox_ac97_stop();
    if (dpc_ready)
        KeRemoveQueueDpc(&dpc_obj);
    done_pending = 0;
    MmLockUnlockBufferPages(d, sizeof *d, FALSE);
    d->mmio = (unsigned int *)0xFEC00000;
    d->nextDescriptor = 0;
    d->callback = callback;
    d->callbackData = data;
    d->sampleSizeInBits = sampleSizeInBits;
    d->numChannels = numChannels;
    memset((void *)d->pcmOutDescriptor, 0, sizeof d->pcmOutDescriptor);
    memset((void *)d->pcmSpdifDescriptor, 0, sizeof d->pcmSpdifDescriptor);

    *mmio32(GLOB_CNT) &= ~2u;           /* AC-link cold reset */
    t.QuadPart = -10 * 1000;            /* 1 ms */
    KeDelayExecutionThread(KernelMode, FALSE, &t);
    *mmio32(GLOB_CNT) |= 2u;
    while (!(*mmio32(GLOB_STA) & 0x100) && ++n < 1000) {
        t.QuadPart = -10 * 1000;
        KeDelayExecutionThread(KernelMode, FALSE, &t);
    }

    bm_reset(PO_CR, PO_SR);
    bm_reset(SP_CR, SP_SR);
    *mmio32(0x100) = 0;                 /* no PCM in */
    *mmio32(PO_BDBAR) = MmGetPhysicalAddress((PVOID)d->pcmOutDescriptor);
    *mmio32(SP_BDBAR) = MmGetPhysicalAddress((PVOID)d->pcmSpdifDescriptor);

    vector = HalGetInterruptVector(AUDIO_IRQ, &irql);
    KeInitializeDpc(&dpc_obj, dpc, NULL);
    dpc_ready = true;
    KeInitializeInterrupt(&irq_obj, isr, NULL, vector, irql, LevelSensitive, FALSE);
    if (callback) {                     /* (SDL's close calls this with none: stay stopped) */
        KeConnectInterrupt(&irq_obj);
        irq_connected = true;
        XAudioPause();
    }
}

void XAudioPlay(void)
{
    mmio8()[PO_CR] = CR_RUN;
    mmio8()[SP_CR] = CR_RUN;
    playing = true;
}

void XAudioPause(void)
{
    mmio8()[PO_CR] = CR_PAUSE;
    mmio8()[SP_CR] = CR_PAUSE;
    playing = false;
}

void XAudioProvideSamples(unsigned char *buffer, unsigned short bufferLength, int isFinal)
{
    AC97_DEVICE *d = &ac97Device;
    volatile unsigned char *pb = mmio8();
    unsigned short ctl = 0x8000 | (isFinal ? 0x4000 : 0);
    unsigned int addr = MmGetPhysicalAddress((PVOID)buffer);
    int i = d->nextDescriptor;

    d->pcmOutDescriptor[i].bufferStartAddress = addr;
    d->pcmOutDescriptor[i].bufferLengthInSamples = bufferLength / 2;
    d->pcmOutDescriptor[i].bufferControl = ctl;
    d->pcmSpdifDescriptor[i].bufferStartAddress = addr;
    d->pcmSpdifDescriptor[i].bufferLengthInSamples = bufferLength / 2;
    d->pcmSpdifDescriptor[i].bufferControl = ctl;
    pb[PO_LVI] = (unsigned char)i;
    pb[SP_LVI] = (unsigned char)i;
    d->nextDescriptor = (unsigned char)((i + 1) % 32);
    if (playing) {                      /* a channel that ran dry halted at its last buffer: go on */
        if (pb[PO_SR] & 0x01)
            pb[PO_CR] = CR_RUN;
        if (pb[SP_SR] & 0x01)
            pb[SP_CR] = CR_RUN;
    }
}
