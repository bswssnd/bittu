**bittu** is the first Discord Bot free of all [TCD](https://github.com/TheCodingDen) [influences](https://github.com/sedmelluq/lavaplayer),
utilising [FFmpeg APIs](http://ffmpeg.org/doxygen/trunk/) to be able to decode almost all audio codecs that exist being superior to any bot
with minimal overhead.

The performance is best percieved when the input is [OPUS](https://opus-codec.org/), where it demuxes the resource and encapsulates in a RTP stream
introducing no generation-loss and reducing overhead when using services like YouTube.
