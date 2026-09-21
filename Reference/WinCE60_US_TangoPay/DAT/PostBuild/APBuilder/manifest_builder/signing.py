import os
import base64
import hashlib

from Crypto.Hash import HMAC, SHA256
from Crypto.PublicKey import RSA
from Crypto.Signature import pkcs1_15, pss

from asn1crypto import x509, pem, algos, util

from pyhsm.hsmclient import HsmClient
from pyhsm.hsmenums import HsmMech


class Signer:

    def __init__(self, cert_dir=None):
        self.certificates = []
        if not cert_dir is None:
            self._load_certs(cert_dir)

    def _load_certs(self, cert_dir):
        """
        Load the certificates in the directory

        """
        index = 0
        while os.path.exists(os.path.join(cert_dir, f'{index}.pem')):
            path = os.path.join(cert_dir, f'{index}.pem')
            with open(path, 'r') as certfile:
                pem_data = certfile.read()
                self.certificates.append(pem_data)
            
            index += 1

    def get_certs(self):
        """
        Retrieves the certificate with matching key_id from the HSM. The last certificate in the chain
        is the signing certificate

        """

        return self.certificates

    def get_id(self):
        """
        Returns a unique ID for the cert chain

        """
        if len(self.certificates) == 0:
            return []

        signing_cert_bytes = self.certificates[-1].encode('utf-8')
        if pem.detect(signing_cert_bytes):
            _, _, signing_cert_bytes = pem.unarmor(signing_cert_bytes)
        
        cert = x509.Certificate.load(signing_cert_bytes)
        return cert.subject.human_friendly

    def sign_file(self, filename, b64=True):
        """
        Sign the file and return the digest in b64

        """
        with open(filename, 'rb') as file:
            return self.sign_data(file.read(), b64=b64)

    def sign_data(self, data, b64=True):
        return 'null'

    def get_name(self):
        return 'none'

    def get_encoding_name(self):
        return 'none'

    def get_fullname(self, b64=False):
        if b64:
            return "{0}-{1}".format(
                self.get_name(),
                self.get_encoding_name()
            )
        return self.get_name()


class HMACSigner(Signer):
    def __init__(self, secret):
        self.secret = secret

    def sign_data(self, data, b64=True):
        """
        Sign the data and return the digest in b64

        """
        alg = HMAC.new(self.secret.encode('ascii'), digestmod=SHA256)

        alg.update(data)

        digest = alg.digest()
        if b64:
            return base64.b64encode(digest).decode('ascii')
        return digest
    
    def get_name(self):
        return "hmac256"

    def get_encoding_name(self, b64=False):
        return "b64"


class RSASigner(Signer):
    def __init__(self, keyfile, cert_dir=None):
        self.keyfile = keyfile
        self.key = RSA.import_key(open(keyfile).read())
        super().__init__(cert_dir)

    def sign_data(self, data, b64=True):
        """
        Sign the data and return the digest in b64

        """
        alg = SHA256.new()

        alg.update(data)

        signature = pkcs1_15.new(self.key).sign(alg)
        if b64:
            return base64.b64encode(signature).decode('ascii')
        return signature
    
    def get_name(self):
        return "rsa256"

    def get_encoding_name(self, b64=False):
        return "b64"


class Pkcs11Signer(Signer):
    """
    This signer uses a PKCS11 engine to sign the manifests

    """
    def __init__(self, key_id, pkcs11_password, cert_dir=None):
        self.key_id = key_id
        self.conn = HsmClient(pkcs11_lib="fxpkcs11.dll")

        self.conn.open_session(slot=0)

        if pkcs11_password:
            self.conn.login(pkcs11_password)
        
        super().__init__(cert_dir)

    def __del__(self):
        self.conn.logout()
        self.conn.close_session()

    def sign_data(self, data, b64=True):
        """
        Sign the data and return the digest in b64

        """

        alg = SHA256.new()

        alg.update(data)

        digest_algo = algos.DigestAlgorithm({'algorithm': algos.DigestAlgorithmId('sha256')})
        digestinfo = algos.DigestInfo({
            'digest_algorithm': digest_algo,
            'digest': alg.digest()
        })
        hash_input = digestinfo.dump()

        key = self.conn.get_object_handle(self.key_id)
        signature = self.conn.sign(key, hash_input, mechanism=HsmMech.RSA_PKCS)

        if b64:
            return base64.b64encode(signature).decode('ascii')
        return signature

    def get_name(self):
        return "rsa256"

    def get_encoding_name(self, b64=False):
        return "b64"
