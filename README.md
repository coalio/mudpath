## mudpath

Mudpath is a Mad Pod bot for Codingame. Currently adapted for Gold League.

## Pre-requisites

You'll need a Linux environment to compile `sourcer.cpp` due to the usage of the `<filesystem>` header. You can use WSL(2) if you're under Windows.

## Getting started

Run `g++ --std=c++17 sourcer.cpp -o sourcer && ./sourcer` to compile the sources into a single file.
The resulting file will be called `mudpath.cpp`, copy this file into your Codingame project and run.

Update the [`LEAGUE`](https://github.com/coalio/mudpath/blob/e49ebfba41487cf31eaf90affa8b3c7d17eefcf7/sources/bot.cpp#L8) macro in bot.cpp to your current league.
