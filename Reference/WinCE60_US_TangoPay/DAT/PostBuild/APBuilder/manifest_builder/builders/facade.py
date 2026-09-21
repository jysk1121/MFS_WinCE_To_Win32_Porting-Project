from datetime import datetime
import os

from asn1crypto import cms, util, x509, pem, algos

from .v1_builder import ManifestV1Builder
from .v1_1_builder import ManifestV11Builder

class ManifestBuilder:
    def __init__(self, signer, input_dir, manifest, extra_signers=[]):
        self.sub_builder = None
        self.manifest = manifest
        if self.manifest.file_version == '1.0.0':
            self.sub_builder = ManifestV1Builder(signer, input_dir, manifest)
        elif self.manifest.file_version == '1.1.0':
            self.sub_builder = ManifestV11Builder(signer, input_dir, manifest, extra_signers=extra_signers)

        if self.sub_builder is None:
            raise Exception("Unknown builder version!")

    def build_manifest(self):
        """
        Calls the appropriate builder version

        """
        return self.sub_builder.build_manifest()

def write_manifest_sig(manifest_file, signer):
    """
    Signs the manifest file itself

    """
    sig = signer.sign_file(manifest_file, b64=False)
    manifest_path = os.path.dirname(manifest_file)
    manifest_sig_filename = os.path.join(manifest_path, 'manifest.sig.' + signer.get_name())

    with open(manifest_sig_filename, 'wb') as file:
        file.write(sig)


def write_manifest_p7(manifest_file, signers):
    """
    Creates the manifest's PKCS7 file

    Adapted from : https://stackoverflow.com/a/58720769

    """

    with open(manifest_file, 'rb') as file:
        data = file.read()

    # Creating a SignedData object from cms
    sd = cms.SignedData()

    # Populating some of its fields
    sd['version'] = 'v1'
    sd['encap_content_info'] = util.OrderedDict([
            ('content_type', 'data'),
            ('content', None)])
    sd['digest_algorithms'] = [ util.OrderedDict([
            ('algorithm', 'sha256'),
            ('parameters', None) ])]

    certificates = []
    signer_infos = []
    for signer in signers:
        # Get certificate for signer
        # NB: The last cert in the list is the signing cert
        for pem_data in signer.get_certs():
            pem_bytes = pem_data.encode('utf-8')
            if pem.detect(pem_bytes):
                _, _, der_bytes = pem.unarmor(pem_bytes)
                cert = x509.Certificate.load(der_bytes)
                certificates.append(cert)

        # Setting signer info section
        signer_info = cms.SignerInfo()
        signer_info['version'] = 'v1'
        signer_info['digest_algorithm'] = util.OrderedDict([
                        ('algorithm', 'sha256'),
                        ('parameters', None) ])
        signer_info['signature_algorithm'] = util.OrderedDict([
                        ('algorithm', 'rsassa_pkcs1v15'),
                        ('parameters', None) ])

        # Creating a signature using a private key object from pkcs11
        signer_info['signature'] = signer.sign_data(data, b64=False)

        # Finding issuer_and_serial_number from certificate (asn1crypto.x509 object)
        signer_info['sid'] = cms.SignerIdentifier({
                'issuer_and_serial_number': cms.IssuerAndSerialNumber({
                    'issuer': cert.issuer,
                    'serial_number': cert.serial_number
                }) })

        signer_infos.append(signer_info)

    # Adding SignerInfo object to SignedData object
    sd['signer_infos'] = signer_infos

    # Set the Certificates structure
    sd['certificates'] = certificates

    # Writing everything into ASN.1 object
    asn1obj = cms.ContentInfo()
    asn1obj['content_type'] = 'signed_data'
    asn1obj['content'] = sd

    # This asn1obj can be dumped to a disk using dump() method (DER format)
    manifest_path = os.path.dirname(manifest_file)
    manifest_pkcs7_filename = os.path.join(manifest_path, 'manifest.p7')
    with open(manifest_pkcs7_filename,'wb+') as file:
        file.write(pem.armor('PKCS7', asn1obj.dump()))
