#!/bin/sh

rustc -C opt-level=3 base64.rs -o base64 && ./base64
