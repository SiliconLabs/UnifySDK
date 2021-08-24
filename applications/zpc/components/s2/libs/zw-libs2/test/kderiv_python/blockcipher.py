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
import util
import padding

MODE_ECB = 1
MODE_CBC = 2
MODE_CFB = 3
MODE_OFB = 5
MODE_CTR = 6
MODE_XTS = 7
MODE_CMAC = 8

class BlockCipher():
    """ Base class for all blockciphers
    """

    key_error_message = "Wrong key size" #should be overwritten in child classes

    def __init__(self,key,mode,IV,counter,cipher_module,segment_size,args={}):
        # Cipher classes inheriting from this one take care of:
        #   self.blocksize
        #   self.cipher
        self.key = key
        self.mode = mode
        self.cache = ''
        self.ed = None

        if 'keylen_valid' in dir(self): #wrappers for pycrypto functions don't have this function
         if not self.keylen_valid(key) and type(key) is not tuple:
                raise ValueError(self.key_error_message)

        if IV == None:
            self.IV = '\x00'*self.blocksize
        else:
            self.IV = IV

        if mode != MODE_XTS:
            self.cipher = cipher_module(self.key,**args)
        if mode == MODE_ECB:
            self.chain = ECB(self.cipher, self.blocksize)
        elif mode == MODE_CBC:
            if len(self.IV) != self.blocksize:
                raise Exception("the IV length should be %i bytes"%self.blocksize)
            self.chain = CBC(self.cipher, self.blocksize,self.IV)
        elif mode == MODE_CFB:
            if len(self.IV) != self.blocksize:
                raise Exception("the IV length should be %i bytes"%self.blocksize)
            if segment_size == None:
                raise ValueError("segment size must be defined explicitely for CFB mode")
            if segment_size > self.blocksize*8 or segment_size%8 != 0:
                # current CFB implementation doesn't support bit level acces => segment_size should be multiple of bytes
                raise ValueError("segment size should be a multiple of 8 bits between 8 and %i"%(self.blocksize*8))
            self.chain = CFB(self.cipher, self.blocksize,self.IV,segment_size)
        elif mode == MODE_OFB:
            if len(self.IV) != self.blocksize:
                raise ValueError("the IV length should be %i bytes"%self.blocksize)
            self.chain = OFB(self.cipher, self.blocksize,self.IV)
        elif mode == MODE_CTR:
            if (counter == None) or  not callable(counter):
                raise Exception("Supply a valid counter object for the CTR mode")
            self.chain = CTR(self.cipher,self.blocksize,counter)
        elif mode == MODE_XTS:
            if self.blocksize != 16:
                raise Exception('XTS only works with blockcipher that have a 128-bit blocksize')
            if not(type(key) == tuple and len(key) == 2):
                raise Exception('Supply two keys as a tuple when using XTS')
            if 'keylen_valid' in dir(self): #wrappers for pycrypto functions don't have this function
             if not self.keylen_valid(key[0]) or  not self.keylen_valid(key[1]):
                raise ValueError(self.key_error_message)
            self.cipher = cipher_module(self.key[0],**args)
            self.cipher2 = cipher_module(self.key[1],**args)
            self.chain = XTS(self.cipher, self.cipher2)
        elif mode == MODE_CMAC:
            if self.blocksize not in (8,16):
                raise Exception('CMAC only works with blockcipher that have a 64 or 128-bit blocksize')
            self.chain = CMAC(self.cipher,self.blocksize,self.IV)
        else:
                raise Exception("Unknown chaining mode!")

    def encrypt(self,plaintext,n=''):
        """Encrypt some plaintext

            plaintext   = a string of binary data
            n           = the 'tweak' value when the chaining mode is XTS

        The encrypt function will encrypt the supplied plaintext.
        The behavior varies slightly depending on the chaining mode.

        ECB, CBC:
        ---------
        When the supplied plaintext is not a multiple of the blocksize
          of the cipher, then the remaining plaintext will be cached.
        The next time the encrypt function is called with some plaintext,
          the new plaintext will be concatenated to the cache and then
          cache+plaintext will be encrypted.

        CFB, OFB, CTR:
        --------------
        When the chaining mode allows the cipher to act as a stream cipher,
          the encrypt function will always encrypt all of the supplied
          plaintext immediately. No cache will be kept.

        XTS:
        ----
        Because the handling of the last two blocks is linked,
          it needs the whole block of plaintext to be supplied at once.
        Every encrypt function called on a XTS cipher will output
          an encrypted block based on the current supplied plaintext block.

        CMAC:
        -----
        Everytime the function is called, the hash from the input data is calculated.
        No finalizing needed.
        The hashlength is equal to block size of the used block cipher.
        """
        #self.ed = 'e' if chain is encrypting, 'd' if decrypting,
        # None if nothing happened with the chain yet
        #assert self.ed in ('e',None) 
        # makes sure you don't encrypt with a cipher that has started decrypting
        self.ed = 'e'
        if self.mode == MODE_XTS:
            # data sequence number (or 'tweak') has to be provided when in XTS mode
            return self.chain.update(plaintext,'e',n)
        else:
            return self.chain.update(plaintext,'e')

    def decrypt(self,ciphertext,n=''):
        """Decrypt some ciphertext

            ciphertext  = a string of binary data
            n           = the 'tweak' value when the chaining mode is XTS

        The decrypt function will decrypt the supplied ciphertext.
        The behavior varies slightly depending on the chaining mode.

        ECB, CBC:
        ---------
        When the supplied ciphertext is not a multiple of the blocksize
          of the cipher, then the remaining ciphertext will be cached.
        The next time the decrypt function is called with some ciphertext,
          the new ciphertext will be concatenated to the cache and then
          cache+ciphertext will be decrypted.

        CFB, OFB, CTR:
        --------------
        When the chaining mode allows the cipher to act as a stream cipher,
          the decrypt function will always decrypt all of the supplied
          ciphertext immediately. No cache will be kept.

        XTS:
        ----
        Because the handling of the last two blocks is linked,
          it needs the whole block of ciphertext to be supplied at once.
        Every decrypt function called on a XTS cipher will output
          a decrypted block based on the current supplied ciphertext block.

        CMAC:
        -----
        Mode not supported for decryption as this does not make sense.
        """
        #self.ed = 'e' if chain is encrypting, 'd' if decrypting,
        # None if nothing happened with the chain yet
        #assert self.ed in ('d',None)
        # makes sure you don't decrypt with a cipher that has started encrypting
        self.ed = 'd'
        if self.mode == MODE_XTS:
            # data sequence number (or 'tweak') has to be provided when in XTS mode
            return self.chain.update(ciphertext,'d',n)
        else:
            return self.chain.update(ciphertext,'d')

    def final(self,padfct=padding.PKCS7):
        # TODO: after calling final, reset the IV? so the cipher is as good as new?
        """Finalizes the encryption by padding the cache

            padfct = padding function
                     import from CryptoPlus.Util.padding

        For ECB, CBC: the remaining bytes in the cache will be padded and
                      encrypted.
        For OFB,CFB, CTR: an encrypted padding will be returned, making the
                          total outputed bytes since construction of the cipher
                          a multiple of the blocksize of that cipher.

        If the cipher has been used for decryption, the final function won't do
          anything. You have to manually unpad if necessary.

        After finalization, the chain can still be used but the IV, counter etc
          aren't reset but just continue as they were after the last step (finalization step).
        """
        assert self.mode not in (MODE_XTS, MODE_CMAC) # finalizing (=padding) doesn't make sense when in XTS or CMAC mode
        if self.ed == 'e':
            # when the chain is in encryption mode, finalizing will pad the cache and encrypt this last block
            if self.mode in (MODE_OFB,MODE_CFB,MODE_CTR):
                dummy = '0'*(self.chain.totalbytes%self.blocksize) # a dummy string that will be used to get a valid padding
            else: #ECB, CBC
                dummy = self.chain.cache
            pad = padfct(dummy,padding.PAD,self.blocksize)[len(dummy):] # construct the padding necessary
            return self.chain.update(pad,'e') # supply the padding to the update function => chain cache will be "cache+padding"
        else:
            # final function doesn't make sense when decrypting => padding should be removed manually
            pass

