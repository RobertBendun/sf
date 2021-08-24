# Collection of useful single-file libraries

# C++

### [profiler.hh](./profiler.hh) - simple profiler

Simple profiling tool. Requires C++17. Example usage:

```cpp
#include <thread>
#include <chrono>

using namespace std::literals::chrono_literals;

void sleep1s()
{
  [[maybe_unused]] auto _ = profiler::section_guard("sleep1s function");
  std::this_thread::sleep_for(1s);
}

int main()
{
  profiler::time_section("Main");

  profiler::time_subsection("Sleep 1.5 seconds");
  sleep1s();

  profiler::section_start("Sleep 0.25s");
  	std::this_thread::sleep_for(0.25s);
  profiler::section_end();

  profiler::time_subsection("Sleep 0.25s again", 2);
  std::this_thread::sleep_for(0.25s);

  profiler::time_subsection("Sleep 0.5s");
  std::this_thread::sleep_for(0.5s);

  profiler::print_summary();
}
```

## JavaScript
### [ppm.js](./ppm.js) - save canvas as image

Save canvas in simple image format [PPM](http://davis.lbl.gov/Manuals/NETPBM/doc/ppm.html).

### [yuv4mpeg2.js](./yuv4mpeg2.js) - save canvas as video file

Save canvas using simple video format [YUV4MPEG2](https://wiki.multimedia.cx/index.php?title=YUV4MPEG2). Format is supported by `ffmpeg`, `vlc`, `mpv`, and much more. It does not have any compression so videos must be short in rather low resolution.
