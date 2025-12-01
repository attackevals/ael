#[
  Copied from https://github.com/OHermesJunior/nimRC4
  MIT License

  Copyright (c) 2019 Hermes Junior

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
]#

import strutils
import winim/lean

proc genKeystream(key: string): array[256, int] =
  for i in 0..255:
    result[i] = i
  var j, k = 0
  for i in 0..255:
    j = (j + result[i] + ord(key[k])) mod 256
    swap(result[i], result[j])
    k = (k + 1) mod key.len

iterator iterate(keystream: var array[256, int],
                size: int, incr = 1): tuple[i, j, k: int] =
  var i, j, k = 0
  while i < size:
    j = (j + 1) mod 256
    k = (k + keystream[j]) mod 256
    swap(keystream[k], keystream[j])
    yield (i, j, k)
    i += incr

proc toRC4*(key, data: string): string =
  var keyst = genKeystream(key)

  for i, j, k in iterate(keyst, data.len):
    result.add((ord(data[i]) xor keyst[(keyst[j] +
                keyst[k]) mod 256]).toHex(2))

proc fromRC4*(key, data: string): string =
  var keyst = genKeystream(key)

  for i, j, k in iterate(keyst, data.len, 2):
    result.add((fromHex[int](data[i] & data[i+1]) xor
                keyst[(keyst[j] + keyst[k]) mod 256]).char)
