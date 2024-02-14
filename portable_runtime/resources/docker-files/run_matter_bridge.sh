#!/bin/bash

# Maintain the same order: first chip-tool commissioning command followed by starting matter bridge.
# As the container would exit as soon as chip-tool command is done incase the order is not followed.
(./chip-tool pairing onnetwork 1 2021 &) && (uic-mb --conf /etc/uic/uic.cfg "$@")