class ECB:
    """ECB chaining mode
    """
    def __init__(self, codebook, blocksize):
        self.cache = ''
        self.codebook = codebook
        self.blocksize = blocksize

    def update(self, data, ed):
        """Processes the given ciphertext/plaintext

        Inputs:
            data: raw string of any length
            ed:   'e' for encryption, 'd' for decryption
        Output:
            processed raw string block(s), if any

        When the supplied data is not a multiple of the blocksize
          of the cipher, then the remaining input data will be cached.
        The next time the update function is called with some data,
          the new data will be concatenated to the cache and then
          cache+data will be processed and full blocks will be outputted.
        """
        output_blocks = []
        self.cache += data
        if len(self.cache) < self.blocksize:
            return ''
        for i in range(0, len(self.cache)-self.blocksize+1, self.blocksize):
            #the only difference between encryption/decryption in the chain is the cipher block
            if ed == 'e':
                output_blocks.append(self.codebook.encrypt( self.cache[i:i + self.blocksize] ))
            else:
                output_blocks.append(self.codebook.decrypt( self.cache[i:i + self.blocksize] ))
        self.cache = self.cache[i+self.blocksize:]
        return ''.join(output_blocks)

class CBC:
    """CBC chaining mode
    """
    def __init__(self, codebook, blocksize, IV):
        self.IV = IV
        self.cache = ''
        self.codebook = codebook
        self.blocksize = blocksize

    def update(self, data, ed):
        """Processes the given ciphertext/plaintext

        Inputs:
            data: raw string of any length
            ed:   'e' for encryption, 'd' for decryption
        Output:
            processed raw string block(s), if any

        When the supplied data is not a multiple of the blocksize
          of the cipher, then the remaining input data will be cached.
        The next time the update function is called with some data,
          the new data will be concatenated to the cache and then
          cache+data will be processed and full blocks will be outputted.
        """
        if ed == 'e':
            encrypted_blocks = ''
            self.cache += data
            if len(self.cache) < self.blocksize:
                return ''
            for i in range(0, len(self.cache)-self.blocksize+1, self.blocksize):
                self.IV = self.codebook.encrypt(util.xorstring(self.cache[i:i+self.blocksize],self.IV))
                encrypted_blocks += self.IV
            self.cache = self.cache[i+self.blocksize:]
            return encrypted_blocks
        else:
            decrypted_blocks = ''
            self.cache += data
            if len(self.cache) < self.blocksize:
                return ''
            for i in range(0, len(self.cache)-self.blocksize+1, self.blocksize):
                plaintext = util.xorstring(self.IV,self.codebook.decrypt(self.cache[i:i + self.blocksize]))
                self.IV = self.cache[i:i + self.blocksize]
                decrypted_blocks+=plaintext
            self.cache = self.cache[i+self.blocksize:]
            return decrypted_blocks

