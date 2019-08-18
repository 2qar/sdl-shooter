#!/bin/bash

FLAGS=$(sdl2-config --libs --cflags)
clang $FLAGS -lSDL2_image main.c -o sdl-shooter
