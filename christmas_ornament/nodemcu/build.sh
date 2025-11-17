#!/bin/bash
# Convenience wrapper script for build_web.py
# Usage: ./build.sh

cd "$(dirname "$0")"
python3 build_web.py

