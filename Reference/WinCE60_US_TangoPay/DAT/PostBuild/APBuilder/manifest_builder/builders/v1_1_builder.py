import os


class ManifestV11Builder:
    def __init__(self, signer, input_dir, manifest, extra_signers=[]):
        self.signer = signer
        self.input_dir = input_dir
        self.manifest = manifest
        self.extra_signers = extra_signers

        if not hasattr(signer, 'get_certs'):
                raise Exception('Extra signers must implement get_certs()')

        if not hasattr(signer, 'get_id'):
            raise Exception('Extra signers must implement get_id()')

        for s in self.extra_signers:
            if not hasattr(s, 'get_certs'):
                raise Exception('Extra signers must implement get_certs()')

            if not hasattr(s, 'get_id'):
                raise Exception('Extra signers must implement get_id()')

    def _create_file_sig(self, filename, signer):
        """
        Sign the file and return the digest in b64

        """
        return signer.sign_file(filename)

    def _add_item(self, filepath, filesize, signature, signer):
        self.manifest.add_item(signature, signer.get_fullname(b64=True), filepath, filesize)

    def _add_item_signature(self, filepath, signature, signer):
        self.manifest.add_signature(signature, signer.get_fullname(b64=True), filepath, signer.get_id())

    def _add_files(self):
        """
        Walks the dir/files and calculates the signatures

        """

        for directory, sub_dirs, files in os.walk(self.input_dir):
            for filename in files:

                # Don't include the manifest in the manifest
                if filename.startswith("manifest"):
                    continue

                # Path to the file relative to the execution dir
                full_path = os.path.join(directory, filename)
                # Path to the file relative to the update dir
                item_name = os.path.relpath(full_path, self.input_dir)

                # Get file size
                info = os.stat(full_path)
                filesize = info.st_size

                # Get signature from base signer
                sig = self._create_file_sig(full_path, self.signer)
                self._add_item(item_name, filesize, sig, self.signer)
                # Do it again for the v1.1.0-style signatures
                self._add_item_signature(item_name, sig, self.signer)

                # Add extra signatures
                for signer in self.extra_signers:
                    sig = self._create_file_sig(full_path, signer)
                    self._add_item_signature(item_name, sig, signer)

    def _add_certs(self):
        """
        Adds the certificate information from the signer

        """
        for signer in self.extra_signers:
            certs = signer.get_certs()
            chain_id = signer.get_id()

            self.manifest.add_chain(chain_id, certs)

        # And add the original signer for v1.1.0 style chains
        self.manifest.add_chain(self.signer.get_id(), self.signer.get_certs())

    def build_manifest(self):
        """
        Builds the remaining data

        """
        self._add_files()
        self._add_certs()
