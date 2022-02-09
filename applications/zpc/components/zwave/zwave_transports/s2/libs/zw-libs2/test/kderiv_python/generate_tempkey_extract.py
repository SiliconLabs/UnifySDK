#!/usr/bin/env python

from CryptoPlus.testvectors import dict_cmac_aes128,dict_cmac_aes192,dict_cmac_aes256,dict_cmac_tdes2,dict_cmac_tdes3
import pdb

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
ecdh_share_secret = "4041424344454647404142434445464740414243444546474041424344454647"
auth_tag = "48494a4b4c4d4e4f48494a4b4c4d4e4f48494a4b4c4d4e4f48494a4b4c4d4e4f48494a4b4c4d4e4f48494a4b4c4d4e4f48494a4b4c4d4e4f48494a4b4c4d4e4f"
constant_prk = "33333333333333333333333333333333"
cipher = python_AES.new(constant_prk.decode('hex'),python_AES.MODE_CMAC);
temp = ecdh_share_secret+auth_tag
pdb.set_trace()
pseudo_random_keymat_output = cipher.encrypt(temp.decode('hex'));


print("pseudo_random_keymat_output: "+pseudo_random_keymat_output.encode('hex'))
