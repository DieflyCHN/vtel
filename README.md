# vtel

`vtel` is a CLI-based virtual telegram simulator over UDP. It is currently a
POSIX-oriented prototype for Unix/Linux/macOS systems.

The project intentionally avoids Windows support for now. The sender and
receiver use normal POSIX terminal and UDP socket APIs.

## Build

```sh
make
```

Clean generated binary:

```sh
make clean
```

## Usage

Start the receiver on one machine:

```sh
./vtel listen 45678
```

Start the sender on another machine, using the receiver machine's IP address:

```sh
./vtel send 192.168.1.23 45678
```

The sender tick defaults to `50ms`. It can be adjusted with `-t`:

```sh
./vtel send 192.168.1.23 45678 -t 100
```

Allowed tick range: `10-5000ms`.

## Sender Controls

The current keyboard model is optimized for ordinary terminal input:

- Up arrow: dot, sends one `1` tick
- Down arrow: dash, sends three `1` ticks
- Idle: sends `0` ticks continuously
- `q`: quit sender

The sender also shows a local Morse reference table and a local keying line so
the operator can see the outgoing dot/dash sequence.

## Receiver Display

The receiver listens for UDP packets containing ASCII `0` and `1` bytes.

In an interactive terminal, it displays separate areas:

- `Morse`: received Morse dot/dash stream
- `Text`: decoded text
- Morse reference table

When stdout is redirected, it uses a linear output format, for example:

```text
...[S] ---[O] ...[S] /
```

## Current Protocol Model

The wire format is intentionally simple:

- `1`: mark / key down
- `0`: space / key up

The sender currently synthesizes this stream from arrow keys:

- dot: `1`
- dash: `111`
- idle gap: repeated `0`

The receiver reconstructs runs of `1` and `0`:

- `1` run shorter than 3 ticks: dot
- `1` run of 3 or more ticks: dash
- `0` run shorter than 6 ticks: intra-letter gap
- `0` run of 6-23 ticks: letter gap
- `0` run of 24 or more ticks: word gap

With the default `50ms` tick, this means:

- Letter gap starts around `300ms`
- Word gap starts around `1200ms`

## Implementation Notes

Files:

- `main.c`: command parsing and argument validation
- `sender.c`: terminal input, local keying display, UDP bit sending
- `receiver.c`: UDP listener and bit stream dispatch
- `decoder.c`: Morse run-length decoding, text decoding, display rendering
- `decoder.h`, `sender.h`, `receiver.h`: module interfaces
- `makefile`: simple GCC build

The receiver already supports International Morse decoding for letters, digits,
and common punctuation.

## Known Limitations

- This is not a real keydown/keyup terminal input system. Ordinary terminals do
  not expose reliable key release events.
- The current sender uses two keys for dot/dash because that is more reliable
  with normal keyboards than long-press detection.
- UDP delivery is best-effort. There is no packet loss recovery, sequencing, or
  timing correction yet.
- Tick timing is controlled by the sender. The receiver currently assumes a
  compatible tick rate and fixed thresholds.
- Receiver output buffers are bounded and currently stop appending when full.
- Receiver has no graceful quit command yet.

## Planned Work

- Dynamic timing calibration for different operators and tick values.
- Better adaptive decoding of dot/dash and letter/word gaps.
- Optional sound output for received marks.
- Optional sound or visual feedback for sent marks.
- Cleaner terminal UI, possibly with a lightweight POSIX terminal library.
- Chinese telegraph support, including Chinese Standard Telegraph Code
  decoding.
- Better message/session framing.
- UDP robustness features such as sequence numbers, loss detection, and
  optional retransmission modes.
- Tests for Morse decoding and argument parsing.

