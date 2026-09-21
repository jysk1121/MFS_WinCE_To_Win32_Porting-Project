#!/usr/bin/env python3
from datetime import datetime


class Chain:
    def __init__(self, chain_id):
        self.chain_id = chain_id

        self.certificates = []

    def add_cert(self, cert):
        self.certificates.append(cert)


class Manifest:
    def __init__(self, lineage, territory, version, model, severity, _type, sig_type, file_version):
        self.lineage = lineage
        self.territory = territory
        self.version = version
        self.severity = severity
        self.type = _type
        self.sig_type = sig_type

        self.version_readable = '{0}{1}_{2}'.format(
            self.lineage,
            self.territory,
            self.version
        )

        self.file_version = file_version
        self.model = model
        self.builddate = datetime.now().isoformat()

        self.contents = []
        self.signatures = []

        # These are set up by the builder using the data from the signer in version 1.1.0
        self.chains = []

    def add_item(self, signature, sig_type, filename, filesize):
        """
        Registers an item with the manifest

        """

        self.contents.append({
            'sig': signature,
            'sig_type': sig_type,
            'filename': filename,
            'filesize': filesize,
        })

    def add_signature(self, signature, sig_type, filename, chain_id):
        """
        Adds the signature for the file and chain ID

        """
        self.signatures.append({
            'sig': signature,
            'sig_type': sig_type,
            'filename': filename,
            'chain_id': chain_id
        })

    def add_chain(self, chain_id, certs):
        """
        Registers a new chain for the manifest

        """
        chain = Chain(chain_id)
        for cert in certs:
            chain.add_cert(cert)

        self.chains.append(chain)
