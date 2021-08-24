# =============================================================================
# Copyright (c) 2008 Christophe Oosterlynck <christophe.oosterlynck_AT_gmail.com>
#                    & NXP ( Philippe Teuwen <philippe.teuwen_AT_nxp.com> )
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# =============================================================================

"""Module for padding functions

padding info here: http://en.wikipedia.org/wiki/Padding_(cryptography)
"""
    
import random

PAD = 0
UNPAD = 1

def bitPadding (padData, direction, length=None):
        """Pad a string using bitPadding

            padData = raw string to pad/unpad
            direction = PAD or UNPAD
            length = amount of bytes the padded string should be a multiple of
                     (length variable is not used when unpadding)
            
            returns: (un)padded raw string
            
            A new block full of padding will be added when padding data that is
            already a multiple of the length.
            
            Example:
            =========
            >>> import padding

            >>> padding.bitPadding('test', padding.PAD, 8)
            'test\\x80\\x00\\x00\\x00'
            >>> padding.bitPadding(_,padding.UNPAD)
            'test'"""
        if direction == PAD:
            if length == None:
                raise ValueError("Supply a valid length")
            return __bitPadding(padData, length)
        elif direction == UNPAD:
            return __bitPadding_unpad(padData)
        else:
            raise ValueError("Supply a valid direction")

def __bitPadding (toPad,length):
    padded = toPad + '\x80' + '\x00'*(length - len(toPad)%length -1)
    return padded

def __bitPadding_unpad (padded):
    if padded.rstrip('\x00')[-1] == '\x80':
        return padded.rstrip('\x00')[:-1]
    else:
        return padded

def zerosPadding (padData, direction, length=None):
        """Pad a string using zerosPadding

            padData = raw string to pad/unpad
            direction = PAD or UNPAD
                        beware: padding and unpadding a string ending in 0's
                                will remove those 0's too
            length = amount of bytes the padded string should be a multiple of
                     (length variable is not used when unpadding)
            
            returns: (un)padded raw string
            
            No padding will be added when padding data that is already a
            multiple of the given length.
            
            Example:
            =========
            >>> import padding

            >>> padding.zerosPadding('12345678',padding.PAD,16)
            '12345678\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00'
            >>> padding.zerosPadding(_,padding.UNPAD)
            '12345678'"""
        if direction == PAD:
            if length == None:
                raise ValueError("Supply a valid length")
            return __zerosPadding(padData, length)
        elif direction == UNPAD:
            return __zerosPadding_unpad(padData)
        else:
            raise ValueError("Supply a valid direction")

def __zerosPadding (toPad, length):
    padLength = (length - len(toPad))%length
    return toPad + '\x00'*padLength

def __zerosPadding_unpad (padded ):
    return padded.rstrip('\x00')

def PKCS7(padData, direction, length=None):
        """Pad a string using PKCS7

            padData = raw string to pad/unpad
            direction = PAD or UNPAD
            length = amount of bytes the padded string should be a multiple of
                     (length variable is not used when unpadding)
            
            returns: (un)padded raw string
            
            A new block full of padding will be added when padding data that is
            already a multiple of the given length.
            
            Example:
            =========
            >>> import padding

            >>> padding.PKCS7('12345678',padding.PAD,16)
            '12345678\\x08\\x08\\x08\\x08\\x08\\x08\\x08\\x08'
            >>> padding.PKCS7(_,padding.UNPAD)
            '12345678'"""
        if direction == PAD:
            if length == None:
                raise ValueError("Supply a valid length")
            return __PKCS7(padData, length)
        elif direction == UNPAD:
            return __PKCS7_unpad(padData)
        else:
            raise ValueError("Supply a valid direction")


def __PKCS7 (toPad, length):
    amount = length - len(toPad)%length
    pattern = chr(amount)
    pad = pattern*amount
    return toPad + pad

def __PKCS7_unpad (padded):
    pattern = padded[-1]
    length = ord(pattern)
    #check if the bytes to be removed are all the same pattern
    if padded.endswith(pattern*length):
        return padded[:-length]
    else:
        return padded
        print('error: padding pattern not recognized')

def ANSI_X923 (padData, direction, length=None):
        """Pad a string using ANSI_X923

            padData = raw string to pad/unpad
            direction = PAD or UNPAD
            length = amount of bytes the padded string should be a multiple of
                     (length variable is not used when unpadding)
            
            returns: (un)padded raw string
            
            A new block full of padding will be added when padding data that is
            already a multiple of the given length.
            
            Example:
            =========
            >>> import padding
            
            >>> padding.ANSI_X923('12345678',padding.PAD,16)
            '12345678\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x08'
            >>> padding.ANSI_X923(_,padding.UNPAD)
            '12345678'"""
        if direction == PAD:
            if length == None:
                raise ValueError("Supply a valid length")
            return __ANSI_X923(padData, length)
        elif direction == UNPAD:
            return __ANSI_X923_unpad(padData)
        else:
            raise ValueError("Supply a valid direction")

def __ANSI_X923 (toPad, length):
    bytesToPad = length - len(toPad)%length
    trail = chr(bytesToPad)
    pattern = '\x00'*(bytesToPad -1) + trail
    return toPad + pattern

def __ANSI_X923_unpad (padded):
    length =ord(padded[-1])
    #check if the bytes to be removed are all zero
    if padded.count('\x00',-length,-1) == length - 1:
        return padded[:-length]
    else:
        print('error: padding pattern not recognized %s' % padded.count('\x00',-length,-1))
        return padded

def ISO_10126 (padData, direction, length=None):
        """Pad a string using ISO_10126

            padData = raw string to pad/unpad
            direction = PAD or UNPAD
            length = amount of bytes the padded string should be a multiple of
                     (length variable is not used when unpadding)
            
            returns: (un)padded raw string
            
            A new block full of padding will be added when padding data that is
            already a multiple of the given length.
            
            Example:
            =========
            >>> import padding

            >>> padded = padding.ISO_10126('12345678',padding.PAD,16)
            >>> padding.ISO_10126(padded,padding.UNPAD)
            '12345678'"""
        if direction == PAD:
            if length == None:
                raise ValueError("Supply a valid length")
            return __ISO_10126(padData, length)
        elif direction == UNPAD:
            return __ISO_10126_unpad(padData)
        else:
            raise ValueError("Supply a valid direction")

def __ISO_10126 (toPad, length):
    bytesToPad = length - len(toPad)%length
    randomPattern = ''.join(chr(random.randint(0,255)) for x in range(0,bytesToPad-1))
    return toPad + randomPattern + chr(bytesToPad)

def __ISO_10126_unpad (padded):
   return padded[0:len(padded)-ord(padded[-1])]

def _test():
    import doctest
    doctest.testmod()

if __name__ == "__main__":
    _test()
