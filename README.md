Bitflip
-------

This is a very simple prototype of a way to test for ransomware.
It should execute in a similar way to most ransomware attacks by modifying the contents of the files it's pointed at. In this case, it just uses a very simple algorithm - invert all the bits - so that it's trivially easy to undo the process if something's gone wrong.

Given the aim of this is to try and detect if ransomware detectors are working, there are several different modes of operation:

1) Stealth mode - bitflip in place and do nothing to the path. (default)
2) Update files in-place and rename them (adding the extension .bitflip)
3) Create new files with the .bitflip extension and writing them out fully before removing the originals

Each method should trip a ransomware detector.
