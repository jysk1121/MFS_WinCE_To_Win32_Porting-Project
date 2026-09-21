from xml.etree import ElementTree as ET
from .models import Reference, RemotePackage


def packages_from_file(filename: str):
    """
    Deserialized the package index from the file

    """
    index = []

    xml = ET.parse(filename)

    for package_node in xml.findall("package"):
        ref = RemotePackage()
        ref.file_type = package_node.attrib['type']
        ref.version = package_node.attrib['version']
        ref.name = package_node.attrib['name']
        ref.path = package_node.attrib['path']

        index.append(ref)

    return index


def references_from_file(filename: str, customer: str='V', territory: str='US'):
    """
    Deserializes the references from the file

    """
    references = []

    xml = ET.parse(filename)

    for package_node in xml.findall("package"):
        ref = Reference()

        ref.name = package_node.attrib['name']
        ref.version = package_node.attrib['version']
        ref.destination = package_node.attrib['destination']
        
        references.append(ref)

    for config_node in xml.findall("configuration"):
        if config_node.attrib['territory'] == territory and config_node.attrib['customer'] == customer:
            # This configuration node applies, so add in the references

            for package_node in config_node.findall("package"):
                ref = Reference()

                ref.name = package_node.attrib['name']
                ref.version = package_node.attrib['version']
                ref.destination = package_node.attrib['destination']
                
                references.append(ref)

    return references
