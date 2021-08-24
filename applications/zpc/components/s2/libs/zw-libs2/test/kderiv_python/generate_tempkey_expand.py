#!/usr/bin/env python

#from pkg_resources import require
#require("CryptoPlus>=1.0")
from CryptoPlus.testvectors import dict_cmac_aes128,dict_cmac_aes192,dict_cmac_aes256,dict_cmac_tdes2,dict_cmac_tdes3
import pdb

def print_c_array(number):
    res = ''
    for x in number:
        res = res + '0x'+x.encode('hex')+', '
    return res

# CMAC-AES128/192/256
print("CMAC-AES")

import python_AES

'''
for d in dict_cmac_aes128,dict_cmac_aes192,dict_cmac_aes256:
    for i in range(0,len(d)/4):
        msg = d['msg%i'%i].decode('hex')
        key = d['key%i'%i].decode('hex')
        if msg == '\x00':
            msg = ''
        mac = d['mac%i'%i].decode('hex')
#	pdb.set_trace()
        cipher = python_AES.new(key,python_AES.MODE_CMAC)
        if mac <> cipher.encrypt(msg)[:d['taglength%i'%i]]:
            print 'ERROR for %i'%i
        print "Passed for msg: "+msg.encode('hex')+"key: "+key.encode('hex')+"output (mac): "+mac.encode('hex')
'''

network_key = "74778f89d4d88c6e4659634a88ea987a"
cipher = python_AES.new(network_key.decode('hex'),python_AES.MODE_CMAC);
constant_nk = "888888888888888888888888888888"
temp = constant_nk;
temp = temp+"01"

ccm_key = cipher.encrypt(temp.decode('hex'));

temp = ccm_key.encode('hex') 
temp = temp+constant_nk
temp = temp+"02"

cipher = python_AES.new(network_key.decode('hex'),python_AES.MODE_CMAC);
pers_string = cipher.encrypt(temp.decode('hex'));

temp = pers_string.encode('hex') 
temp = temp+constant_nk
temp = temp+"03"

#pdb.set_trace()
cipher = python_AES.new(network_key.decode('hex'),python_AES.MODE_CMAC);
pers_string+= cipher.encrypt(temp.decode('hex'));

temp = pers_string[16:].encode('hex') 
temp = temp+constant_nk
temp = temp+"04"
cipher = python_AES.new(network_key.decode('hex'),python_AES.MODE_CMAC);
mpan_key = cipher.encrypt(temp.decode('hex'))

print("ccm_key: "+print_c_array(ccm_key))
print("pers_string: "+print_c_array(pers_string))
print("mpan_key: "+print_c_array(mpan_key))

'''
msg = ''
key1 = "2b7e151628aed2a6abf7158809cf4f3c"
cipher = python_AES.new(key1.decode('hex'),python_AES.MODE_CMAC);
mac = cipher.encrypt(msg.decode('hex'));

print "mac: "+mac.encode('hex')

msg = "6bc1bee22e409f96e93d7e117393172a"
cipher = python_AES.new(key1.decode('hex'),python_AES.MODE_CMAC);
mac = cipher.encrypt(msg.decode('hex'));

print "mac: "+mac.encode('hex')
msg = "6bc1bee22e409f96e93d7e117393172aae2d8a571e03ac9c9eb76fac45af8e5130c81c46a35ce411"
cipher = python_AES.new(key1.decode('hex'),python_AES.MODE_CMAC);
mac = cipher.encrypt(msg.decode('hex'));

print "mac: "+mac.encode('hex')
'''