class CFB:
    # TODO: bit access instead of only byte level access
    """CFB Chaining Mode

    Can be accessed as a stream cipher.
    """

    def __init__(self, codebook, blocksize, IV,segment_size):
        self.codebook = codebook
        self.IV = IV
        self.blocksize = blocksize
        self.segment_size = segment_size/8
        self.keystream = []
        self.totalbytes = 0
        
    def update(self, data, ed):
        """Processes the given ciphertext/plaintext

        Inputs:
            data: raw string of any multiple of bytes
            ed:   'e' for encryption, 'd' for decryption
        Output:
            processed raw string

        The encrypt/decrypt functions will always process all of the supplied
          input data immediately. No cache will be kept.
        """
        output = list(data)

        for i in range(len(data)):
            if ed =='e':
                if len(self.keystream) == 0:
                    block = self.codebook.encrypt(self.IV)
                    self.keystream = list(block)[:self.segment_size] # keystream consists of the s MSB's
                    self.IV = self.IV[self.segment_size:] # keeping (b-s) LSB's
                output[i] = chr(ord(output[i]) ^ ord(self.keystream.pop(0)))
                self.IV += output[i] # the IV for the next block in the chain is being built byte per byte as the ciphertext flows in
            else:
                if len(self.keystream) == 0:
                    block = self.codebook.encrypt(self.IV)
                    self.keystream = list(block)[:self.segment_size]
                    self.IV = self.IV[self.segment_size:]
                self.IV += output[i]
                output[i] = chr(ord(output[i]) ^ ord(self.keystream.pop(0)))
        self.totalbytes += len(output)
        return ''.join(output)

