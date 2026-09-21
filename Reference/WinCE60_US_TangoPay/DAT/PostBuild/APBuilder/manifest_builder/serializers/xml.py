import xml.etree.ElementTree as ET
from xml.dom import minidom

from manifest_builder.manifest import Manifest


class XmlSerializerV1:

    def deserialize(self, root):
        # Metadata
        metadata = root.find('metadata')
        version_elem = metadata.find('version')
        lineage = version_elem.attrib['lineage']
        territory = version_elem.attrib['territory']
        manifest_version = version_elem.text
        model = metadata.find('model').text
        severity = metadata.find('severity').text
        _type = metadata.find('type').text
        sig_type = metadata.find('sig_type').text
        file_version = root.attrib['version']

        manifest = Manifest(lineage, territory, manifest_version, model, severity, _type, sig_type, file_version)
        
        # Contents
        contents_list = root.find('contents')
        for item in contents_list.findall('item'):
            manifest.add_item('', '', item.text, 0)

        return manifest

    def serialize(self, manifest):
        xml_data = ET.Element('manifest')
        xml_data.set('version', manifest.file_version)

        # Build Metadata
        metadata = ET.SubElement(xml_data, 'metadata')
        
        # Version
        version = ET.SubElement(metadata, 'version')
        version.set('territory', manifest.territory)
        version.set('lineage', manifest.lineage)
        version.text = manifest.version

        ET.SubElement(metadata, 'version_human').text = manifest.version_readable

        ET.SubElement(metadata, 'build_date').text = manifest.builddate
        ET.SubElement(metadata, 'model').text = manifest.model
        ET.SubElement(metadata, 'severity').text = manifest.severity
        ET.SubElement(metadata, 'type').text = manifest.type
        ET.SubElement(metadata, 'sig_type').text = manifest.sig_type

        # Build Contents
        total_size = 0
        count = 0

        contents = ET.SubElement(xml_data, 'contents')

        for item in manifest.contents:
            xml_item = ET.SubElement(contents, 'item')

            total_size += item['filesize']
            count += 1

            xml_item.set('sig_type', item['sig_type'])
            xml_item.set('sig', item['sig'])
            xml_item.set('size', str(item['filesize']))
            xml_item.text = item['filename']

        contents.set('total_size', str(total_size))
        contents.set('count', str(count))

        return xml_data


class XmlSerializerV1_1:

    def deserialize(self, root):
        # Metadata
        metadata = root.find('metadata')
        version_elem = metadata.find('version')
        lineage = version_elem.attrib['lineage']
        territory = version_elem.attrib['territory']
        manifest_version = version_elem.text
        model = metadata.find('model').text
        severity = metadata.find('severity').text
        _type = metadata.find('type').text
        sig_type = metadata.find('sig_type').text
        file_version = root.attrib['version']

        manifest = Manifest(lineage, territory, manifest_version, model, severity, _type, sig_type, file_version)
        
        # Contents
        contents_list = root.find('contents')
        for item in contents_list.findall('item'):
            manifest.add_item('', '', item.text, 0)

        # Signatures
        sig_list = root.find('signatures')
        for sig in sig_list.findall('sig'):
            manifest.add_signature(item.text, sig.attrib['sig_type'], sig.attrib['item'], sig.attrib['chain'])

        # Chains
        chain_list = metadata.find('chains')
        for chain in chain_list.findall('chain'):
            chain_id = chain.attrib['id']

            certs = []
            for cert in chain.findall('cert'):
                certs.append(cert.text)

            manifest.add_chain(chain_id, certs)

        return manifest

    def serialize(self, manifest):
        xml_data = ET.Element('manifest')
        xml_data.set('version', manifest.file_version)

        # Build Metadata
        metadata = ET.SubElement(xml_data, 'metadata')
        
        # Version
        version = ET.SubElement(metadata, 'version')
        version.set('territory', manifest.territory)
        version.set('lineage', manifest.lineage)
        version.text = manifest.version

        ET.SubElement(metadata, 'version_human').text = manifest.version_readable

        ET.SubElement(metadata, 'build_date').text = manifest.builddate
        ET.SubElement(metadata, 'model').text = manifest.model
        ET.SubElement(metadata, 'severity').text = manifest.severity
        ET.SubElement(metadata, 'type').text = manifest.type
        ET.SubElement(metadata, 'sig_type').text = manifest.sig_type

        # Build Contents
        total_size = 0
        count = 0

        contents = ET.SubElement(xml_data, 'contents')

        for item in manifest.contents:
            xml_item = ET.SubElement(contents, 'item')

            total_size += item['filesize']
            count += 1

            xml_item.set('sig_type', item['sig_type'])
            xml_item.set('sig', item['sig'])
            xml_item.set('size', str(item['filesize']))
            xml_item.text = item['filename']

        contents.set('total_size', str(total_size))
        contents.set('count', str(count))

        # Build Signatures
        signatures = ET.SubElement(xml_data, 'signatures')
        for sig in manifest.signatures:
            xml_item = ET.SubElement(signatures, 'sig')

            xml_item.set('item', sig['filename'])
            xml_item.set('sig_type', sig['sig_type'])
            xml_item.set('chain', sig['chain_id'])

            xml_item.text = sig['sig']

        # Build Chains
        chains = ET.SubElement(metadata, 'chains')
        for chain in manifest.chains:
            chain_item = ET.SubElement(chains, 'chain')

            chain_item.set('id', chain.chain_id)

            for cert in chain.certificates:
                cert_item = ET.SubElement(chain_item, 'cert')
                cert_item.text = cert

        return xml_data


class XmlSerializer:
    def __init__(self):
        pass

    def deserialize(self, input_file):
        xml = ET.parse(input_file)
        root = xml.getroot()

        if root == False:
            # Failed to parse
            return None

        serializer = None
        version = root.attrib['version']
        if version == '1.0.0':
            serializer = XmlSerializerV1()
        elif version == '1.1.0':
            serializer = XmlSerializerV1_1()

        return serializer.deserialize(root)

    def serialize(self, manifest, output_file):

        serializer = None
        if manifest.file_version == '1.0.0':
            serializer = XmlSerializerV1()
        elif manifest.file_version == '1.1.0':
            serializer = XmlSerializerV1_1()

        xml_data = serializer.serialize(manifest)        

        # Write the manifest file
        self._write_xml(xml_data, output_file)

    def _write_xml(self, xml_data, output_file):
        xml_string = ET.tostring(xml_data)
        indented = minidom.parseString(xml_string).toprettyxml()

        with open(output_file, 'w') as manifest:
            manifest.write(indented)