class OFB:
    """OFB Chaining Mode

    Can be accessed as a stream cipher.
    """
    def __init__(self, codebook, blocksize, IV):
        self.codebook = codebook
        self.IV = IV
        self.blocksize = blocksize
        self.keystream = []
        self.totalbytes = 0
        
    def update(self, data, ed):
        """Processes the given ciphertext/plaintext

        Inputs:
            data: raw string of any multiple of bytes
            ed:   'e' for encryption, 'd' for decryption
        Output:
            processed raw string

        The encrypt/decrypt functions will always process all of the supplied
          input data immediately. No cache will be kept.
        """
        #no difference between encryption and decryption mode
        n = len(data)
        blocksize = self.blocksize
        output = list(data)

        for i in range(n):
            if len(self.keystream) == 0: #encrypt a new counter block when the current keystream is fully used
                self.IV = self.codebook.encrypt(self.IV)
                self.keystream = list(self.IV)
            output[i] = chr(ord(output[i]) ^ ord(self.keystream.pop(0))) #as long as an encrypted counter value is available, the output is just "input XOR keystream"
        
        self.totalbytes += len(output)
        return ''.join(output)

class CTR:
    """CTR Chaining Mode

    Can be accessed as a stream cipher.
    """
    # initial counter value can be choosen, decryption always starts from beginning
    #   -> you can start from anywhere yourself: just feed the cipher encoded blocks and feed a counter with the corresponding value
    def __init__(self, codebook, blocksize, counter):
        self.codebook = codebook
        self.counter = counter
        self.blocksize = blocksize
        self.keystream = [] #holds the output of the current encrypted counter value
        self.totalbytes = 0

    def update(self, data, ed):
        """Processes the given ciphertext/plaintext

        Inputs:
            data: raw string of any multiple of bytes
            ed:   'e' for encryption, 'd' for decryption
        Output:
            processed raw string

        The encrypt/decrypt functions will always process all of the supplied
          input data immediately. No cache will be kept.
        """
        # no need for the encryption/decryption distinction: both are the same
        n = len(data)
        blocksize = self.blocksize

        output = list(data)
        for i in range(n):
            if len(self.keystream) == 0: #encrypt a new counter block when the current keystream is fully used
                block = self.codebook.encrypt(self.counter())
                self.keystream = list(block)
            output[i] = chr(ord(output[i])^ord(self.keystream.pop(0))) #as long as an encrypted counter value is available, the output is just "input XOR keystream"
        self.totalbytes += len(output)
        return ''.join(output)

class XTS:
    """XTS Chaining Mode
    
    Usable with blockciphers with a 16-byte blocksize
    """
    # TODO: allow other blocksizes besides 16bytes?
    def __init__(self,codebook1, codebook2):
        self.cache = ''
        self.codebook1 = codebook1
        self.codebook2 = codebook2

    def update(self, data, ed,tweak=''):
        # supply n as a raw string
        # tweak = data sequence number
        """Perform a XTS encrypt/decrypt operation.

        Because the handling of the last two blocks is linked,
          it needs the whole block of ciphertext to be supplied at once.
        Every decrypt function called on a XTS cipher will output
          a decrypted block based on the current supplied ciphertext block.
        """
        output = ''
        assert len(data) > 15, "At least one block of 128 bits needs to be supplied"
        assert len(data) < 128*pow(2,20)

        # initializing T
        # e_k2_n = E_K2(tweak)
        e_k2_n = self.codebook2.encrypt(tweak+ '\x00' * (16-len(tweak)))[::-1]
        self.T = util.string2number(e_k2_n)

        i=0
        while i < ((len(data) // 16)-1): #Decrypt all the blocks but one last full block and opt one last partial block
            # C = E_K1(P xor T) xor T
            output += self.__xts_step(ed,data[i*16:(i+1)*16],self.T)
            # T = E_K2(n) mul (a pow i)
            self.__T_update()
            i+=1

        # Check if the data supplied is a multiple of 16 bytes -> one last full block and we're done
        if len(data[i*16:]) == 16:
            # C = E_K1(P xor T) xor T
            output += self.__xts_step(ed,data[i*16:(i+1)*16],self.T)
            # T = E_K2(n) mul (a pow i)
            self.__T_update()
        else:
            T_temp = [self.T]
            self.__T_update()
            T_temp.append(self.T)
            if ed=='d':
                # Permutation of the last two indexes
                T_temp.reverse()
            # Decrypt/Encrypt the last two blocks when data is not a multiple of 16 bytes
            Cm1 = data[i*16:(i+1)*16]
            Cm = data[(i+1)*16:]
            PP = self.__xts_step(ed,Cm1,T_temp[0])
            Cp = PP[len(Cm):]
            Pm = PP[:len(Cm)]
            CC = Cm+Cp
            Pm1 = self.__xts_step(ed,CC,T_temp[1])
            output += Pm1 + Pm
        return output

    def __xts_step(self,ed,tocrypt,T):
        T_string = util.number2string_N(T,16)[::-1]
        # C = E_K1(P xor T) xor T
        if ed == 'd':
            return util.xorstring(T_string, self.codebook1.decrypt(util.xorstring(T_string, tocrypt)))
        else:
            return util.xorstring(T_string, self.codebook1.encrypt(util.xorstring(T_string, tocrypt)))

    def __T_update(self):
        # Used for calculating T for a certain step using the T value from the previous step
        self.T = self.T << 1
        # if (Cout)
        if self.T >> (8*16):
            #T[0] ^= GF_128_FDBK;
            self.T = self.T ^ 0x100000000000000000000000000000087


class CMAC:
    """CMAC chaining mode

    Supports every cipher with a blocksize available
      in the list CMAC.supported_blocksizes.
    The hashlength is equal to block size of the used block cipher.
    
    Usable with blockciphers with a 8 or 16-byte blocksize
    """
    # TODO: move to hash module?
    # TODO: change update behaviour to .update() and .digest() as for all hash modules?
    #       -> other hash functions in pycrypto: calling update, concatenates current input with previous input and hashes everything
    __Rb_dictionary = {64:0x000000000000001b,128:0x00000000000000000000000000000087}
    supported_blocksizes = list(__Rb_dictionary.keys())
    def __init__(self,codebook,blocksize,IV):
        # Purpose of init: calculate Lu & Lu2
        #blocksize (in bytes): to select the Rb constant in the dictionary
        #Rb as a dictionary: adding support for other blocksizes is easy
        self.cache=''
        self.blocksize = blocksize
        self.codebook = codebook
        self.IV = IV

        #Rb_dictionary: holds values for Rb for different blocksizes
        # values for 64 and 128 bits found here: http://www.nuee.nagoya-u.ac.jp/labs/tiwata/omac/omac.html
        # explanation from: http://csrc.nist.gov/publications/nistpubs/800-38B/SP_800-38B.pdf
        #             Rb is a representation of a certain irreducible binary polynomial of degree b, namely,
        #             the lexicographically first among all such polynomials with the minimum possible number of
        #             nonzero terms. If this polynomial is expressed as ub+cb-1ub-1+...+c2u2+c1u+c0, where the
        #             coefficients cb-1, cb-2, ..., c2, c1, c0 are either 0 or 1, then Rb is the bit string cb-1cb-2...c2c1c0.

        self.Rb = self.__Rb_dictionary[blocksize*8]

        mask1 = int(('\xff'*blocksize).encode('hex'),16)
        mask2 = int(('\x80' + '\x00'*(blocksize-1) ).encode('hex'),16)

        L = int(self.codebook.encrypt('\x00'*blocksize).encode('hex'),16)

        if L & mask2:
            Lu = ((L << 1) & mask1) ^ self.Rb
        else:
            Lu = L << 1
            Lu = Lu & mask1

        if Lu & mask2:
            Lu2 = ((Lu << 1) & mask1)^ self.Rb
        else:
            Lu2 = Lu << 1
            Lu2 = Lu2 & mask1

        self.Lu =util.number2string_N(Lu,self.blocksize)
        self.Lu2=util.number2string_N(Lu2,self.blocksize)

    def update(self, data, ed):
        """Processes the given ciphertext/plaintext

        Inputs:
            data: raw string of any length
            ed:   'e' for encryption, 'd' for decryption
        Output:
            hashed data as raw string

        This is not really an update function:
        Everytime the function is called, the hash from the input data is calculated.
        No finalizing needed.
        """
        assert ed == 'e'
        blocksize = self.blocksize

        m = (len(data)+blocksize-1)/blocksize #m = amount of datablocks
        i=0
        for i in range(1,m):
            self.IV = self.codebook.encrypt( util.xorstring(data[(i-1)*blocksize:(i)*blocksize],self.IV) )

        if len(data[(i)*blocksize:])==blocksize:
            X = util.xorstring(util.xorstring(data[(i)*blocksize:],self.IV),self.Lu)
        else:
            tmp = data[(i)*blocksize:] + '\x80' + '\x00'*(blocksize - len(data[(i)*blocksize:])-1)
            X = util.xorstring(util.xorstring(tmp,self.IV),self.Lu2)

        T = self.codebook.encrypt(X)
        return T

